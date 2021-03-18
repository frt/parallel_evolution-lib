#include <config.h>

#include "parallel_evolution.h"
#include "parallel_evolution/mpi_util.h"
#include "parallel_evolution/report.h"
#if HAVE_MPI_H
    #include <mpi.h>
#elif HAVE_MPI_MPI_H
    #include <mpi/mpi.h>
#endif
#include <stdlib.h>
#include <stdio.h>

#ifndef SYSCONFDIR
    #define SYSCONFDIR "/usr/local/etc"
#endif

/* error codes */
#define ERROR_PROCESSES_CREATE 3
#define ERROR_PROCESSES_GET_ALGORITHM 4
#define ERROR_POPULATIONS_ALLOC 5
#define ERROR_CONFIG 6

#define MODULE_PARALLEL_EVOLUTION "parallel_evolution"

parallel_evolution_t parallel_evolution;
char log_msg[256];

void algorithm_totalizer(int world_size)
{
    population_t **populations;
    int done_rank;
    int i;
    algorithm_stats_t *algorithm_stats;
    int stats_node;
    algorithm_t *algorithm;
    population_t *my_population;

    if (parallel_evolution.algorithm == NULL) {
        parallel_evolution_log(LOG_PRIORITY_ERR, MODULE_PARALLEL_EVOLUTION, "Could not get my algorithm. Quit...");
        exit(ERROR_PROCESSES_GET_ALGORITHM);
    } else
        algorithm = parallel_evolution.algorithm;
    algorithm->get_population(&my_population);

    populations = (population_t **)malloc(world_size * sizeof(population_t *));
    if (populations == NULL) {
        parallel_evolution_log(LOG_PRIORITY_ERR, MODULE_PARALLEL_EVOLUTION, "Fail to allocate the array of populations. Quit.");
        exit(ERROR_POPULATIONS_ALLOC);
    }
    populations[0] = my_population;
    populations[0]->stats = algorithm->get_stats();

    parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "Waiting for convergence...");
    for (i = 1; i < world_size; ++i) {
        done_rank = mpi_util_recv_report_done();
        sprintf(log_msg, "Received report_done from process %d...", done_rank);
        parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_PARALLEL_EVOLUTION, log_msg);
    }

    parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "Waiting resultant populations...");
    for (i = 1; i < world_size; ++i) {
        sprintf(log_msg, "Receiving population from process %d...", i);
        parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_PARALLEL_EVOLUTION, log_msg);

        mpi_util_recv_population(i, populations);

        sprintf(log_msg, "Population from process %d received.", i);
        parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_PARALLEL_EVOLUTION, log_msg);
    }
    for (i = 1; i < world_size; ++i) {
        algorithm_stats = (algorithm_stats_t *)malloc(sizeof(algorithm_stats_t *));
        if (algorithm_stats == NULL) {
            parallel_evolution_log(LOG_PRIORITY_ERR, MODULE_PARALLEL_EVOLUTION, "Fail to allocate stats. Quit.");
            exit(ERROR_POPULATIONS_ALLOC);
        }

        // blocking recv
        if (SUCCESS == mpi_util_recv_stats(algorithm_stats, &stats_node)) {
            populations[stats_node]->stats = algorithm_stats;
        } else {
            parallel_evolution_log(LOG_PRIORITY_ERR, MODULE_PARALLEL_EVOLUTION, "Fail to receive stats. Quit.");
            exit(ERROR_POPULATIONS_ALLOC);
        }
    }

    parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "All populations received.");
    report_results(populations, world_size);
}

void log_if_error(int ret, const char *logmsg)
{
    if (ret == CONFIG_FALSE) {
        parallel_evolution_log(LOG_PRIORITY_ERR, MODULE_PARALLEL_EVOLUTION, logmsg);
        exit(ERROR_CONFIG);
    }
}

void algorithm_executor(int rank, config_t *config)
{
    int n = 0;
    size_t size = 0;
    char *str = NULL;

    int i;
    double limit_min, limit_max;
    algorithm_t *algorithm;
    algorithm_stats_t *algorithm_stats = NULL;
    migrant_t *migrant;

    int converged = 0;
    population_t *my_population;

    adjacency_list_t *adjacency_list;
    int *adjacency_array = NULL;
    int adjacency_array_size;
    int node_id;
    status_t ret;

    // config vars
    config_setting_t *setting, *elem;
    const char *algorithm_name;
    int number_of_dimensions;
    int migration_interval;
    const char *log_level;

    log_if_error(config_lookup_string(config, "algorithm_name", &algorithm_name), "Error reading algorithm_name. Quit.");
    log_if_error(config_lookup_int(config, "parallel_evolution.number_of_dimensions", &number_of_dimensions), "Error reading parallel_evolution.number_of_dimensions. Quit.");
    parallel_evolution_set_number_of_dimensions(number_of_dimensions);
    log_if_error(config_lookup_int(config, "parallel_evolution.migration_interval", &migration_interval), "Error reading parallel_evolution.migration_interval. Quit.");
    parallel_evolution_set_migration_interval(migration_interval);
    log_if_error(config_lookup_string(config, "parallel_evolution.log_level", &log_level), "Error reading parallel_evolution.log_level. Quit.");

    // get dimensions limits from config
    // "parallel_evolution.dimensions_limits[0].min .. parallel_evolution.dimensions_limits[n-1].min"
    // "parallel_evolution.dimensions_limits[0].max .. parallel_evolution.dimensions_limits[n-1].max"
    parallel_evolution.limits = (limit_t *)malloc(parallel_evolution.number_of_dimensions * sizeof(limit_t *));
    if (parallel_evolution.limits == NULL) {
        parallel_evolution_log(LOG_PRIORITY_ERR, MODULE_PARALLEL_EVOLUTION, "Fail to allocate the array of limits. Quit.");
        exit(ERROR_CONFIG);
    }
    setting = config_lookup(config, "parallel_evolution.dimensions_limits");
    for (i = 0; i < number_of_dimensions; ++i) {
        if ((elem = config_setting_get_elem(setting, i)) != NULL) {
            log_if_error(config_setting_lookup_float(elem, "min", &limit_min), "Error reading parallel_evolution.dimensions_limits. Quit.");
            log_if_error(config_setting_lookup_float(elem, "max", &limit_max), "Error reading parallel_evolution.dimensions_limits. Quit.");
        }
        parallel_evolution.limits[i].min = limit_min;
        parallel_evolution.limits[i].max = limit_max;

        sprintf(log_msg, "Limits of dimension %d set to [%f, %f].", i, limit_min, limit_max);
        parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_PARALLEL_EVOLUTION, log_msg);
    }

    // get topology from config
    // "parallel_evolution.topology.nodes.0[0, ..] .. parallel_evolution.topology.nodes.(m-1)[0, ..]"
    setting = config_lookup(config, "parallel_evolution.topology.nodes");
    if (setting != NULL) {
        n = snprintf(str, size, "%d", rank);    // discover the size of memory needed.
        if (n < 0) {
            parallel_evolution_log(LOG_PRIORITY_ERR, MODULE_PARALLEL_EVOLUTION, "Error reading topology. Quit.");
            exit(ERROR_CONFIG);
        }
        size = (size_t) n + 1;
        str = malloc(size);
        if (str == NULL) {
            parallel_evolution_log(LOG_PRIORITY_ERR, MODULE_PARALLEL_EVOLUTION, "Error reading topology. Quit.");
            exit(ERROR_CONFIG);
        }
        snprintf(str, size, "%d", rank);    // put in str the element name corresponding to my rank

        if (adjacency_list_create(&adjacency_list) != SUCCESS) {
            parallel_evolution_log(LOG_PRIORITY_ERR, MODULE_PARALLEL_EVOLUTION, "Error reading topology. Quit.");
            exit(ERROR_CONFIG);
        }

        setting = config_setting_get_member(setting, str);
        free(str);
        if (setting == NULL) {
            for (i = 0; (elem = config_setting_get_elem(setting, i)) != NULL; ++i) {
                node_id = config_setting_get_int(elem);
                if (adjacency_list_add(adjacency_list, node_id) != SUCCESS) {
                    parallel_evolution_log(LOG_PRIORITY_ERR, MODULE_PARALLEL_EVOLUTION, "Error reading topology. Quit.");
                    exit(ERROR_CONFIG);
                }
            }
            if (adjacency_list_get_all(adjacency_list, &adjacency_array) != SUCCESS) {
                parallel_evolution_log(LOG_PRIORITY_ERR, MODULE_PARALLEL_EVOLUTION, "Error reading topology. Quit.");
                exit(ERROR_CONFIG);
            }
            adjacency_array_size = i;
            adjacency_list_destroy(&adjacency_list);
        }
    }

    if (parallel_evolution.algorithm == NULL) {
        parallel_evolution_log(LOG_PRIORITY_ERR, MODULE_PARALLEL_EVOLUTION, "Could not get the algorithm. Quit...");
        exit(ERROR_PROCESSES_GET_ALGORITHM);
    } else
        algorithm = parallel_evolution.algorithm;

    algorithm->init(config);
    parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "Algorithm initialized.");
    migrant_create(&migrant, parallel_evolution.number_of_dimensions);

    while (1) {
        if (!converged) {
            /* receive migrants */
            while (mpi_util_recv_migrant(migrant) == SUCCESS) {
                algorithm->insert_migrant(migrant);
                parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "Migrant inserted into local population.");
            }

            /* run algorithm */
            algorithm->run_iterations(parallel_evolution.migration_interval);
            sprintf(log_msg, "Algorithm has runned for %d iterations.", parallel_evolution.migration_interval);
            parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_PARALLEL_EVOLUTION, log_msg);

            /* send migrant */
            if (adjacency_array != NULL) {
                algorithm->pick_migrant(migrant);
                parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "Migrant picked up from local population to send to other processes.");
                mpi_util_send_migrant(migrant, adjacency_array, adjacency_array_size);
            }

            /* report to master that the algorithm has converged */
            if (algorithm->ended()) {
                parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "Algorithm ended.");
                if (rank != 0) mpi_util_send_report_done();
                converged = !converged;
            }
        } else {
            if (rank == 0) {
                break;
            }

            algorithm->get_population(&my_population);
            parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "Population ready to send.");
            mpi_util_send_population(my_population);

            /* colect and send stats */
            algorithm_stats = algorithm->get_stats();
            mpi_util_send_stats(algorithm_stats);

            break;
        }
    }
}

config_error_t parallel_evolution_read_config_file(config_t *config)
{
    const char *config_file = SYSCONFDIR "/parallel_evolution.conf";
    config_error_t error_type;
    char error_msg[1024];

    config_read_file(config, config_file);
    error_type = config_error_type(config);
    switch (error_type) {
        case CONFIG_ERR_FILE_IO:
            snprintf(error_msg, 1024, "Configuration file error: file '%s' I/O error.",
                    config_file);
            parallel_evolution_log(LOG_PRIORITY_ERR, MODULE_PARALLEL_EVOLUTION, error_msg);
            break;
        case CONFIG_ERR_PARSE:
            snprintf(error_msg, 1024,
                    "Configuration file error: parse error '%s' on line %d of file '%s'.",
                    config_error_text(config),
                    config_error_line(config),
                    config_error_file(config));
            parallel_evolution_log(LOG_PRIORITY_ERR, MODULE_PARALLEL_EVOLUTION, error_msg);
            break;
        case CONFIG_ERR_NONE:
            parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_PARALLEL_EVOLUTION,
                    "Configuration file read.");
            break;
    }
    return error_type;
}

int parallel_evolution_run(int *argc, char ***argv)
{
    int rank, world_size;
    config_t *config;

    if (parallel_evolution_read_config_file(config) != CONFIG_ERR_NONE)
        exit(ERROR_CONFIG);

    MPI_Init(argc, argv);
    parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "MPI inicializado.");

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    sprintf (log_msg, "I am process %d of %d.", rank, world_size);
    parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_PARALLEL_EVOLUTION, log_msg);

    algorithm_executor(rank, config);
    if (rank == 0)
        algorithm_totalizer(world_size);

    parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "MPI will be finalized.");
    MPI_Finalize();

    return 0;
}

void parallel_evolution_set_topology(topology_t *topology)
{
    parallel_evolution.topology = topology;
    parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "Topology set.");
}

void parallel_evolution_set_number_of_dimensions(int number_of_dimensions)
{
    parallel_evolution.number_of_dimensions = number_of_dimensions;

    sprintf(log_msg, "Number of dimensions set to %d.", number_of_dimensions);
    parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_PARALLEL_EVOLUTION, log_msg);
}

void parallel_evolution_set_algorithm(algorithm_t *algorithm)
{
    parallel_evolution.algorithm = algorithm;

    parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "Algorithm set.");
}

void parallel_evolution_set_migration_interval(int iterations)
{
    parallel_evolution.migration_interval = iterations;
    parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "Interval set.");
}

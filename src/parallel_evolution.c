#include "config.h"
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

/* error codes */
#define ERROR_PROCESSES_CREATE 3
#define ERROR_PROCESSES_GET_ALGORITHM 4
#define ERROR_POPULATIONS_ALLOC 5

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

	parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "Sending \"finalize\" notifications...");
	mpi_util_send_finalize();

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

void algorithm_executor(int rank)
{
	algorithm_t *algorithm;
	algorithm_stats_t *algorithm_stats = NULL;
	migrant_t *migrant;

	int converged = 0;
	population_t *my_population;

	int *adjacency_array = NULL;
	int adjacency_array_size;
	int node_id;
	status_t ret;

	ret = topology_get_first_node(parallel_evolution.topology, &node_id, &adjacency_array, &adjacency_array_size);
	while (ret == SUCCESS) {
        if (node_id == rank)
            break;
		ret = topology_get_next_node(parallel_evolution.topology, &node_id, &adjacency_array, &adjacency_array_size);
	}

	if (parallel_evolution.algorithm == NULL) {
		parallel_evolution_log(LOG_PRIORITY_ERR, MODULE_PARALLEL_EVOLUTION, "Could not get the algorithm. Quit...");
		exit(ERROR_PROCESSES_GET_ALGORITHM);
	} else
        algorithm = parallel_evolution.algorithm;

	algorithm->init();
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

            /* wait for "finalize" msg and send population */
            mpi_util_recv_finalize();
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

int parallel_evolution_run(int *argc, char ***argv)
{
	int rank, world_size;

	MPI_Init(argc, argv);
	parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "MPI inicializado.");

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	sprintf (log_msg, "I am process %d of %d.", rank, world_size);
	parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_PARALLEL_EVOLUTION, log_msg);

    algorithm_executor(rank);
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

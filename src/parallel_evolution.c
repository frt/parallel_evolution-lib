#include "config.h"
#include "parallel_evolution.h"
#if HAVE_MPI_H 
	#include <mpi.h>
#elif HAVE_MPI_MPI_H
	#include <mpi/mpi.h>
#endif
#include <stdlib.h>
#include <stdio.h>

#define MIGRATION_INTERVAL 100	/* FIXME should move this to parallel_evolution struct */

/* error codes */
#define ERROR_TOPOLOGY_CREATE 1
#define ERROR_TOPOLOGY_PARSE 2
#define ERROR_PROCESSES_CREATE 3
#define ERROR_PROCESSES_GET_ALGORITHM 4
#define ERROR_POPULATIONS_ALLOC 5

parallel_evolution_t parallel_evolution;

int parallel_evolution_run(int *argc, char ***argv)
{
	int rank, world_size;
	int i;
	population_t **populations;
	algorithm_t *algorithm;
	migrant_t *migrant;
	population_t *my_population;
	topology_t *topology;
	int *adjacency_array = NULL;
	int adjacency_array_size;
	processes_t *processes;
	const char log_msg[256];

	MPI_Init(argc, argv);
	parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "MPI inicializado.");

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	sprintf (log_msg, "I am process %d of %d.", rank, world_size);
	parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, log_msg);

	if (rank == 0) {	/* topology controller */
		parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "I am the master of topologies!");
		/* create the topology */
		if (topology_create(&topology) != SUCCESS) {
			parallel_evolution_log(SEVERITY_ERROR, MODULE_PARALLEL_EVOLUTION, "Topology could not be created. Quit.");
			return ERROR_TOPOLOGY_CREATE;
		}

		/* FIXME topology parser should not be in the lib */
		/* parse topology from file */
		if (topology_parser_parse(topology, parallel_evolution.topology_file_name) != SUCCESS) {
			topology_destroy(&topology);
			parallel_evolution_log(SEVERITY_ERROR, MODULE_PARALLEL_EVOLUTION, "Topology could not be parsed. This is the end...");
			return ERROR_TOPOLOGY_PARSE;
		}

		mpi_util_send_topology(topology);
		topology_destroy(&topology);	/* TODO Makes topology A-Changin, so no need to destroy is here */
		parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "Topology sent to executors. I don't need it anymore. Destroy!");

		populations = (population_t **)malloc((world_size - 1) * sizeof(population_t *));
		if (populations == NULL) {
			parallel_evolution_log(SEVERITY_ERROR, MODULE_PARALLEL_EVOLUTION, "Fail to allocate the array of populations. Quit.");
			return ERROR_POPULATIONS_ALLOC;
		}

		parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "Waiting resultant populations...");
		for (i = 1; i <= world_size - 1; ++i) {
			sprintf(log_msg, "Receiving population from process %d...", i);
			parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, log_msg);

			mpi_util_recv_population(i, populations);

			sprintf(log_msg, "Population from process %d received.", i);
			parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, log_msg);
		}
		
		parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "All populations received.");
		report_results(populations, world_size - 1);

	} else {	/* algorithm executor */
		parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "I am an algorithm executor!");
		if (processes_get_algorithm(parallel_evolution.processes, &algorithm, rank) != SUCCESS) {
			parallel_evolution_log(SEVERITY_ERROR, MODULE_PARALLEL_EVOLUTION, "Could not get the algorithm. Quit...");
			return ERROR_PROCESSES_GET_ALGORITHM;
		}
		algorithm->init();
		parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "Algorithm initialized.");
		migrant_create(&migrant, parallel_evolution.number_of_dimensions);
		while (1) {
			mpi_util_recv_adjacency_list(&adjacency_array, &adjacency_array_size);
			parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "Adjacency list received.");

			algorithm->run_iterations(MIGRATION_INTERVAL);
			sprintf(log_msg, "Algorithm has runned for %d iterations.", MIGRATION_INTERVAL);
			parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, log_msg);

			while (mpi_util_recv_migrant(migrant) == SUCCESS) {
				algorithm->insert_migrant(migrant);
				parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "Migrant inserted into local population.");
			}
			algorithm->pick_migrant(migrant);
			parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "Migrant picked up from local population to send to other processes.");
			mpi_util_send_migrant(migrant, adjacency_array, adjacency_array_size);
			if (algorithm->ended()) {
				parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "Algorithm ended.");
				algorithm->get_population(&my_population);
				parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "Population ready to send.");
				mpi_util_send_population(my_population);
				break;
			}
		}
	}

	parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "MPI will be finalized.");
	MPI_Finalize();
	return 0;
}

void parallel_evolution_set_topology_file_name(const char *file_name)
{
	const char log_msg[256];

	parallel_evolution.topology_file_name = file_name;

	sprintf(log_msg, "Topology file name set to \"%s\".", file_name);
	parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, log_msg);
}

void parallel_evolution_set_number_of_dimensions(int number_of_dimensions)
{
	const char log_msg[256];

	parallel_evolution.number_of_dimensions = number_of_dimensions;

	sprintf(log_msg, "Number of dimensions set to %d.", number_of_dimensions);
	parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, log_msg);
}

void parallel_evolution_create_processes(int number_of_islands)
{
	status_t ret;
	int world_size;
	
	world_size = number_of_islands + 1;
	ret = processes_create(&(parallel_evolution.processes), world_size - 1);

	if (ret != SUCCESS) {
		parallel_evolution_log(SEVERITY_ERROR, MODULE_PARALLEL_EVOLUTION, "Processes struct could not be created.");
		exit(ERROR_PROCESSES_CREATE);
	}

	parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "Processes struct created.");
}

void parallel_evolution_add_algorithm(algorithm_t *algorithm, int first_rank, int last_rank)
{
	const char log_msg[256];

	processes_add_algorithm(parallel_evolution.processes, algorithm, first_rank, last_rank);

	sprintf(log_msg, "Algorithm added to processes from %d to %d.", first_rank, last_rank);
	parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, log_msg);
}

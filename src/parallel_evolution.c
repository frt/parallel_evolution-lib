#include "config.h"
#include "parallel_evolution.h"
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

int parallel_evolution_run(int *argc, char ***argv)
{
	int rank, world_size;
	int i;
	population_t **populations;
	algorithm_t *algorithm;
	migrant_t *migrant;
	population_t *my_population;
	int *adjacency_array = NULL;
	int adjacency_array_size;
	processes_t *processes;
	char log_msg[256];
	int converged = 0;
	int stop_sending = 0;
	int done_count = 0;
	int done_rank;

	MPI_Init(argc, argv);
	parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "MPI inicializado.");

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	sprintf (log_msg, "I am process %d of %d.", rank, world_size);
	parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, log_msg);

	if (rank == 0) {	/* topology controller */
		parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "I am the master of topologies!");

		mpi_util_send_topology(parallel_evolution.topology);	/* TODO Makes topology A-Changin and a change detector here */
		parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "Topology sent to executors. I don't need it anymore. Destroy!");

		populations = (population_t **)malloc((world_size - 1) * sizeof(population_t *));
		if (populations == NULL) {
			parallel_evolution_log(SEVERITY_ERROR, MODULE_PARALLEL_EVOLUTION, "Fail to allocate the array of populations. Quit.");
			return ERROR_POPULATIONS_ALLOC;
		}

		parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "Waiting for convergence...");
		while (done_count < world_size - 1) {
			done_rank = mpi_util_recv_report_done();
			if (done_rank != 0) {
				++done_count;
				sprintf(log_msg, "Received report_done from process %d...", done_rank);
				parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, log_msg);
			}
		}

		parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "Sending \"stop_sending\" notifications...");
		mpi_util_send_stop_sending();

		parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "Sending \"finalize\" notifications...");
		mpi_util_send_finalize();

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
			/* receive migrants */
			while (mpi_util_recv_migrant(migrant) == SUCCESS) {
				algorithm->insert_migrant(migrant);
				parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "Migrant inserted into local population.");
			}

			/* run algorithm */
			algorithm->run_iterations(parallel_evolution.migration_interval);
			sprintf(log_msg, "Algorithm has runned for %d iterations.", parallel_evolution.migration_interval);
			parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, log_msg);

			/* will need the adjacency array before sending migrants */
			if (mpi_util_recv_adjacency_list(&adjacency_array, &adjacency_array_size) == SUCCESS)
				parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION,
						"Adjacency list received.");

			/* send migrant */
			if (adjacency_array != NULL && !stop_sending) {
				algorithm->pick_migrant(migrant);
				parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "Migrant picked up from local population to send to other processes.");
				mpi_util_send_migrant(migrant, adjacency_array, adjacency_array_size);
				stop_sending = mpi_util_recv_stop_sending();
			}

			/* report to master that the algorithm has converged */
			if (!converged && algorithm->ended()) {
				parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "Algorithm ended.");
				mpi_util_send_report_done();
				converged = !converged;
			}

			/* if "finalize" msg received, send population and finalize */
			if (converged && mpi_util_recv_finalize()) {
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

void parallel_evolution_set_topology(topology_t *topology)
{
	parallel_evolution.topology = topology;
	parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, "Topology set.");
}

void parallel_evolution_set_number_of_dimensions(int number_of_dimensions)
{
	char log_msg[256];

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
	char log_msg[256];

	processes_add_algorithm(parallel_evolution.processes, algorithm, first_rank, last_rank);

	sprintf(log_msg, "Algorithm added to processes from %d to %d.", first_rank, last_rank);
	parallel_evolution_log(SEVERITY_DEBUG, MODULE_PARALLEL_EVOLUTION, log_msg);
}

void parallel_evolution_set_migration_interval(int iterations)
{
	parallel_evolution.migration_interval = iterations;
}

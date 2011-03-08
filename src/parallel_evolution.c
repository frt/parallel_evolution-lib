#include "parallel_evolution.h"

#include <mpi.h>
#include "mpi_util.h"

#define MIGRATION_INTERVAL 100	/* XXX */

/* error codes */
#define ERROR_TOPOLOGY_CREATE 1
#define ERROR_TOPOLOGY_PARSE 2

parallel_evolution_t parallel_evolution;

int parallel_evolution_run(int *argc, char ***argv)
{
	int rank, size;
	int i;
	population_t **populations;
	algorithm_t *algorithm;
	migrant_t migrant, my_migrant;
	population_t *my_population;
	topology_t *topology;
	int *adjacency_array;

	MPI_Init(argc, argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	if (rank == 0) {	/* topology controller */
		/* create the topology */
		if (topology_create(&topology) != SUCCESS)
			return ERROR_TOPOLOGY_CREATE;

		/* parse topology from file */
		if (parallel_evolution_parse_topology(topology, parallel_evolution.topology_file_name) != SUCCESS)
			return ERROR_TOPOLOGY_PARSE;

		mpi_util_send_topology(topology);
		for (i = 0; i < size; ++i)
			mpi_util_recv_population(i, populations);
		report_results(populations);	/* TODO */
	} else {	/* algorithm executor */
		while (1) {
			parallel_evolution_get_algorithm(&algorithm, rank);	/* TODO */
			algorithm->init();
			mpi_util_recv_adjacency_list(&adjacency_array);
			algorithm->run_iterations(MIGRATION_INTERVAL);
			if (mpi_util_recv_migrant(&migrant))
				algorithm->insert_migrant(&migrant);
			algorithm->pick_migrant(&my_migrant);
			mpi_util_send_migrant(my_migrant);
			if (algorithm->ended()) {
				algorithm->get_population(&my_population);
				mpi_util_send_population(my_population);
				break;
			}
		}
	}

	MPI_Finalize();
	return 0;
}

status_t parallel_evolution_parse_topology(topology_t *topology, const char *file_name)
{
	return topology_parser_parse(topology, file_name);
}

void parallel_evolution_set_topology_file_name(const char *file_name)
{
	parallel_evolution.topology_file_name = file_name;
}

void parallel_evolution_set_dimensions(int number_of_dimensions)
{
	parallel_evolution.number_of_dimensions = number_of_dimensions;
}

#include "parallel_evolution.h"

#define MIGRATION_INTERVAL 100	/* FIXME */

int parallel_evolution_run(int *argc, char ***argv)
{
	int rank, size;
	topology_t *topology;
	int i;
	population_t *populations[];
	algorithm_t *algorithm;
	migrant_t *migrant, *my_migrant;

	MPI_Init(argc, argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	if (rank == 0) {	/* topology controller */
		mpi_util_send_topology(parallell_evolution->topology);	/* TODO */
		for (i = 0; i < size; ++i)
			mpi_util_recv_population(i, populations);	/* TODO */
		report_results(populations);	/* TODO */
	} else {	/* algorithm executor */
		while (1) {
			parallel_evolution_get_algorithm(&algorithm, rank);	/* TODO */
			algorithm_init(algorithm);	/* TODO */
			mpi_util_recv_adjacency_list(...);	/* TODO */
			algorithm_run(algorithm, MIGRATION_INTERVAL);	/* TODO */
			if (mpi_util_recv_migrant(&migrant))	/* TODO non-blocking */
				algorithm_insert_migrant(algorithm, migrant);	/* TODO */
			algorithm_pick_migrant(algorithm, my_migrant);
			mpi_util_send_migrant(my_migrant);	/* TODO non-blocking MPI_Isend() */
			if (algorithm_ended()) {	/* TODO */
				algorithm_get_population();	/* TODO */
				mpi_util_send_population();	/* TODO */
				break;
			}
		}
	}

	MPI_Finalize();
	return 0;
}

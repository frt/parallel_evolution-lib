#include "topology.h"
#include "topology_parser.h"
#include "mpi_util.h"
#include "report.h"
#include "parallel_evolution.h"	/* maybe this will be the façade for all parallel_evolution library in the future */

#include <bfo.h>

#define MIGRATION_INTERVAL

int main(int argc, char *argv[])
{
	int rank, size;
	topology_t *topology;
	int i;
	population_t *populations[];

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	if (rank == 0) {	/* topology controller */
		topology_parser_parse(&topology, "ring.topology");	/* TODO */
		mpi_util_send_topology(topology);	/* TODO */
		for (i = 0; i < size; ++i)
			mpi_util_recv_population(i, populations);	/* TODO */
		report_results(populations);	/* TODO */
	} else {	/* algorithm executor */
		/* TODO */
		parallel_evolution_set_algorithm(
				bfo_init,
				bfo_run_iterations,
				bfo_insert_migrant,
				bfo_colect_migrant,
				bfo_ended(),
				bfo_get_population
				);

		while (1) {
			algorithm_init();	/* TODO */
			mpi_util_recv_adjacency_list();	/* TODO */
			algorithm_run(MIGRATION_INTERVAL);	/* TODO */
			if (mpi_util_recv_migrant())	/* TODO non-blocking */
				algorithm_insert(migrant);	/* TODO */
			algorithm_colect(my_migrant);
			mpi_util_send_migrant();	/* TODO non-blocking MPI_Isend() */
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

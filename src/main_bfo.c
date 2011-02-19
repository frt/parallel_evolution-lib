#include "topology.h"
#include "topology_parser.h"
#include "mpi_util.h"
#include "report.h"
#include "parallel_evolution.h"	/* maybe this will be the façade for all parallel_evolution library in the future */

#include <bfo.h>

#define MIGRATION_INTERVAL

int main(int argc, char *argv[])
{
	algorithm_t *bfo_algorithm;	/* TODO */

	parallel_evolution_parse_topology("ring.topology");	/* TODO */
	/* TODO */
	algorithm_create(&bfo_algorithm,
			bfo_init,
			bfo_run_iterations,
			bfo_insert_migrant,
			bfo_colect_migrant,
			bfo_ended(),
			bfo_get_population
			);
	parallel_evolution_add_algorithm(bfo_algorithm, 1, 3);	/* TODO */
	return parallel_evolution_run();
}

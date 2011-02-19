#include "parallel_evolution.h"

#include <bfo.h>

int main(int argc, char *argv[])
{
	algorithm_t *bfo_algorithm;	/* TODO */

	parallel_evolution_parse_topology("ring.topology");	/* TODO */
	/* TODO */
	algorithm_create(&bfo_algorithm,
			bfo_init,
			bfo_run_iterations,
			bfo_insert_migrant,
			bfo_pick_migrant,
			bfo_ended,
			bfo_get_population
			);
	parallel_evolution_add_algorithm(bfo_algorithm, 1, 3);	/* TODO */
	return parallel_evolution_run();	/* TODO */
}

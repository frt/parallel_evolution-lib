#include "parallel_evolution.h"

#include <bfo.h>

int main(int argc, char *argv[])
{
	algorithm_t *bfo_algorithm;
	int ret;

	parallel_evolution_set_topology_file_name("ring.topology");
	algorithm_create(&bfo_algorithm,
			bfo_init,	/* TODO */
			bfo_run_iterations,	/* TODO */
			bfo_insert_migrant,	/* TODO */
			bfo_pick_migrant,	/* TODO */
			bfo_ended,	/* TODO */
			bfo_get_population	/* TODO */
			);
	parallel_evolution_add_algorithm(bfo_algorithm, 1, 3);	/* TODO */
	ret = parallel_evolution_run();	/* TODO */
	algorithm_destroy(&bfo_algorithm);
	
	return ret;
}

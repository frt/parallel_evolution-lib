#include "parallel_evolution.h"

#include "../bfo/include/bfo_parallel_evolution.h"

int main(int argc, char *argv[])
{
	algorithm_t *bfo_algorithm;
	int ret;

	parallel_evolution_set_number_of_dimensions(50);
	parallel_evolution_set_topology_file_name("ring.topology");
	algorithm_create(&bfo_algorithm,
			bfo_init,
			bfo_run_iterations,
			bfo_insert_migrant,
			bfo_pick_migrant,
			bfo_ended,
			bfo_get_population
			);
	parallel_evolution_create_processes(3);
	parallel_evolution_add_algorithm(bfo_algorithm, 1, 3);
	ret = parallel_evolution_run(&argc, &argv);
	algorithm_destroy(&bfo_algorithm);
	
	return ret;
}

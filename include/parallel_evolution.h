#pragma once

#include "common.h"
#include "topology.h"
#include "topology_parser.h"
#include "population.h"
#include "algorithm.h"
#include "processes.h"

typedef struct parallel_evolution {
	/* used on topology controller */
	const char *topology_file_name;
	int number_of_dimensions;

	/* used on algorithm runners */
	algorithm_t **algorithms;
	processes_t *processes;
} parallel_evolution_t;

extern parallel_evolution_t parallel_evolution;

void parallel_evolution_set_topology_file_name(const char *file_name);
void parallel_evolution_set_number_of_dimensions(int number_of_dimensions);
void parallel_evolution_create_processes(int number_of_islands);
void parallel_evolution_add_algorithm(algorithm_t *algorithm, int first_rank, int last_rank);

int parallel_evolution_run(int *argc, char ***argv);

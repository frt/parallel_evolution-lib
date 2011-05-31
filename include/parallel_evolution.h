/**
 * This is a includes all header for the parallel_evolution-lib.
 *
 * Programs using the parallel_evolution-lib should include only this one.
 */
#pragma once

#include "parallel_evolution/common.h"
#include "parallel_evolution/topology.h"
#include "parallel_evolution/topology_parser.h"
#include "parallel_evolution/population.h"
#include "parallel_evolution/algorithm.h"
#include "parallel_evolution/processes.h"
#include "parallel_evolution/log.h"

typedef struct parallel_evolution {
	/* used on topology controller */
	const char *topology_file_name;
	int number_of_dimensions;

	/* used on algorithm runners */
	algorithm_t **algorithms;
	processes_t *processes;
} parallel_evolution_t;

extern parallel_evolution_t parallel_evolution;

void parallel_evolution_set_topology_file_name(const char *file_name);	/* FIXME topology parser should not be in the lib */
void parallel_evolution_set_number_of_dimensions(int number_of_dimensions);
void parallel_evolution_create_processes(int number_of_islands);
void parallel_evolution_add_algorithm(algorithm_t *algorithm, int first_rank, int last_rank);

int parallel_evolution_run(int *argc, char ***argv);

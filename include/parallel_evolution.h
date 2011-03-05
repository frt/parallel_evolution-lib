#pragma once

#include "common.h"
#include "topology.h"
#include "topology_parser.h"
#include "population.h"
#include "algorithm.h"

typedef struct parallel_evolution {
	/* used on topology controller */
	const char *topology_file_name;

	/* used on algorithm runners */
	algorithm_t **algorithms;
} parallel_evolution_t;

extern parallel_evolution_t parallel_evolution;

void parallel_evolution_set_topology_file_name(const char *file_name);
int parallel_evolution_run(int *argc, char ***argv);
status_t parallel_evolution_parse_topology(topology_t *topology, const char *file_name);
#pragma once

#include "common.h"
#include "topology.h"
#include "topology_parser.h"
#include "population.h"
#include "algorithm.h"

typedef struct parallel_evolution_config {
	/* used on topology controller */
	topology_t *topology;

	/* used on algorithm runners */
	algorithm_t *algorithm;
} parallel_evolution_config_t;

int parallel_evolution_run(int *argc, char ***argv);
status_t parallel_evolution_parse_topology(const char *file_name);

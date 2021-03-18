/**
 * This is a includes all header for the parallel_evolution-lib.
 *
 * Programs using the parallel_evolution-lib should include only this one.
 */
#pragma once

#include <libconfig.h>
#include "parallel_evolution/common.h"
#include "parallel_evolution/topology.h"
#include "parallel_evolution/population.h"
#include "parallel_evolution/algorithm.h"
#include "parallel_evolution/log.h"

typedef struct limit {
    double min;
    double max;
} limit_t;

typedef struct parallel_evolution {
	topology_t *topology;
	int number_of_dimensions;
    limit_t *limits;
	algorithm_t *algorithm;
	int migration_interval;
} parallel_evolution_t;

extern parallel_evolution_t parallel_evolution;

void parallel_evolution_set_topology(topology_t *topology);
void parallel_evolution_set_number_of_dimensions(int number_of_dimensions);
void parallel_evolution_create_processes(int number_of_islands);
void parallel_evolution_set_algorithm(algorithm_t *algorithm);
void parallel_evolution_set_migration_interval(int);

int parallel_evolution_run(int *argc, char ***argv);

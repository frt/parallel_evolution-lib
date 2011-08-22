/**
 * This is a includes all header for the parallel_evolution-lib.
 *
 * Programs using the parallel_evolution-lib should include only this one.
 */
#pragma once

#include "parallel_evolution/common.h"
#include "parallel_evolution/topology.h"
#include "parallel_evolution/population.h"
#include "parallel_evolution/algorithm.h"
#include "parallel_evolution/processes.h"
#include "parallel_evolution/log.h"

typedef struct parallel_evolution {
	/* used on topology controller */
	topology_t *topology;
	int number_of_dimensions;

	/**
	 * Pointer to function that receives a topology and returns a set of topology_operations.
	 *
	 * \param[in] topology Topology with stats.
	 * \param[out] topology_operation_array Changes to make to the topology.
	 * \param[out] number_of_operations Size of the array of changes.
	 */
	void (*topology_changer)(topology_t *topology, topology_operation_t **topology_operation_array, 
			int *number_of_operations);

	/* used on algorithm runners */
	algorithm_t **algorithms;
	processes_t *processes;
	int migration_interval;
} parallel_evolution_t;

extern parallel_evolution_t parallel_evolution;

void parallel_evolution_set_topology(topology_t *topology);
void parallel_evolution_set_number_of_dimensions(int number_of_dimensions);
void parallel_evolution_create_processes(int number_of_islands);
void parallel_evolution_add_algorithm(algorithm_t *algorithm, int first_rank, int last_rank);
void parallel_evolution_set_migration_interval(int);

int parallel_evolution_run(int *argc, char ***argv);

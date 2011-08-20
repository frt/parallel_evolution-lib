#pragma once

#include "topology.h"
#include "population.h"
#include "common.h"
#include "migrant.h"

void mpi_util_send_topology(topology_t* topology);
status_t mpi_util_recv_population(int rank, population_t *populations[]);
status_t mpi_util_send_population(population_t *population);
status_t mpi_util_recv_adjacency_list(int **adjacency_array, int *adjacency_array_size);
status_t mpi_util_recv_migrant(migrant_t *migrant);
status_t mpi_util_send_migrant(migrant_t *migrant, int *adjacency_array, int adjacency_array_size);
status_t mpi_util_send_stats(algorithm_stats_t *algorithm_stats);

/**
 * Receive algorithm_stats from a algorithm executor.
 *
 * \param[out] algorithm_stats The var that will receive the stats.
 * \param[out] rank The rank of the processes that sended the stats.
 * \return Status: FAIL if could not get any stats, SUCCESS if it could.
 */
status_t mpi_util_recv_stats(algorithm_stats_t *algorithm_stats, int *rank);

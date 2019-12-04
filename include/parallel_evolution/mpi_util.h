#pragma once

#include "topology.h"
#include "population.h"
#include "common.h"
#include "migrant.h"

void mpi_util_send_topology(topology_t* topology);
status_t mpi_util_recv_population(int rank, population_t *populations[]);
status_t mpi_util_send_population(population_t *population);

/**
 * Blocking receive of adjacency list.
 *
 * \param[out] adjacency_array The list of adjacent nodes will be put here.
 * \param[out] adjacency_array_size The number of adjacent nodes wil be put here.
 * \return Status: FAIL if it couldn't receive the ajacency list, SUCCESS if it could.
 */
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

int mpi_util_recv_report_done();
void mpi_util_send_stop_sending();
void mpi_util_send_finalize();
int mpi_util_recv_stop_sending();
void mpi_util_send_report_done();
int mpi_util_recv_finalize();

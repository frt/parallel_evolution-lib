#pragma once

#include "topology.h"
#include "population.h"
#include "common.h"

void mpi_util_send_topology(topology_t* topology);
status_t mpi_util_recv_population(int rank, population_t *populations[]);
status_t mpi_util_send_population(population_t *population);

/* TODO
src/parallel_evolution.c:                       mpi_util_recv_adjacency_list();
src/parallel_evolution.c:                       if (mpi_util_recv_migrant(&migrant))
src/parallel_evolution.c:                       mpi_util_send_migrant(my_migrant);
*/

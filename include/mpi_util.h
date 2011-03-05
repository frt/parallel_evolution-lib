#pragma once

#include "topology.h"

void mpi_util_send_topology(topology_t* topology);

/* TODO
src/parallel_evolution.c:                       mpi_util_recv_population(i, populations);
src/parallel_evolution.c:                       mpi_util_recv_adjacency_list();
src/parallel_evolution.c:                       if (mpi_util_recv_migrant(&migrant))
src/parallel_evolution.c:                       mpi_util_send_migrant(my_migrant);
src/parallel_evolution.c:                               mpi_util_send_population();
*/

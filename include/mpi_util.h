#pragma once

#include <mpi.h>

src/parallel_evolution.c:               mpi_util_send_topology(topology);       /* TODO */
src/parallel_evolution.c:                       mpi_util_recv_population(i, populations);       /* TODO */
src/parallel_evolution.c:                       mpi_util_recv_adjacency_list(/* ... */);        /* TODO */
src/parallel_evolution.c:                       if (mpi_util_recv_migrant(&migrant))    /* TODO non-blocking */
src/parallel_evolution.c:                       mpi_util_send_migrant(my_migrant);      /* TODO non-blocking MPI_Isend() */
src/parallel_evolution.c:                               mpi_util_send_population();     /* TODO */


#include "mpi_util.h"
#include "common.h"
#include <mpi.h>

#define TAG_TOPOLOGY 1

void mpi_util_send_topology(topology_t* topology)
{
	int node_id;
	int *adjacency_array;
	int adjacency_array_size;
	status_t ret;

	ret = topology_get_first_node(topology, &node_id, &adjacency_array, &adjacency_array_size);
	while (ret == SUCCESS) {
		MPI_Send(adjacency_array, adjacency_array_size, MPI_INT, node_id,
				    TAG_TOPOLOGY, MPI_COMM_WORLD);
		ret = topology_get_next_node(topology, &node_id, &adjacency_array, &adjacency_array_size);
	}
}

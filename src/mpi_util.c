#include "mpi_util.h"
#include <mpi.h>
#include <stdlib.h>
#include "migrant.h"
#include "parallel_evolution.h"

#define TAG_TOPOLOGY 1
#define TAG_POPULATION_SIZE 2
#define TAG_POPULATION 3

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

status_t mpi_util_recv_popularion_array(int population_size, double **msg_array, int rank)
{
	int msg_size;

	/* receive the raw msg, an array of doubles */
	msg_size = population_size * parallel_evolution.number_of_dimensions;
	*msg_array = (double *)malloc(msg_size * sizeof(double));
	if (*msg_array == NULL)
		return FAIL;
	MPI_Recv(*msg_array, msg_size, MPI_DOUBLE, rank, TAG_POPULATION, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

status_t mpi_util_recv_population(int rank, population_t *populations[])
{
	int population_size;
	population_t *recv_population;
	migrant_t *new_migrant;
	int i, j;
	double *msg_array;

	/* receive the number of migrants in the population */
	MPI_Recv(&population_size, 1, MPI_INT, rank, TAG_POPULATION_SIZE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	if (mpi_util_recv_popularion_array(population_size, &msg_array, rank) != SUCCESS)
		return FAIL;

	/* assign values to population structure */
	if (population_create(&recv_population, population_size) != SUCCESS)
		return FAIL;
	for (i = 0; i < population_size; ++i) {
		if (migrant_create(&new_migrant, parallel_evolution.number_of_dimensions) != SUCCESS)
			return FAIL;
		for (j = 0; j < parallel_evolution.number_of_dimensions; ++j) {
			new_migrant->var[j] = msg_array[i * parallel_evolution.number_of_dimensions + j];
		}
		population_set_individual(recv_population, new_migrant, i);
	}

	free(msg_array);

	populations[rank - 1] = recv_population;

	return SUCCESS;
}

status_t mpi_util_send_population(population_t *population)
{
	int msg_size;
	double *msg_array;
	int i, j, k;

	MPI_Send(&(population->size), 1, MPI_INT, 0, TAG_POPULATION_SIZE, MPI_COMM_WORLD);
	
	msg_size = population->size * parallel_evolution.number_of_dimensions;
	msg_array = (double *)malloc(msg_size * sizeof(double));
	if (msg_array == NULL)
		return FAIL;
	for (i = 0; i < population->size; ++i) {
		for (j = 0; j < parallel_evolution.number_of_dimensions; ++j) {
			msg_array[i * parallel_evolution.number_of_dimensions + j] = population->individuals[i]->var[j];
		}
	}
	MPI_Send(msg_array, msg_size, MPI_DOUBLE, 0, TAG_POPULATION, MPI_COMM_WORLD);
	free(msg_array);
}

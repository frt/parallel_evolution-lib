#include "config.h"
#include "parallel_evolution/mpi_util.h"
#if HAVE_MPI_H 
	#include <mpi.h>
#elif HAVE_MPI_MPI_H
	#include <mpi/mpi.h>
#endif
#include <stdlib.h>
#include "parallel_evolution.h"
#include "parallel_evolution/log.h"

#define TAG_ADJACENCY_SIZE 1
#define TAG_ADJACENCY 2
#define TAG_POPULATION_SIZE 3
#define TAG_POPULATION 4
#define TAG_MIGRANT 5

void mpi_util_send_topology(topology_t* topology)
{
	int node_id;
	int *adjacency_array;
	int adjacency_array_size;
	status_t ret;

	ret = topology_get_first_node(topology, &node_id, &adjacency_array, &adjacency_array_size);
	while (ret == SUCCESS) {
		MPI_Send(&adjacency_array_size, 1, MPI_INT, node_id,
				    TAG_ADJACENCY_SIZE, MPI_COMM_WORLD);
		parallel_evolution_log(SEVERITY_DEBUG, MODULE_MPI_UTIL, "Adjacency array size sent.");
		MPI_Send(adjacency_array, adjacency_array_size, MPI_INT, node_id,
				    TAG_ADJACENCY, MPI_COMM_WORLD);
		parallel_evolution_log(SEVERITY_DEBUG, MODULE_MPI_UTIL, "Adjacency array sent.");
		ret = topology_get_next_node(topology, &node_id, &adjacency_array, &adjacency_array_size);
	}
}

status_t mpi_util_recv_population_array(int population_size, double **msg_array, int rank)
{
	int msg_size;

	/* receive the raw msg, an array of doubles */
	msg_size = population_size * parallel_evolution.number_of_dimensions;
	*msg_array = (double *)malloc(msg_size * sizeof(double));
	if (*msg_array == NULL) {
		parallel_evolution_log(SEVERITY_ERROR, MODULE_MPI_UTIL, "Population array could not be allocated.");
		return FAIL;
	}
	MPI_Recv(*msg_array, msg_size, MPI_DOUBLE, rank, TAG_POPULATION, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	parallel_evolution_log(SEVERITY_DEBUG, MODULE_MPI_UTIL, "Population array received.");

	return SUCCESS;
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
	parallel_evolution_log(SEVERITY_DEBUG, MODULE_MPI_UTIL, "Population size received.");

	if (mpi_util_recv_population_array(population_size, &msg_array, rank) != SUCCESS) {
		parallel_evolution_log(SEVERITY_ERROR, MODULE_MPI_UTIL, "Population array not received.");
		return FAIL;
	}

	/* assign values to population structure */
	if (population_create(&recv_population, population_size) != SUCCESS) {
		parallel_evolution_log(SEVERITY_ERROR, MODULE_MPI_UTIL, "Population struct could not be created.");
		return FAIL;
	}
	for (i = 0; i < population_size; ++i) {
		if (migrant_create(&new_migrant, parallel_evolution.number_of_dimensions) != SUCCESS) {
			parallel_evolution_log(SEVERITY_ERROR, MODULE_MPI_UTIL, "Migrant struct could not be created.");
			return FAIL;
		}
		for (j = 0; j < parallel_evolution.number_of_dimensions; ++j) {
			new_migrant->var[j] = msg_array[i * parallel_evolution.number_of_dimensions + j];
		}
		population_set_individual(recv_population, new_migrant, i);
	}

	free(msg_array);

	populations[rank - 1] = recv_population;

	parallel_evolution_log(SEVERITY_DEBUG, MODULE_MPI_UTIL, "Population struct received.");
	return SUCCESS;
}

status_t mpi_util_send_population(population_t *population)
{
	int msg_size;
	double *msg_array;
	int i, j, k;

	MPI_Send(&(population->size), 1, MPI_INT, 0, TAG_POPULATION_SIZE, MPI_COMM_WORLD);
	parallel_evolution_log(SEVERITY_DEBUG, MODULE_MPI_UTIL, "Population size sent.");
	
	msg_size = population->size * parallel_evolution.number_of_dimensions;
	msg_array = (double *)malloc(msg_size * sizeof(double));
	if (msg_array == NULL) {
		parallel_evolution_log(SEVERITY_ERROR, MODULE_MPI_UTIL, "Population array could not be allocated.");
		return FAIL;
	}
	for (i = 0; i < population->size; ++i) {
		for (j = 0; j < parallel_evolution.number_of_dimensions; ++j) {
			msg_array[i * parallel_evolution.number_of_dimensions + j] = population->individuals[i]->var[j];
		}
	}
	MPI_Send(msg_array, msg_size, MPI_DOUBLE, 0, TAG_POPULATION, MPI_COMM_WORLD);
	free(msg_array);

	parallel_evolution_log(SEVERITY_DEBUG, MODULE_MPI_UTIL, "Population array sent.");
	return SUCCESS;
}

status_t mpi_util_recv_adjacency_list(int **adjacency_array, int *adjacency_array_size)
{
	int has_msg = 0;

	parallel_evolution_log(SEVERITY_DEBUG, MODULE_MPI_UTIL, "Probing for adjacency list to receive...");
	MPI_Iprobe(0, TAG_ADJACENCY_SIZE, MPI_COMM_WORLD, &has_msg,
			MPI_STATUS_IGNORE);
	if (!has_msg) {
		parallel_evolution_log(SEVERITY_DEBUG, MODULE_MPI_UTIL, "There's no adjacency list to receive.");
		return FAIL;
	}
	
	MPI_Recv(adjacency_array_size, 1, MPI_INT, 0, TAG_ADJACENCY_SIZE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	parallel_evolution_log(SEVERITY_DEBUG, MODULE_MPI_UTIL, "Adjacency size received.");
	
	if (*adjacency_array != NULL) {
		free(*adjacency_array);
		*adjacency_array = NULL;
		parallel_evolution_log(SEVERITY_DEBUG, MODULE_MPI_UTIL, "Previous adjacency array deallocated.");
	}
	
	*adjacency_array = (int *)malloc(*adjacency_array_size * sizeof(int));
	if (adjacency_array == NULL) {
		parallel_evolution_log(SEVERITY_ERROR, MODULE_MPI_UTIL, "Adjacency array could not be allocated.");
		return FAIL;
	}
	
	MPI_Recv(*adjacency_array, *adjacency_array_size, MPI_INT, 0, TAG_ADJACENCY, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	parallel_evolution_log(SEVERITY_DEBUG, MODULE_MPI_UTIL, "Adjacency array received.");

	return SUCCESS;
}

status_t mpi_util_recv_migrant(migrant_t *migrant)
{
	double *migrant_array;
	int i;
	int has_msg = 0;
	MPI_Status status;

	parallel_evolution_log(SEVERITY_DEBUG, MODULE_MPI_UTIL, "Probing for migrant to receive...");
	MPI_Iprobe(MPI_ANY_SOURCE, TAG_MIGRANT, MPI_COMM_WORLD, &has_msg, &status);
	if (!has_msg) {
		parallel_evolution_log(SEVERITY_DEBUG, MODULE_MPI_UTIL, "There's no migrant to receive.");
		return FAIL;
	}
	parallel_evolution_log(SEVERITY_DEBUG, MODULE_MPI_UTIL, "There is a migrant to receive!");

	migrant_array = (double *)malloc(parallel_evolution.number_of_dimensions * sizeof(double));
	if (migrant_array == NULL) {
		parallel_evolution_log(SEVERITY_ERROR, MODULE_MPI_UTIL, "Migrant array could not be allocated.");
		return FAIL;
	}
	MPI_Recv(migrant_array, parallel_evolution.number_of_dimensions, MPI_DOUBLE, status.MPI_SOURCE, TAG_MIGRANT,
			MPI_COMM_WORLD,	&status);
	parallel_evolution_log(SEVERITY_DEBUG, MODULE_MPI_UTIL, "Migrant array received.");

	for (i = 0; i < parallel_evolution.number_of_dimensions; ++i) {
		migrant->var[i] = migrant_array[i];
	}

	free(migrant_array);

	parallel_evolution_log(SEVERITY_DEBUG, MODULE_MPI_UTIL, "Migrant received.");
	return SUCCESS;
}

status_t mpi_util_send_migrant(migrant_t *migrant, int *adjacency_array, int adjacency_array_size)
{
	double *migrant_array;
	int i;
	int rank;

	migrant_array = (double *)malloc(parallel_evolution.number_of_dimensions * sizeof(double));
	if (migrant_array == NULL) {
		parallel_evolution_log(SEVERITY_ERROR, MODULE_MPI_UTIL, "Migrant array could not be allocated.");
		return FAIL;
	}

	for (i = 0; i < parallel_evolution.number_of_dimensions; ++i) {
		migrant_array[i] = migrant->var[i];
	}

	for (i = 0; i < adjacency_array_size; ++i) {
		rank = adjacency_array[i];
		MPI_Send(migrant_array, parallel_evolution.number_of_dimensions, MPI_DOUBLE, rank, TAG_MIGRANT, MPI_COMM_WORLD);
		parallel_evolution_log(SEVERITY_DEBUG, MODULE_MPI_UTIL, "Migrant sent to a destiny.");
	}

	free(migrant_array);

	parallel_evolution_log(SEVERITY_DEBUG, MODULE_MPI_UTIL, "Migrant sent to all destinys.");
	return SUCCESS;
}

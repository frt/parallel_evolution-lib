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
#include <stdio.h>

#define TAG_ADJACENCY_SIZE 1
#define TAG_ADJACENCY 2
#define TAG_POPULATION_SIZE 3
#define TAG_POPULATION 4
#define TAG_MIGRANT 5
#define TAG_REPORT_DONE 6
#define TAG_STATS 9

#define MODULE_MPI_UTIL "mpi_util"

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
		parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_MPI_UTIL, "Adjacency array size sent.");
		MPI_Send(adjacency_array, adjacency_array_size, MPI_INT, node_id,
				    TAG_ADJACENCY, MPI_COMM_WORLD);
		parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_MPI_UTIL, "Adjacency array sent.");
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
		parallel_evolution_log(LOG_PRIORITY_ERR, MODULE_MPI_UTIL, "Population array could not be allocated.");
		return FAIL;
	}
	MPI_Recv(*msg_array, msg_size, MPI_DOUBLE, rank, TAG_POPULATION, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_MPI_UTIL, "Population array received.");

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
	parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_MPI_UTIL, "Population size received.");

	if (mpi_util_recv_population_array(population_size, &msg_array, rank) != SUCCESS) {
		parallel_evolution_log(LOG_PRIORITY_ERR, MODULE_MPI_UTIL, "Population array not received.");
		return FAIL;
	}

	/* assign values to population structure */
	if (population_create(&recv_population, population_size) != SUCCESS) {
		parallel_evolution_log(LOG_PRIORITY_ERR, MODULE_MPI_UTIL, "Population struct could not be created.");
		return FAIL;
	}
	for (i = 0; i < population_size; ++i) {
		if (migrant_create(&new_migrant, parallel_evolution.number_of_dimensions) != SUCCESS) {
			parallel_evolution_log(LOG_PRIORITY_ERR, MODULE_MPI_UTIL, "Migrant struct could not be created.");
			return FAIL;
		}
		for (j = 0; j < parallel_evolution.number_of_dimensions; ++j) {
			new_migrant->var[j] = msg_array[i * parallel_evolution.number_of_dimensions + j];
		}
		if (population_set_individual(recv_population, new_migrant, i) != SUCCESS)
			parallel_evolution_log(LOG_PRIORITY_ERR, MODULE_MPI_UTIL, "Index of new migrant out of bounds. Not setting!");
	}

	free(msg_array);

	populations[rank] = recv_population;

	parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_MPI_UTIL, "Population struct received.");
	return SUCCESS;
}

status_t mpi_util_send_population(population_t *population)
{
	int msg_size;
	double *msg_array;
	int i, j, k;

	MPI_Send(&(population->size), 1, MPI_INT, 0, TAG_POPULATION_SIZE, MPI_COMM_WORLD);
	parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_MPI_UTIL, "Population size sent.");
	
	msg_size = population->size * parallel_evolution.number_of_dimensions;
	msg_array = (double *)malloc(msg_size * sizeof(double));
	if (msg_array == NULL) {
		parallel_evolution_log(LOG_PRIORITY_ERR, MODULE_MPI_UTIL, "Population array could not be allocated.");
		return FAIL;
	}
	for (i = 0; i < population->size; ++i) {
		for (j = 0; j < parallel_evolution.number_of_dimensions; ++j) {
			msg_array[i * parallel_evolution.number_of_dimensions + j] = population->individuals[i]->var[j];
		}
	}
	MPI_Send(msg_array, msg_size, MPI_DOUBLE, 0, TAG_POPULATION, MPI_COMM_WORLD);
	free(msg_array);

	parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_MPI_UTIL, "Population array sent.");
	return SUCCESS;
}

status_t mpi_util_recv_adjacency_list(int **adjacency_array, int *adjacency_array_size)
{
	MPI_Recv(adjacency_array_size, 1, MPI_INT, 0, TAG_ADJACENCY_SIZE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_MPI_UTIL, "Adjacency size received.");
	
	if (*adjacency_array != NULL) {
		free(*adjacency_array);
		*adjacency_array = NULL;
		parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_MPI_UTIL, "Previous adjacency array deallocated.");
	}
	
	*adjacency_array = (int *)malloc(*adjacency_array_size * sizeof(int));
	if (adjacency_array == NULL) {
		parallel_evolution_log(LOG_PRIORITY_ERR, MODULE_MPI_UTIL, "Adjacency array could not be allocated.");
		return FAIL;
	}
	
	MPI_Recv(*adjacency_array, *adjacency_array_size, MPI_INT, 0, TAG_ADJACENCY, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_MPI_UTIL, "Adjacency array received.");

	return SUCCESS;
}

status_t mpi_util_recv_migrant(migrant_t *migrant)
{
	double *migrant_array;
	int i;
	int has_msg = 0;
	MPI_Status status;

	parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_MPI_UTIL, "Probing for migrant to receive...");
	MPI_Iprobe(MPI_ANY_SOURCE, TAG_MIGRANT, MPI_COMM_WORLD, &has_msg, &status);
	if (!has_msg) {
		parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_MPI_UTIL, "There's no migrant to receive.");
		return FAIL;
	}
	parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_MPI_UTIL, "There is a migrant to receive!");

	migrant_array = (double *)malloc(parallel_evolution.number_of_dimensions * sizeof(double));
	if (migrant_array == NULL) {
		parallel_evolution_log(LOG_PRIORITY_ERR, MODULE_MPI_UTIL, "Migrant array could not be allocated.");
		return FAIL;
	}
	MPI_Recv(migrant_array, parallel_evolution.number_of_dimensions, MPI_DOUBLE, status.MPI_SOURCE, TAG_MIGRANT,
			MPI_COMM_WORLD,	&status);
	parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_MPI_UTIL, "Migrant array received.");

	for (i = 0; i < parallel_evolution.number_of_dimensions; ++i) {
		migrant->var[i] = migrant_array[i];
	}

	free(migrant_array);

	parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_MPI_UTIL, "Migrant received.");
	return SUCCESS;
}

status_t mpi_util_send_migrant(migrant_t *migrant, int *adjacency_array, int adjacency_array_size)
{
	double *migrant_array;
	int i;
	int rank;

	migrant_array = (double *)malloc(parallel_evolution.number_of_dimensions * sizeof(double));
	if (migrant_array == NULL) {
		parallel_evolution_log(LOG_PRIORITY_ERR, MODULE_MPI_UTIL, "Migrant array could not be allocated.");
		return FAIL;
	}

	for (i = 0; i < parallel_evolution.number_of_dimensions; ++i) {
		migrant_array[i] = migrant->var[i];
	}

	for (i = 0; i < adjacency_array_size; ++i) {
		rank = adjacency_array[i];
		MPI_Send(migrant_array, parallel_evolution.number_of_dimensions, MPI_DOUBLE, rank, TAG_MIGRANT, MPI_COMM_WORLD);
		parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_MPI_UTIL, "Migrant sent to a destiny.");
	}

	free(migrant_array);

	parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_MPI_UTIL, "Migrant sent to all destinys.");
	return SUCCESS;
}

int mpi_util_recv_tag(int tag, const char *tag_name, int source)
{
	char log_msg[256];
	MPI_Status status;
	int has_msg;

	sprintf(log_msg, "Probing for \"%s\" received...", tag_name);
	parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_MPI_UTIL, log_msg);
	MPI_Iprobe(source, tag, MPI_COMM_WORLD, &has_msg, &status);
	if (has_msg) {
		if (source == MPI_ANY_SOURCE) {
			sprintf (log_msg, "There is a \"%s\" from process %d to receive...", tag_name, status.MPI_SOURCE);
		} else {
			sprintf (log_msg, "There is a \"%s\" to receive...", tag_name);
		}
		parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_MPI_UTIL, log_msg);
		MPI_Recv(NULL, 0, MPI_CHAR, status.MPI_SOURCE, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		sprintf (log_msg, "\"%s\" received.", tag_name);
		parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_MPI_UTIL, log_msg);
	} else {
		sprintf (log_msg, "There's no \"%s\" to receive.", tag_name);
		parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_MPI_UTIL, log_msg);
	}
	return has_msg;
}

int mpi_util_recv_tag_blocking(int tag, const char *tag_name, int source)
{
	char log_msg[256];
	MPI_Status status;

    if (source == MPI_ANY_SOURCE) {
        sprintf (log_msg, "Waiting for a \"%s\" from process %d to receive...", tag_name, source);
    } else {
        sprintf (log_msg, "Waiting for a \"%s\" to receive...", tag_name);
    }
    parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_MPI_UTIL, log_msg);
    MPI_Recv(NULL, 0, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
    sprintf (log_msg, "\"%s\" received.", tag_name);
    parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_MPI_UTIL, log_msg);
    return  status.MPI_SOURCE;
}

void mpi_util_send_tag(int tag, const char *tag_name, int dest)
{
	char log_msg[256];

	sprintf(log_msg, "Sendind \"%s\"...", tag_name);
	parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_MPI_UTIL, log_msg);

	MPI_Send(NULL, 0, MPI_CHAR, dest, tag, MPI_COMM_WORLD);

	sprintf(log_msg, "\"%s\" sended.", tag_name);
	parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_MPI_UTIL, log_msg);
}

int mpi_util_recv_report_done()
{
	return mpi_util_recv_tag_blocking(TAG_REPORT_DONE, "report done", MPI_ANY_SOURCE);
}

void mpi_util_send_report_done()
{
	mpi_util_send_tag(TAG_REPORT_DONE, "report_done", 0);
}

void mpi_util_send_tag_from_master_to_all(int tag, const char *tag_name)
{
	int dest, world_size;

	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	for (dest = 1; dest < world_size; ++dest) {
		mpi_util_send_tag(tag, tag_name, dest);
	}
}

MPI_Datatype *mpi_util_get_algorithm_stats_MPI_type()
{
	static MPI_Datatype *algorithm_stats_MPI_type = NULL;

	MPI_Datatype types[3] = {MPI_INT, MPI_DOUBLE, MPI_DOUBLE};
	int blocklengths[3] = {1, 1, 1};
	MPI_Aint displacements[3];
	int i;
	MPI_Aint struct_start;	/* should be equal to displacement[0], but who knows... */
	algorithm_stats_t algorithm_stats;

	if (algorithm_stats_MPI_type != NULL)
		return algorithm_stats_MPI_type;

	/* allocate a MPI type */
	algorithm_stats_MPI_type = (MPI_Datatype *)malloc(sizeof(MPI_Datatype));
	if (algorithm_stats_MPI_type == NULL)
		return NULL;

	/* create a MPI type */
	MPI_Get_address(&algorithm_stats, &struct_start);
	MPI_Get_address(&algorithm_stats.iterations, &displacements[0]);
	MPI_Get_address(&algorithm_stats.avg_fitness, &displacements[1]);
	MPI_Get_address(&algorithm_stats.best_fitness, &displacements[2]);
	for (i = 2; i >= 0; --i)
		displacements[i] -= struct_start;
	MPI_Type_create_struct(3, blocklengths, displacements, types, algorithm_stats_MPI_type);
	MPI_Type_commit(algorithm_stats_MPI_type);

	return algorithm_stats_MPI_type;
}

status_t mpi_util_send_stats(algorithm_stats_t *algorithm_stats)
{
	MPI_Datatype *algorithm_stats_MPI_type;
	
	algorithm_stats_MPI_type = mpi_util_get_algorithm_stats_MPI_type();
	if (algorithm_stats_MPI_type == NULL)
		return FAIL;

	MPI_Send(algorithm_stats, 1, *algorithm_stats_MPI_type, 0, TAG_STATS, MPI_COMM_WORLD);

	return SUCCESS;
}

status_t mpi_util_recv_stats(algorithm_stats_t *algorithm_stats, int *rank)
{
	MPI_Datatype *algorithm_stats_MPI_type;
	int has_msg = 0;
	MPI_Status status;
	
	algorithm_stats_MPI_type = mpi_util_get_algorithm_stats_MPI_type();
	if (algorithm_stats_MPI_type == NULL)
		return FAIL;

	MPI_Recv(algorithm_stats, 1, *algorithm_stats_MPI_type, MPI_ANY_SOURCE, TAG_STATS,
			MPI_COMM_WORLD,	&status);
	*rank = status.MPI_SOURCE;

	parallel_evolution_log(LOG_PRIORITY_DEBUG, MODULE_MPI_UTIL, "algorithm_stats received.");
	return SUCCESS;
}

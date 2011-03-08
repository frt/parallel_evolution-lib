#include "processes.h"

#include <stdlib.h>

status_t processes_create(processes_t **processes, int size)
{
	process_algorithm_map_t *new_processes_array;
	processes_t *new_processes;
	int i;

	*processes = NULL;

	new_processes_array = (process_algorithm_map_t *)malloc(size * sizeof(process_algorithm_map_t));
	if (new_processes_array == NULL)
		return FAIL;

	new_processes = (processes_t *)malloc(sizeof(processes_t));
	if (new_processes == NULL) {
		free(new_processes_array);
		return FAIL;
	}

	for (i = 0; i < size; ++i) {
		new_processes_array[i].process_rank = 0;
		new_processes_array[i].algorithm = NULL;
	}

	new_processes->processes_array = new_processes_array;
	new_processes->size = size;

	*processes = new_processes;

	return SUCCESS;
}

void processes_destroy(processes_t **processes)
{
	free((*processes)->processes_array);
	free(*processes);
	*processes = NULL;
}

void processes_add_algorithm(processes_t *processes, algorithm_t *algorithm, int first_rank, int last_rank)
{
	int i;

	for (i = first_rank; i <= last_rank; ++i) {
		processes->processes_array[i - 1].process_rank = i;
		processes->processes_array[i - 1].algorithm = algorithm;
	}
}

status_t processes_get_algorithm(processes_t *processes, algorithm_t **algorithm, int rank)
{
	*algorithm = NULL;

	if (processes->processes_array[rank - 1].process_rank != rank)
		return FAIL;
	if (processes->processes_array[rank - 1].algorithm == NULL)
		return FAIL;

	*algorithm = processes->processes_array[rank - 1].algorithm;

	return SUCCESS;
}

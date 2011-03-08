#pragma once

#include "algorithm.h"
#include "common.h"

typedef struct process_algorithm_map {
	int process_rank;
	algorithm_t *algorithm;
} process_algorithm_map_t;

typedef struct processes {
	process_algorithm_map_t *processes_array;
	int size;
} processes_t;

status_t processes_create(processes_t **processes, int size);
void processes_destroy(processes_t **processes);
void processes_add_algorithm(processes_t *processes, algorithm_t *algorithm, int first_rank, int last_rank);
status_t processes_get_algorithm(processes_t *processes, algorithm_t **algorithm, int rank);

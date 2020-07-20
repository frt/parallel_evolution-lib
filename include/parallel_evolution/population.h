#pragma once

#include "common.h"
#include "migrant.h"
#include "algorithm.h"

typedef struct algorithm_stats algorithm_stats_t;

typedef struct population {
	migrant_t **individuals;
	int size;
	algorithm_stats_t *stats;
} population_t;

status_t population_create(population_t **population, int size);
void population_destroy(population_t **population);
status_t population_set_individual(population_t *population, migrant_t *migrant, int idx);

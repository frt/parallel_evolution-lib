#pragma once

#include "common.h"
#include "migrant.h"

typedef struct population {
	migrant_t **individuals;
	int size;
} population_t;

status_t population_create(population_t **population, int size);
void population_destroy(population_t **population);
void population_set_individual(population_t *population, migrant_t *migrant, int idx);

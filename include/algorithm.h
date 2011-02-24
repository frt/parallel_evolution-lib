#pragma once

#include "common.h"
#include "migrant.h"
#include "population.h"	/* TODO */

typedef struct altorithm {
	void (*init)();
	void (*run_interactions)(int iterations);
	void (*insert_migrant)(migrant_t *migrant);
	void (*pick_migrant)(migrant_t *my_migrant);
	int (*ended)();
	void (*get_population)(population_t *population);
} algorithm_t;

status_t algorithm_create(
		algorithm_t *algorithm,
		void (*init)(),
		void (*run_iterations)(int),
		void (*insert_migrant)(migrant_t *),
		void (*pick_migrant)(migrant_t *),
		int (*ended)(),
		void (*get_population)(population_t *)
		);
#pragma once

#include "common.h"
#include "migrant.h"
#include "population.h"

typedef struct algorithm_stats {
	int iterations;
	double avg_fitness;
	double best_fitness;
} algorithm_stats_t;

typedef struct algorithm {
	void (*init)();
	void (*run_iterations)(int iterations);
	void (*insert_migrant)(migrant_t *migrant);
	void (*pick_migrant)(migrant_t *my_migrant);
	int (*ended)();
	status_t (*get_population)(population_t **population);
	algorithm_stats_t *(*get_stats)();
} algorithm_t;

/**
 * Creates a data structure representing an optimization algorithm.
 *
 * \param[out] algorithm The pointer that will receive the algorithm data structure.
 * \param init Function that do the initialization of the algorithm.
 * \param run_iterations Function that runs a number of iterations of the algorithm.
 * \param insert_migrant Function that inserts a candidate solution into the algorithm.
 * \param pick_migrant Function that returns a candidate solution from the algorithm.
 * \param ended Function that returns a value diferent than 0 if the algorithm has ended, generally based on convergence or precision.
 * \param get_population Function that returns all the candidate soluitons of the algorithm at the moment.
 * \param get_stats Function that returns the algorithm statistics.
 *
 * \return Status: FAIL if fails to create the algorithm data structure, SUCCESS otherwise.
 */
status_t algorithm_create(
		algorithm_t **algorithm,
		void (*init)(),
		void (*run_iterations)(int),
		void (*insert_migrant)(migrant_t *),
		void (*pick_migrant)(migrant_t *),
		int (*ended)(),
		status_t (*get_population)(population_t **),
		algorithm_stats_t *(*get_stats)()
		);

void algorithm_destroy(algorithm_t **algorithm);

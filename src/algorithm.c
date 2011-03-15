#include "algorithm.h"

#include <stdlib.h>
#include "log.h"

status_t algorithm_create(
		algorithm_t **algorithm,
		void (*init)(),
		void (*run_iterations)(int),
		void (*insert_migrant)(migrant_t *),
		void (*pick_migrant)(migrant_t *),
		int (*ended)(),
		status_t (*get_population)(population_t **)
		)
{
	algorithm_t *new_algorithm;

	*algorithm = NULL;

	new_algorithm = (algorithm_t *)malloc(sizeof(algorithm_t));
	if (new_algorithm == NULL) {
		log(SEVERITY_ERROR, MODULE_ALGORITHM, "I can't allocate memory for the algorithm.");
		return FAIL;
	}
	new_algorithm->init = init;
	new_algorithm->run_iterations = run_iterations;
	new_algorithm->insert_migrant = insert_migrant;
	new_algorithm->pick_migrant = pick_migrant;
	new_algorithm->ended = ended;
	new_algorithm->get_population = get_population;

	*algorithm = new_algorithm;

	log(SEVERITY_DEBUG, MODULE_ALGORITHM, "Algorithm created.");
	return SUCCESS;
}

void algorithm_destroy(algorithm_t **algorithm)
{
	free(*algorithm);
	*algorithm = NULL;
	log(SEVERITY_DEBUG, MODULE_ALGORITHM, "Algorithm destroyed.");
}

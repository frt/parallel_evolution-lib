#include "parallel_evolution/population.h"

#include <stdlib.h>

status_t population_create(population_t **population, int size)
{
	population_t *new_population;
	
	new_population = (population_t *)malloc(sizeof(population_t));
	if (new_population == NULL)
		return FAIL;

	new_population->individuals = (migrant_t **)malloc(size * sizeof(migrant_t *));
	if (new_population->individuals == NULL) {
		free(new_population);
		return FAIL;
	}
	new_population->size = size;

	*population = new_population;

	return SUCCESS;
}
void population_destroy(population_t **population)
{
	free((*population)->individuals);
	free(*population);
	*population = NULL;
}

status_t population_set_individual(population_t *population, migrant_t *migrant, int idx)
{
	if (idx < 0 || idx >= population->size)
		return FAIL;

	population->individuals[idx] = migrant;

	return SUCCESS;
}

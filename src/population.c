#include "parallel_evolution/population.h"

#include <stdlib.h>

status_t population_create(population_t **population, int size)
{
	population_t *new_population;
    migrant_t *new_migrant;
    int i;
	
	new_population = (population_t *)malloc(sizeof(population_t));
	if (new_population == NULL)
		return FAIL;

	new_population->individuals = (migrant_t **)malloc(size * sizeof(migrant_t *));
	if (new_population->individuals == NULL)
		goto cleanup_population;

	new_population->size = size;

    for (i = 0; i < new_population->size; ++i) {
        new_migrant = (migrant_t *)malloc(sizeof(migrant_t));
        if (new_migrant == NULL)
            goto cleanup_individuals;

        new_population->individuals[i] = new_migrant;
    }

	*population = new_population;

	return SUCCESS;

cleanup_individuals:
    for (--i; i >= 0; --i)
        free(new_population->individuals[i]);

cleanup_population:
    free(new_population);

    return FAIL;
}

void population_destroy(population_t **population)
{
    int i;

    for (i =  0; i < (*population)->size; ++i)
        free((*population)->individuals[i]);
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

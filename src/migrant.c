#include "parallel_evolution/migrant.h"
#include <stdlib.h>
#include "parallel_evolution/log.h"

status_t migrant_create(migrant_t **migrant, int var_size)
{
	migrant_t *new_migrant;

	*migrant = NULL;
	
	new_migrant = (migrant_t*)malloc(sizeof(migrant_t));
	if (new_migrant == NULL) {
		parallel_evolution_log(SEVERITY_ERROR, MODULE_MIGRANT, "I can't allocate memory for the migrant.");
		return FAIL;
	}

	new_migrant->var = (double*)malloc(var_size * sizeof(double));
	if (new_migrant->var == NULL) {
		free(new_migrant);
		parallel_evolution_log(SEVERITY_ERROR, MODULE_MIGRANT, "I can't allocate memory for the migrant's variables.");
		return FAIL;
	}
	new_migrant->var_size = var_size;

	*migrant = new_migrant;

	parallel_evolution_log(SEVERITY_DEBUG, MODULE_MIGRANT, "Migrant created.");
	return SUCCESS;
}

void migrant_destroy(migrant_t **migrant)
{
	free(*migrant);
	*migrant = NULL;
	parallel_evolution_log(SEVERITY_DEBUG, MODULE_MIGRANT, "Migrant destroyed.");
}

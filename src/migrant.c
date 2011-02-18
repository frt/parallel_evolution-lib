#include "migrant.h"
#include <stdlib.h>

status_t migrant_create(migrant_t **migrant, int var_size, double *var)
{
	migrant_t *new_migrant;

	*migrant = NULL;
	
	new_migrant = (migrant_t*)malloc(sizeof(migrant_t));
	if (new_migrant == NULL)
		return FAIL;

	new_migrant->var = (double*)malloc(var_size * sizeof(double));
	if (new_migrant->var == NULL) {
		free(new_migrant);
		return FAIL;
	}
	new_migrant->var_size = var_size;

	*migrant = new_migrant;

	return SUCCESS;
}

void migrant_destroy(migrant_t **migrant)
{
	free(*migrant);
	*migrant = NULL;
}

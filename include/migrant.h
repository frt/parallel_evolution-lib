#pragma once

#include "common.h"

typedef struct migrant {
	double *var;	/* the array of variables */
	int var_size;	/* number of elements in var */
} migrant_t;

status_t migrant_create(migrant_t **migrant, int var_size, double *var);
void migrant_destroy(migrant_t **migrant);

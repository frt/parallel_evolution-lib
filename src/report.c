#include "parallel_evolution/report.h"

#include <stdio.h>
#include "parallel_evolution.h"

/* FIXME very poor reporting, please sugestions are welcome */
/* FIXME I think it should go into app code, not here */
void report_results(population_t **populations, int size)
{
	int i, j, k;

	printf("best population for each algorithm:\n");
	for (i = 0; i < size; ++i) {
		printf("\tpopulation from process with rank %d:\n", i + 1);
		for (j = 0; j < populations[i]->size; ++j) {
			printf("\t\t(");
			for (k = 0; k < parallel_evolution.number_of_dimensions; ++k) {
				if (k == 0) {
					printf("%g", populations[i]->individuals[j]->var[k]);
				} else {
					printf(", %g", populations[i]->individuals[j]->var[k]);
				}
			}
			printf(")\n");
		}
	}
}

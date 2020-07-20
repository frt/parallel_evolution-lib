#include "parallel_evolution/report.h"

#include <stdio.h>
#include "parallel_evolution.h"

void report_results(population_t **populations, int size)
{
	int i, j, k;

	printf("best population for each algorithm:\n");
	for (i = 0; i < size; ++i) {
		printf("\tpopulation from process with rank %d:\titerations: %d; avg_fitness: %g; best_fitness: %g;\n", 
                i,
                populations[i]->stats->iterations, 
                populations[i]->stats->avg_fitness, 
                populations[i]->stats->best_fitness);
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

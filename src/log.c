#include "log.h"
#include <stdio.h>
#include <mpi.h>

void log(severity_t severity, module_t module, const char *msg)
{
	const char *severity_names[] = {
		"undefined", 
		"debug", 
		"warning", 
		"error"
	};
	const char *module_names[] = {
		"undefined", 
		"algorithm", 
		"common", 
		"migrant", 
		"mpi_util", 
		"parallel_evolution", 
		"population", 
		"processes", 
		"report", 
		"topology", 
		"topology_parser"
	};
	int process_rank;
	int is_mpi_initialized;

	MPI_Initialized(&is_mpi_initialized);
	if (is_mpi_initialized) {
		MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);
		fprintf(stderr, "%s: %s: process %d: %s\n", severity_names[severity], module_names[module], process_rank, msg);
	} else
		fprintf(stderr, "%s: %s: %s\n", severity_names[severity], module_names[module], msg);
}

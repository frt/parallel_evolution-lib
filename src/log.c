#include "config.h"
#include "parallel_evolution/log.h"
#include <stdio.h>
#if HAVE_MPI_H 
	#include <mpi.h>
#elif HAVE_MPI_MPI_H
	#include <mpi/mpi.h>
#endif


void parallel_evolution_log(severity_t severity, module_t module, const char *msg)
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
	};
	int process_rank;
	int is_mpi_initialized;
	int is_mpi_finalized;

	MPI_Initialized(&is_mpi_initialized);
	MPI_Finalized(&is_mpi_finalized);
	if (is_mpi_initialized && !is_mpi_finalized) {
		MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);
		fprintf(stderr, "%s: %s: process %d: %s\n", severity_names[severity], module_names[module], process_rank, msg);
	} else
		fprintf(stderr, "%s: %s: %s\n", severity_names[severity], module_names[module], msg);
}

#include "config.h"
#include "parallel_evolution/log.h"
#include <stdio.h>
#if HAVE_MPI_H 
	#include <mpi.h>
#elif HAVE_MPI_MPI_H
	#include <mpi/mpi.h>
#endif

// TODO: configurable log level
// "emerg" (0), "alert" (1), "crit" (2), "err" (3), "warning" (4), "notice" (5), "info" (6), "debug" (7)
void parallel_evolution_log(log_priority_t priority, const char *module_name, const char *msg)
{
	const char *priority_names[] = {
        "emerg",
        "alert",
        "crit",
		"err",
		"warning", 
        "notice",
        "info",
		"debug" 
	};

	int process_rank;
	int is_mpi_initialized;
	int is_mpi_finalized;

	MPI_Initialized(&is_mpi_initialized);
	MPI_Finalized(&is_mpi_finalized);
	if (is_mpi_initialized && !is_mpi_finalized) {
		MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);
		fprintf(stderr, "%s: %s: process %d: %s\n", priority_names[priority], module_name, process_rank, msg);
	} else
		fprintf(stderr, "%s: %s: %s\n", priority_names[priority], module_name, msg);
}

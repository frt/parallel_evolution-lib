#include "log.h"
#include <stdio.h>

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
	fprintf(stderr, "%s: %s: %s\n", severity_names[severity], module_names[module], msg);
}

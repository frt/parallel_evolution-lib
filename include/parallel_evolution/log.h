#pragma once

#define SEVERITY_UNDEFINED 0
#define SEVERITY_DEBUG 1
#define SEVERITY_WARNING 2
#define SEVERITY_ERROR 3
typedef int severity_t;

#define MODULE_UNDEFINED 0
#define MODULE_ALGORITHM 1
#define MODULE_COMMON 2
#define MODULE_MIGRANT 3
#define MODULE_MPI_UTIL 4
#define MODULE_PARALLEL_EVOLUTION 5
#define MODULE_POPULATION 6
#define MODULE_PROCESSES 7
#define MODULE_REPORT 8
#define MODULE_TOPOLOGY 9
#define MODULE_TOPOLOGY_PARSER 10
typedef int module_t;

void parallel_evolution_log(severity_t severity, module_t module, const char *msg);

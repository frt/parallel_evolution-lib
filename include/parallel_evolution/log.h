#pragma once

#define SEVERITY_UNDEFINED 0
#define SEVERITY_DEBUG 1
#define SEVERITY_WARNING 2
#define SEVERITY_ERROR 3
typedef int severity_t;

void parallel_evolution_log(severity_t severity, const char *module_name, const char *msg);

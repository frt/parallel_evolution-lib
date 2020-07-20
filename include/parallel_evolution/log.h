#pragma once

#define LOG_PRIORITY_EMERG      0
#define LOG_PRIORITY_ALERT      1
#define LOG_PRIORITY_CRIT       2
#define LOG_PRIORITY_ERR        3
#define LOG_PRIORITY_WARNING    4
#define LOG_PRIORITY_NOTICE     5
#define LOG_PRIORITY_INFO       6
#define LOG_PRIORITY_DEBUG      7
typedef int log_priority_t;

// TODO: configurable log level
// "emerg" (0), "alert" (1), "crit" (2), "err" (3), "warning" (4), "notice" (5), "info" (6), "debug" (7)
void parallel_evolution_log(log_priority_t priority, const char *module_name, const char *msg);

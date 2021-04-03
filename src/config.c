#include "parallel_evolution/config.h"

#include <stdlib.h>
#include "parallel_evolution/log.h"

#define MODULE_CONFIG "config"

int _config_lookup_int(const config_t *config, const char *path, void *value)
{
    return config_lookup_int(config, path, (int *)value);
}

int _config_lookup_int64(const config_t *config, const char *path, void *value)
{
    return config_lookup_int64(config, path, (long long *)value);
}

int _config_lookup_float(const config_t *config, const char *path, void *value)
{
    return config_lookup_float(config, path, (double *)value);
}

int _config_lookup_bool(const config_t *config, const char *path, void *value)
{
    return config_lookup_bool(config, path, (int *)value);
}

int _config_lookup_string(const config_t *config, const char *path, void *value)
{
    return config_lookup_string(config, path, (const char **)value);
}

void log_if_error(int (*config_lookup_fn)(const config_t *, const char *, void *), const config_t *config, const char *path, void *value)
{
    char *logmsg = NULL;
    int n = 0;
    size_t size = 0;

    if (CONFIG_FALSE == config_lookup_fn(config, path, value)) {
        // discover the size of memory needed.
        n = snprintf(logmsg, size, "config error in file '%s' on line %d: %s",
                config_error_file(config),
                config_error_line(config),
                config_error_text(config));
        if (n >= 0) {
            size = (size_t) n + 1;
            logmsg = malloc(size);
            if (logmsg != NULL) {
                snprintf(logmsg, size, "config error in file '%s' on line %d: %s",
                        config_error_file(config),
                        config_error_line(config),
                        config_error_text(config));
                parallel_evolution_log(LOG_PRIORITY_ERR, MODULE_CONFIG, logmsg);
                exit(config_error_type(config));
            }
        }

        parallel_evolution_log(LOG_PRIORITY_ERR, MODULE_CONFIG, "config error");
        exit(config_error_type(config));
    }
}

void parallel_evolution_config_lookup_int(const config_t *config, const char *path, void *value)
{
    log_if_error(&_config_lookup_int, config, path, value);
}

void parallel_evolution_config_lookup_int64(const config_t *config, const char *path, void *value)
{
    log_if_error(&_config_lookup_int64, config, path, value);
}

void parallel_evolution_config_lookup_float(const config_t *config, const char *path, void *value)
{
    log_if_error(&_config_lookup_float, config, path, value);
}

void parallel_evolution_config_lookup_bool(const config_t *config, const char *path, void *value)
{
    log_if_error(&_config_lookup_bool, config, path, value);
}

void parallel_evolution_config_lookup_string(const config_t *config, const char *path, void *value)
{
    log_if_error(&_config_lookup_string, config, path, value);
}

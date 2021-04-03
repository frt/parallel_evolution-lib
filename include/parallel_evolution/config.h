#pragma once
#include <libconfig.h>

void parallel_evolution_config_lookup_int(const config_t *config, const char *path, void *value);

void parallel_evolution_config_lookup_int64(const config_t *config, const char *path, void *value);

void parallel_evolution_config_lookup_float(const config_t *config, const char *path, void *value);

void parallel_evolution_config_lookup_bool(const config_t *config, const char *path, void *value);

void parallel_evolution_config_lookup_string(const config_t *config, const char *path, void *value);

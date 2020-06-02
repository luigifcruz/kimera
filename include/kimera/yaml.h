#ifndef YAMLCONFIG_H
#define YAMLCONFIG_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <yaml.h>

#include "kimera/state.h"

bool kimera_parse_config_file(State* state, char* path);

#endif
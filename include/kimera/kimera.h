#ifndef KIMERA_H
#define KIMERA_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <yaml.h>

#include "kimera/config.h"

State* kimera_init();
void kimera_free(State*);
bool kimera_load_state(State*, char*);

#endif
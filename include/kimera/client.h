#ifndef KIMERA_H
#define KIMERA_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <yaml.h>

#include "kimera/state.h"

volatile sig_atomic_t stop;

//
// Client Methods
//

bool kimera_parse_config_file(State* state, char* path);
void kimera_print_version();
void kimera_print_state(State*);
bool kimera_load_state(State*, char*);
int kimera_client(int argc, char *argv[],
                  void(*tx)(State*, volatile sig_atomic_t*),
                  void(*rx)(State*, volatile sig_atomic_t*));

#endif
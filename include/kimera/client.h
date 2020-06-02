#ifndef KIMERA_H
#define KIMERA_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "kimera/state.h"
#include "kimera/yaml.h"

volatile sig_atomic_t stop;

void kimera_print_version();
void kimera_print_state(State*);
bool kimera_load_state(State*, char*);
int kimera_client(int argc, char *argv[],
                  void(*tx)(State*, volatile sig_atomic_t*),
                  void(*rx)(State*, volatile sig_atomic_t*));

#endif
#ifndef KIMERA_H
#define KIMERA_H

extern "C" {
#include <bits/types/sig_atomic_t.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <yaml.h>
#include "kimera/transport.h"
#include "kimera/state.h"
}

class Kimera {
public:
    Kimera(State*);
    ~Kimera();

    State* GetState();

    bool ParseConfigFile(char*);

    void PrintVersion();
    void PrintState();

    int Attach(int, char*, void(*tx)(State*, volatile sig_atomic_t*),
                           void(*rx)(State*, volatile sig_atomic_t*));
private:
    State* state = NULL;
    volatile sig_atomic_t* stop;
};

#endif

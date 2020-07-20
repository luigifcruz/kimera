#ifndef KIMERA_H
#define KIMERA_H

extern "C" {
#include <bits/types/sig_atomic_t.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <yaml.h>

#include <libavutil/avutil.h>
}

#include "kimera/transport.hpp"
#include "kimera/kimera.hpp"

class Client {
public:
    Client(Kimera*);

    Kimera* GetState();

    bool ParseConfigFile(char*);

    void PrintInterface(Interfaces);
    void PrintVersion();
    void PrintState();
    void PrintHelp();
    void PrintKey();

    int Attach(int, char* argv[], void(*tx)(Client*), void(*rx)(Client*));

    bool ShouldStop();

private:
    Kimera* state = NULL;
    volatile sig_atomic_t* stop;
};

#endif

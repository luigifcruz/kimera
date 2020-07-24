#ifndef KIMERA_H
#define KIMERA_H

extern "C" {
#include <bits/types/sig_atomic_t.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


#include <libavutil/avutil.h>
}

#include "kimera/transport.hpp"
#include "kimera/state.hpp"

#include <string>
#include <memory>
#include <iostream>
#include <yaml-cpp/yaml.h>

class Client {
public:
    Client(State*);

    State* GetState();

    bool ParseConfigFile(char*);

    void PrintInterface(Interfaces);
    void PrintVersion();
    void PrintState();
    void PrintHelp();
    void PrintKey();

    int Attach(int, char* argv[], void(*tx)(Client*), void(*rx)(Client*));

    bool ShouldStop();

private:
    State* state = NULL;
    volatile sig_atomic_t* stop;

    void ParseHeader(const YAML::Node&);
    void ParseBody(const YAML::Node&);
    void ParseInterfaces(const YAML::Node&, Interfaces&);
};

#endif

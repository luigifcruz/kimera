#ifndef KIMERA_H
#define KIMERA_H

#include <string>
#include <memory>
#include <cstdbool>
#include <cstdlib>
#include <yaml-cpp/yaml.h>

extern "C" {
#include <bits/types/sig_atomic_t.h>
#include <libavutil/avutil.h>
}

#include "kimera/transport.hpp"
#include "kimera/state.hpp"

class Client {
public:
    Client(State&);
    
    bool ParseConfigFile(char*);

    void PrintInterface(Interfaces);
    void PrintVersion();
    void PrintState();
    void PrintHelp();
    void PrintKey();

    int Attach(int, char* argv[], void(*)(State&, Client&), void(*)(State&, Client&));

    bool ShouldStop();

private:
    State& state;

    volatile sig_atomic_t* stop;

    void ParseHeader(const YAML::Node&);
    void ParseBody(const YAML::Node&);
    void ParseInterfaces(const YAML::Node&, Interfaces&);
};

#endif

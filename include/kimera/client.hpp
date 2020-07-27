#ifndef KIMERA_H
#define KIMERA_H

#include <string>
#include <memory>
#include <cstdbool>
#include <cstdlib>

extern "C" {
#include <bits/types/sig_atomic_t.h>
#include <libavutil/avutil.h>
}

#include "kimera/transport.hpp"
#include "kimera/state.hpp"

namespace Kimera {

class Client {
public:
    Client(State&);

    bool ShouldStop();
    int Attach(int, char* argv[], void(*)(State&, Client&), void(*)(State&, Client&));

    static void PrintState(State&);
    static void PrintVersion();
    static void PrintHelp();
    static void PrintKey();

private:
    State& state;

    volatile sig_atomic_t* stop;

    static void PrintInterface(Interfaces);
};

} // namespace Kimera

#endif

#ifndef LOOPBACK_MAC_H
#define LOOPBACK_MAC_H

#include "kimera/state.hpp"

namespace Kimera {

class Loopback {
public:
    Loopback(State&);
    ~Loopback();

    bool LoadSink();
    bool LoadSource();

    bool Push(AVFrame*);
    AVFrame* Pull();

private:
    State& state;

    AVFrame* frame = NULL;
    void* processor = NULL;
};

} // namespace Kimera

#endif
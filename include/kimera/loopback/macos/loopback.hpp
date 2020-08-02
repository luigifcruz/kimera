#ifndef LOOPBACK_MAC_H
#define LOOPBACK_MAC_H

#include "kimera/state.hpp"

extern "C" {
    #include <libavcodec/avcodec.h>
    #include "kimera/loopback/macos/interface.h"
}

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
    void* proc = NULL;
};

} // namespace Kimera

#endif

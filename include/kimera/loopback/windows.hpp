#ifndef LOOPBACK_WINDOWS_H
#define LOOPBACK_WINDOWS_H

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
};

} // namespace Kimera

#endif
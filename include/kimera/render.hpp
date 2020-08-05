#ifndef RENDER_H
#define RENDER_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>

extern "C" {
#include <libavcodec/avcodec.h>
}

#include "kimera/state.hpp"
#include "kimera/magic_enum.hpp"
#include "kimera/render/backend.hpp"

#ifdef OPENGL_BACKEND_AVAILABLE
#include "kimera/render/opengl/backend.hpp"
#endif

#ifdef VULKAN_BACKEND_AVAILABLE
#include "kimera/render/vukan/backend.hpp"
#endif

namespace Kimera {

class Render {
public:
    Render(State&);
    ~Render();

    bool Push(AVFrame*);
    bool Filter();
    bool Draw();
    AVFrame* Pull();

    void PrintMeta();

private:
    State& state;
    std::shared_ptr<Backend> backend;

    std::vector<Backends> AvailableBackends = {
        #ifdef OPENGL_BACKEND_AVAILABLE
        Backends::OPENGL,
        #endif
        #ifdef VULKAN_BACKEND_AVAILABLE
        Backends::VULKAN,
        #endif
    };

    bool CheckBackend();
};

} // namespace Kimera

#endif

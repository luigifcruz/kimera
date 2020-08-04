#ifndef RENDER_H
#define RENDER_H

extern "C" {
#include <libavcodec/avcodec.h>
}

#ifdef OPENGL_BACKEND_AVAILABLE
#include "kimera/render/opengl/backend.hpp"
#endif

#ifdef VULKAN_BACKEND_AVAILABLE
#include "kimera/render/vukan/backend.hpp"
#endif

#include "kimera/state.hpp"

#include <iostream>
#include <vector>

namespace Kimera {

enum class Backend {
    Vulkan,
    OpenGL,
};

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

    std::vector<Backend> AvailableBackends = {
        #ifdef OPENGL_BACKEND_AVAILABLE
        Backend::OpenGL,
        #endif
        #ifdef VULKAN_BACKEND_AVAILABLE
        Backend::Vulkan,
        #endif
    };
};

} // namespace Kimera

#endif

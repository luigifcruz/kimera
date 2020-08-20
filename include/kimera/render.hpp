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
#include "kimera/codec.hpp"
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

    bool LoadInput(PixelFormat);
    bool LoadDisplay();
    bool LoadFilter();
    bool LoadOutput(PixelFormat);

    bool Push(AVFrame*);
    AVFrame* Pull();

    void PrintMeta();

private:
    State& state;

    std::shared_ptr<Backend> backend;
    std::shared_ptr<Resampler> in_resampler;
    std::shared_ptr<Resampler> out_resampler;

    PixelFormat in_fmt;
    PixelFormat out_fmt;

    std::vector<Backends> AvailableBackends = {
        #ifdef OPENGL_BACKEND_AVAILABLE
        Backends::OPENGL,
        #endif
        #ifdef VULKAN_BACKEND_AVAILABLE
        Backends::VULKAN,
        #endif
    };

    bool CheckBackend();
    PixelFormat SelectFormat(std::vector<PixelFormat>, PixelFormat);
};

} // namespace Kimera

#endif

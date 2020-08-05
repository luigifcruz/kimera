#ifndef BACKEND_H
#define BACKEND_H

extern "C" {
#include "libavcodec/avcodec.h"
}

#include "kimera/state.hpp"
#include "kimera/magic_enum.hpp"

namespace Kimera {

class Backend {
public:
    virtual void PrintMeta() = 0;

    virtual std::vector<PixelFormat> GetInputFormats() = 0;
    virtual std::vector<PixelFormat> GetOutputFormats() = 0;

    virtual bool LoadInput(PixelFormat) = 0;
    virtual bool LoadDisplay() = 0;
    virtual bool LoadFilter() = 0;
    virtual bool LoadOutput(PixelFormat) = 0;

    virtual bool Push(AVFrame*) = 0;
    virtual bool Draw() = 0;
    virtual bool Filter() = 0;
    virtual AVFrame* Pull() = 0;
};

} // namespace Kimera

#endif

#include "kimera/render.hpp"

namespace Kimera {

bool Render::CheckBackend() {
    auto a = AvailableBackends;
    if (std::find(a.begin(), a.end(), state.backend) == a.end()) {
        for (const auto& b : AvailableBackends) {
            std::cout << "[RENDER] Selected "
                      << magic_enum::enum_name(state.backend)
                      << " backend not available, switching to "
                      << magic_enum::enum_name(b)
                      << "." << std::endl;
            state.backend = b;
            return true;
        }

        std::cerr << "[RENDER] No backend available." << std::endl;
        return false;
    }
    return true;
}

PixelFormat Render::SelectFormat(std::vector<PixelFormat> fmts, PixelFormat target) {
    auto av_fmt = std::find(fmts.begin(), fmts.end(), target);

    if (av_fmt == fmts.end()) {
        std::cout << "[RENDER] Backend doesn't support "
                  << magic_enum::enum_name(target)
                  << ", using software to convert it to "
                  << magic_enum::enum_name(fmts.at(0))
                  << "." << std::endl;
        return fmts.at(0);
    }

    return *av_fmt;
}

Render::Render(State& state) : state(state) {
    if (!CheckBackend()) throw "error";

    switch (state.backend) {
        case Backends::OPENGL:
            backend = std::make_shared<OpenGL>();
            break;
        case Backends::VULKAN:
            break;
    }
}

Render::~Render() {
}

bool Render::LoadInput(PixelFormat fmt) {
    in_fmt = SelectFormat(backend->GetInputFormats(), fmt);
    in_resampler = std::make_shared<Resampler>(state, in_fmt);
    return backend->LoadInput(in_fmt);
}

bool Render::LoadOutput(PixelFormat fmt) {
    out_fmt = SelectFormat(backend->GetOutputFormats(), fmt);
    out_resampler = std::make_shared<Resampler>(state, out_fmt);
    return backend->LoadOutput(out_fmt);
}

bool Render::LoadDisplay() {
    return backend->LoadDisplay();
}

bool Render::LoadFilter() {
    return backend->LoadFilter();
}

bool Render::Push(AVFrame* frame) {
    if (!in_resampler->Push(frame)) return false;
    if (!backend->Push(in_resampler->Pull())) return false;
    if (!backend->Filter()) return false;
    if (!backend->Draw()) return false;
}

AVFrame* Render::Pull() {
    if (!out_resampler->Push(backend->Pull())) return NULL;
    return out_resampler->Pull();
}

void Render::PrintMeta() {
    std::cout << "[RENDER] Available Backends: ";
    for (auto i: AvailableBackends)
        std::cout << magic_enum::enum_name(i) << " ";
    std::cout << std::endl;
}

} // namespace Kimera

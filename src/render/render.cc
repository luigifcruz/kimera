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

Render::Render(State& state) : state(state), frame(nullptr), input_active(false), display_active(false), filter_active(false), output_active(false) {
    if (!CheckBackend()) throw "error";

    switch (state.backend) {
        case Backends::OPENGL:
            backend = std::make_shared<OpenGL>(state);
            break;
        case Backends::VULKAN:
            break;
    }
}

Render::~Render() {
}

bool Render::LoadInput(PixelFormat fmt) {
    input_active = true;
    in_fmt = SelectFormat(backend->GetInputFormats(), fmt);
    in_resampler = std::make_shared<Resampler>(state, in_fmt);
    return backend->LoadInput(in_fmt);
}

bool Render::LoadOutput(PixelFormat fmt) {
    output_active = true;
    out_fmt = SelectFormat(backend->GetOutputFormats(), fmt);
    out_resampler = std::make_shared<Resampler>(state, out_fmt);

    frame = av_frame_alloc();
    frame->width = state.width;
    frame->height = state.height;
    frame->format = out_fmt;

    if (av_frame_get_buffer(frame, 0) < 0){
        printf("[RENDER] Couldn't allocate output frame.\n");
        return false;
    }

    return backend->LoadOutput(out_fmt);
}

bool Render::LoadDisplay() {
    display_active = true;
    return backend->LoadDisplay();
}

bool Render::LoadFilter() {
    filter_active = true;
    return backend->LoadFilter();
}

bool Render::CommitPipeline() {
    return backend->CommitPipeline();
}

bool Render::Push(AVFrame* frame) {
    this->frame = frame;

    if (!input_active)
        return true;

    if (!in_resampler->Push(frame))
        return false;

    pts = frame->pts;
    return backend->Push(in_resampler->Pull());
}

bool Render::Filter() {
    if (!filter_active)
        return true;
    return backend->Filter();
}
bool Render::Draw() {
    if (!display_active)
        return true;
    return backend->Draw();
}

AVFrame* Render::Pull() {
    if (!output_active)
        return frame;

    if (!input_active)
        pts += 1;

    this->frame->pts = pts;

    if (!backend->Pull(frame)) return NULL;
    if (!out_resampler->Push(frame)) return NULL;
    return out_resampler->Pull();
}

void Render::PrintMeta() {
    backend->PrintMeta();
}

} // namespace Kimera

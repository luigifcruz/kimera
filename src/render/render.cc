#include "kimera/render.hpp"
#include "kimera/magic_enum.hpp"

namespace Kimera {

Render::Render(State& state) : state(state) {
/*
    render->f_size.w = state->width;
    render->f_size.h = state->height;
    render->f_size.pix = GL_RGBA;

    render->d_size.w = state->width / 2;
    render->d_size.h = state->height / 2;

    int input_formats_size = sizeof(input_formats)/sizeof(PixelFormat);
    int output_formats_size = sizeof(output_formats)/sizeof(PixelFormat);

    render->in_format = state->in_format;
    if (!is_format_supported(render->in_format, input_formats, input_formats_size))
        render->in_format = input_formats[0];
    if (!open_resampler(render->resampler, render->in_format)) return;

    render->out_format = state->out_format;
    if (!is_format_supported(render->out_format, output_formats, output_formats_size))
        render->out_format = output_formats[0];

    if (state->sink & DISPLAY)
        render->use_display = true;

    render->time = mticks();
    render->state = state;

    if (!open_device(render)) return;
    if (!load_default(render)) return;
*/
}

Render::~Render() {
}

void Render::PrintMeta() {
    std::cout << "[RENDER] Available Backends: ";
    for (auto i: AvailableBackends)
        std::cout << magic_enum::enum_name(i) << " ";
    std::cout << std::endl;
}

} // namespace Kimera

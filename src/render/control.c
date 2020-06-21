#include "kimera/render.h"

float render_time(RenderState* render) {
    return (float)((mticks() - render->time)/1000);
}

bool render_step(RenderState* render) {
    // Add window events support.

    if (render->use_process)
        if (!render_proc_frame(&render)) return false;
    if (render->use_display)
        if (!render_draw_frame(&render)) return false;
    if (render->use_output)
        if (!render_pull_frame(&render)) return false;

    return viewport_render(render);
}

bool render_proc_callback(RenderState* render, void(*cb)(RenderState*, void*), void* obj) {
    if (!render->use_process) return false;
    // Add logic
}
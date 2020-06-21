#include "kimera/render.h"

unsigned int punch_framebuffer(RenderState* render) {
    unsigned int index = render->proc_tex[render->proc_index];
    render->proc_index = (render->proc_index + 1) % MAX_PROC;
    return index;
}

unsigned int get_framebuffer(RenderState* render) {
    return render->proc_tex[render->proc_index];
}

unsigned int get_last_framebuffer(RenderState* render) {
    return render->proc_tex[!render->proc_index];
}
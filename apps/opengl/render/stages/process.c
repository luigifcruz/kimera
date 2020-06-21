#include "render.h"

bool render_proc_frame(RenderState* render) {
    glBindFramebuffer(GL_FRAMEBUFFER, render->frame_buffer);
    
    bind_framebuffer_tex(GL_COLOR_ATTACHMENT0, get_framebuffer(render));
    set_draw_buffer(GL_COLOR_ATTACHMENT0);

    {
        glUseProgram(render->proc_shader);

        set_uniform1f(render->proc_shader, "time", render_get_time(render));
        set_uniform2f(render->proc_shader, "resolution", render->f_width, render->f_height);
        set_uniform2f(render->proc_shader, "display", render->d_width, render->d_height);

        //if (cb != NULL) (void)(*cb)(render, obj);

        set_uniform1i(render->proc_shader, "renderedTexture", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, get_last_framebuffer(render));

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    bind_framebuffer_tex(GL_COLOR_ATTACHMENT0, 0);
    punch_framebuffer(render);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return !get_gl_error(__LINE__);
}
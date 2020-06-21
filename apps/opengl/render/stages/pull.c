#include "render.h"

bool render_pull_frame(RenderState* render) {
    glBindFramebuffer(GL_FRAMEBUFFER, render->frame_buffer);

    set_draw_buffer(GL_COLOR_ATTACHMENT0);
    glUseProgram(render->out_shader);

    set_uniform1i(render->out_shader, "renderedTexture", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, get_last_framebuffer(render));

    for (unsigned int i = 0; i < render->out_planes; i++) {
        set_uniform1i(render->out_shader, "PlaneId", i);
        bind_framebuffer_tex(GL_COLOR_ATTACHMENT0, render->out_tex[i]);

        int width = (int)((float)render->f_width / render->out_ratio[i]);
        int height = (int)((float)render->f_height / render->out_ratio[i]);
        
        glViewport(0, 0, width, height);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glReadPixels(0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, render->frame->data[i]);

        bind_framebuffer_tex(GL_COLOR_ATTACHMENT0, 0);
    }

    render->frame->pts += 1;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return !get_gl_error(__LINE__);
}
#include "render.h"

bool render_push_frame(RenderState* render, AVFrame* frame) {
    glBindFramebuffer(GL_FRAMEBUFFER, render->frame_buffer);
    bind_framebuffer_tex(GL_COLOR_ATTACHMENT0, get_framebuffer(render));
    set_draw_buffer(GL_COLOR_ATTACHMENT0);

    {
        glViewport(0, 0, render->f_width, render->f_height);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(render->in_shader);

        for (unsigned int i = 0; i < render->in_planes; i++) {
            int width = (int)((float)render->f_width / render->in_ratio[i]);
            int height = (int)((float)render->f_height / render->in_ratio[i]);

            char plane_name[16];
            sprintf(plane_name, "PlaneTex%d", i);
            set_uniform1i(render->in_shader, plane_name, i);

            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, render->in_tex[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, frame->data[i]);
        }

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    bind_framebuffer_tex(GL_COLOR_ATTACHMENT0, 0);
    punch_framebuffer(render);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return !get_gl_error(__LINE__);
}
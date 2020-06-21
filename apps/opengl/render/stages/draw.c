#include "render.h"

bool render_draw_frame(RenderState* render) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    {
        glViewport(0, 0, render->d_width, render->d_height);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(render->disp_shader);

        float imgAspectRatio = (float)render->f_width / (float)render->f_height;
        float viewAspectRatio = (float)render->d_width / (float)render->d_height;

        float xScale = 1.0f, yScale = 1.0f;
        if (imgAspectRatio > viewAspectRatio) {
            yScale = viewAspectRatio / imgAspectRatio;
        } else {
            xScale = imgAspectRatio / viewAspectRatio;
        }
        set_uniform2f(render->disp_shader, "ScaleFact", xScale, yScale);

        set_uniform1i(render->disp_shader, "renderedTexture", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, get_last_framebuffer(render));

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return !get_gl_error(__LINE__);
}
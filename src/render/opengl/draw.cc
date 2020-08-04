#include "kimera/render.hpp"

bool load_display(RenderState* render) {
    render->disp_shader = load_shader(1, (char*)display_vs, (char*)display_fs);
    if (!render->disp_shader) return false;

    render->display_ready = true;
    return true;
}

bool render_draw_frame(RenderState* render) {
    if (!render->display_ready)
        if (!load_display(render)) return false;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    {
        glViewport(0, 0, render->d_size.w, render->d_size.h);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(render->disp_shader);

        float imgAspectRatio = (float)render->f_size.w / (float)render->f_size.h;
        float viewAspectRatio = (float)render->d_size.w / (float)render->d_size.h;

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

    if (get_gl_error(__LINE__)) return false;
    
    return device_render(render);
}
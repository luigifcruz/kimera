#include "kimera/render.h"
#include "kimera/shaders.h"

bool load_input(RenderState* render, AVFrame* frame) {
    if (!get_planes_count(frame, &render->in_size[0], &render->in_planes))
        return false;

    glBindFramebuffer(GL_FRAMEBUFFER, render->frame_buffer);

    glGenTextures(render->in_planes, &render->in_tex[0]);
    for (unsigned int i = 0; i < render->in_planes; i++)
        create_texture(render->in_tex[i], GL_RED, render->in_size[i].w, render->in_size[i].h);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    render->in_shader = load_shader(1, (char*)in_yuv420_vs, (char*)in_yuv420_fs);
    if (!render->in_shader) return false;

    render->input_ready = true;

    return !get_gl_error(__LINE__);
}

bool render_push_frame(RenderState* render, AVFrame* frame) {
    if (!resampler_push_frame(render->resampler, render->state, frame))
        return false;

    if (!render->input_ready)
        if (!load_input(render, render->resampler->frame)) return false;

    glBindFramebuffer(GL_FRAMEBUFFER, render->frame_buffer);
    bind_framebuffer_tex(GL_COLOR_ATTACHMENT0, get_framebuffer(render));
    set_draw_buffer(GL_COLOR_ATTACHMENT0);

    {
        glViewport(0, 0, render->f_size.w, render->f_size.h);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(render->in_shader);

        for (unsigned int i = 0; i < render->in_planes; i++) {
            char plane_name[16];
            sprintf(plane_name, "PlaneTex%d", i);
            set_uniform1i(render->in_shader, plane_name, i);

            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, render->in_tex[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, render->in_size[i].w, render->in_size[i].h, 
                         0, GL_RED, GL_UNSIGNED_BYTE, render->resampler->frame->data[i]);
        }

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    bind_framebuffer_tex(GL_COLOR_ATTACHMENT0, 0);
    punch_framebuffer(render);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    render->pts = render->resampler->frame->pts;

    return !get_gl_error(__LINE__);
}
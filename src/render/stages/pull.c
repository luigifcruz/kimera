#include "kimera/render.h"
#include "kimera/shaders.h"

bool load_output(RenderState* render) {
    render->frame = av_frame_alloc();
    render->frame->width = render->f_size.w;
    render->frame->height = render->f_size.h;
    render->frame->format = render->out_format;
    render->frame->pts = render->pts;
    if (av_frame_get_buffer(render->frame, 0) < 0){
        printf("[RENDER] Couldn't allocate output frame.\n");
        return false;
    }

    if (!get_planes_count(render->frame, &render->out_size[0], &render->out_planes))
        return false;
    
    glBindFramebuffer(GL_FRAMEBUFFER, render->frame_buffer);

    glGenTextures(render->out_planes, &render->out_tex[0]);
    for (unsigned int i = 0; i < render->out_planes; i++)
        create_texture(render->out_tex[i], GL_RED, render->out_size[i].w, render->out_size[i].h);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
   
    render->out_shader = load_shader(1, (char*)out_yuv420_vs,  (char*)out_yuv420_fs);
    if (!render->out_shader) return false;

    render->output_ready = true;

    return !get_gl_error(__LINE__);
}

bool render_pull_frame(RenderState* render) {
    if (!render->output_ready)
        if (!load_output(render)) return false;

    glBindFramebuffer(GL_FRAMEBUFFER, render->frame_buffer);

    set_draw_buffer(GL_COLOR_ATTACHMENT0);
    glUseProgram(render->out_shader);

    set_uniform1i(render->out_shader, "renderedTexture", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, get_last_framebuffer(render));

    for (unsigned int i = 0; i < render->out_planes; i++) {
        set_uniform1i(render->out_shader, "PlaneId", i);
        bind_framebuffer_tex(GL_COLOR_ATTACHMENT0, render->out_tex[i]);
        
        glViewport(0, 0, render->out_size[i].w, render->out_size[i].h);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glReadPixels(0, 0, render->out_size[i].w, render->out_size[i].h,
                     GL_RED, GL_UNSIGNED_BYTE, render->frame->data[i]);

        bind_framebuffer_tex(GL_COLOR_ATTACHMENT0, 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    render->frame->pts = render->pts;

    return !get_gl_error(__LINE__);
}
#include "kimera/render.h"

RenderState* alloc_render() {
    RenderState* state = (RenderState*)malloc(sizeof(RenderState));
    state->view->display = NULL;
    state->view->surface = NULL;
    state->view->context = NULL;
    state->view->adapter = NULL;
    state->use_display   = false;
    state->use_opengles  = false;
    state->use_input     = false;
    state->use_output    = false;
    state->use_process   = false;
    state->configured    = false;
    return state;
}

bool open_render(RenderState* render, State* state) {
    // Setup Dimensions
    render->f_height = state->height;
    render->f_width = state->width;
    render->d_height = state->height / 2;
    render->d_width = state->width / 2;

    // Load Settings
    if (state->sink & DISPLAY)
        render->use_display = true;

    if (true)
        render->use_process = true;

    // Calculate Best Conversion Strategy
    bool use_opengl = false;

    if (render->use_display || render->use_process)
        use_opengl = true;

    if (!use_opengl) {
        int conversion_index = 0;
        if (!is_format_supported(state->in_format, input_formats))
            conversion_index += 1;
        if (!is_format_supported(state->out_format, output_formats))
            conversion_index += 1;

        if (conversion_index == 0)
            use_opengl = true;
    }

    // Setup Components
    if (use_opengl) {
        if (!open_viewport(render)) return false;
        if (!load_default(render)) return false;

        if (render->use_display)
            if (!load_display(render)) return false;

        if (render->use_process)
            if (!load_process(render)) return false;

        render->frame = av_frame_alloc();
        render->frame->width = state->width;
        render->frame->height = state->height;
        render->frame->format = state->out_format;
        render->frame->pts = 0;
        if (av_frame_get_buffer(render->frame, 0) < 0){
            printf("[RENDER] Couldn't allocate output frame.\n");
            return false;
        }
    }

    render->time = mticks();

    return true;
}

void close_render(RenderState* render) {
    // Add several more...
    if (render->view->surface)
        eglDestroySurface(render->view->display, render->view->surface);
    if (render->view->context)
        eglDestroyContext(render->view->display, render->view->context);
    if (render->view->display)
        eglTerminate(render->view->display);
    glfwTerminate();
    free(render);
}

bool load_default(RenderState* render) {
    // Generating Buffers
    glGenBuffers(1, &render->vertex_buffer);
    glGenBuffers(1, &render->index_buffer);
    glGenFramebuffers(1, &render->frame_buffer);

    // Loading Vertex Buffer
    glBindBuffer(GL_ARRAY_BUFFER, render->vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Loading Indices Buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render->index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Loading framebuffer for internal textures
    glBindFramebuffer(GL_FRAMEBUFFER, render->frame_buffer);

    glGenTextures(MAX_PROC, &render->proc_tex[0]);
    for (unsigned int i = 0; i < MAX_PROC; i++)
        create_texture(render->proc_tex[i], GL_RGBA, render->f_width, render->f_height);
    set_draw_buffer(GL_COLOR_ATTACHMENT0);
    render->proc_index = 0;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return !get_gl_error(__LINE__);
}

bool load_input(RenderState* render, AVFrame* frame) {
    if (!get_planes_count(frame, &render->in_ratio[0], &render->in_planes))
        return false;

    glBindFramebuffer(GL_FRAMEBUFFER, render->frame_buffer);

    glGenTextures(render->in_planes, &render->in_tex[0]);
    for (unsigned int i = 0; i < render->in_planes; i++) {
        int width = (int)((float)render->f_width / render->in_ratio[i]);
        int height = (int)((float)render->f_height / render->in_ratio[i]);
        create_texture(render->in_tex[i], GL_RED, width, height);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    render->in_shader = load_shader(1, (char*)in_yuv420_vs, (char*)in_yuv420_fs);
    if (!render->in_shader) return false;

    return !get_gl_error(__LINE__);
}

bool load_output(RenderState* render, AVFrame* frame) {
    if (!get_planes_count(frame, &render->out_ratio[0], &render->out_planes))
        return false;
    
    glBindFramebuffer(GL_FRAMEBUFFER, render->frame_buffer);

    glGenTextures(render->out_planes, &render->out_tex[0]);
    for (unsigned int i = 0; i < render->out_planes; i++) {
        int width = (int)((float)render->f_width / render->out_ratio[i]);
        int height = (int)((float)render->f_height / render->out_ratio[i]);
        create_texture(render->out_tex[i], GL_RED, width, height);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
   
    render->out_shader = load_shader(1, (char*)out_yuv420_vs,  (char*)out_yuv420_fs);
    if (!render->out_shader) return false;

    return !get_gl_error(__LINE__);
}

bool load_display(RenderState* render) {
    render->disp_shader = load_shader(1, (char*)display_vs, (char*)display_fs);
    if (!render->disp_shader) return false;

    return !get_gl_error(__LINE__);
}

bool load_process(RenderState* render) {
    render->proc_shader = load_shader(1, (char*)filter_vs, (char*)filter_fs);
    if (!render->proc_shader) return false;

    return !get_gl_error(__LINE__);
}
#include "kimera/render.h"

RenderState* init_render() {
    RenderState* state     = (RenderState*)malloc(sizeof(RenderState));
    state.device          = init_device();
    state.resampler       = init_resampler();
    state.use_opengles    = true;
    state.use_display     = false;
    state.display_ready   = false;
    state.input_ready     = false;
    state.output_ready    = false;
    state.process_ready   = false;
    return state;
}

void close_render(RenderState* render) {
    // Add several more...
    close_device(render->device);
    free(render);
}

bool open_render(RenderState* render, State* state) {
    render->f_size.w = state.width;
    render->f_size.h = state.height;
    render->f_size.pix = GL_RGBA;
    
    render->d_size.w = state.width / 2;
    render->d_size.h = state.height / 2;

    int input_formats_size = sizeof(input_formats)/sizeof(enum AVPixelFormat);
    int output_formats_size = sizeof(output_formats)/sizeof(enum AVPixelFormat);

    render->in_format = state.in_format; 
    if (!is_format_supported(render->in_format, input_formats, input_formats_size))
        render->in_format = input_formats[0];
    if (!open_resampler(render->resampler, render->in_format)) return false;

    render->out_format = state.out_format; 
    if (!is_format_supported(render->out_format, output_formats, output_formats_size))
        render->out_format = output_formats[0];

    if (CHECK(state.sink, DISPLAY))
        render->use_display = true;

    render->time = mticks();
    render->state = state;

    if (!open_device(render)) return false;
    if (!load_default(render)) return false;

    return true;
}

float render_time(RenderState* render) {
    return (float)((mticks() - render->time)/1000);
}

bool render_proc_callback(RenderState* render, void(*cb)(RenderState*, void*), void* obj) {
    if (!render->process_ready) return false;
    // Add logic
    return true;
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
        create_texture(render->proc_tex[i], render->f_size);
    set_draw_buffer(GL_COLOR_ATTACHMENT0);
    render->proc_index = 0;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return !get_gl_error(__LINE__);
}
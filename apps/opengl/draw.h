#include "render/helper.h"
#include "kimera/state.h"

/*
//  -> INPUT (RGB, RGBA, YUV420, NV12, YUV422) [in]
//      - Color Planes (1 to 3).
//  -> PROCESSING (RGBA only) [proc]
//      - Texture A.
//      - Texture B.
//  -> DISPLAY (RGBA) [disp]
//      - Display Texture with borders.
//  -> OUPUT (YUV420, YUV422, NV12) [out]
//      - Color Planes (1 to 3).
*/

const float vertices[] = {
     1.0f,  1.0f, 0.0f,  1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,  1.0f, 1.0f,
    -1.0f, -1.0f, 0.0f,  0.0f, 1.0f,
    -1.0f,  1.0f, 0.0f,  0.0f, 0.0f 
};

const unsigned int indices[] = {
    0, 1, 3,
    1, 2, 3
};

typedef struct {
    AVFrame* frame;

    bool use_display;
    bool use_opengles;
    bool use_input;
    bool use_output;
    bool use_process;

    int f_width;
    int f_height;

    int d_width;
    int d_height;

    unsigned int vertex_buffer;
    unsigned int index_buffer;
    unsigned int frame_buffer;

    unsigned int in_shader;
    unsigned int out_shader;
    unsigned int proc_shader;
    unsigned int disp_shader;

    float in_ratio[MAX_PLANES];
    float out_ratio[MAX_PLANES];

    unsigned int in_planes;
    unsigned int out_planes;

    unsigned int proc_index;

    unsigned int in_tex[MAX_PLANES];
    unsigned int out_tex[MAX_PLANES];
    unsigned int proc_tex[MAX_PROC];
} CanvasState;

unsigned int punch_framebuffer(CanvasState* render) {
    unsigned int index = render->proc_tex[render->proc_index];
    render->proc_index = (render->proc_index + 1) % MAX_PROC;
    return index;
}

unsigned int get_framebuffer(CanvasState* render) {
    return render->proc_tex[render->proc_index];
}

unsigned int get_last_framebuffer(CanvasState* render) {
    return render->proc_tex[!render->proc_index];
}

bool load_default(CanvasState* render) {
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

bool load_input(CanvasState* render, AVFrame* frame) {
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
    
    render->in_shader = load_shader("./shaders/in_yuv420.vs", "./shaders/in_yuv420.fs");
    if (!render->in_shader) return false;
    
    render->use_input = true;

    return !get_gl_error(__LINE__);
}

bool load_output(CanvasState* render, AVFrame* frame) {
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
   
    render->out_shader = load_shader("./shaders/out_yuv420.vs", "./shaders/out_yuv420.fs");
    if (!render->out_shader) return false;
    
    render->use_output = true;

    return !get_gl_error(__LINE__);
}

bool load_display(CanvasState* render) {
    render->disp_shader = load_shader("./shaders/display.vs", "./shaders/display.fs");
    if (!render->disp_shader) return false;
    
    render->use_display = true;

    return !get_gl_error(__LINE__);
}

bool load_process(CanvasState* render) {
    render->proc_shader = load_shader("./shaders/filter.vs", "./shaders/filter.fs");
    if (!render->proc_shader) return false;

    render->use_process = true;

    return !get_gl_error(__LINE__);
}

bool render_push_frame(CanvasState* render, AVFrame* frame) {
    if (!render->use_input)
        if (!load_input(render, frame)) return false;

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

bool render_proc_frame(CanvasState* render, void(*cb)(CanvasState*, void*), void* obj) {
    if (!render->use_process)
        if (!load_process(render)) return false;

    glBindFramebuffer(GL_FRAMEBUFFER, render->frame_buffer);
    
    bind_framebuffer_tex(GL_COLOR_ATTACHMENT0, get_framebuffer(render));
    set_draw_buffer(GL_COLOR_ATTACHMENT0);

    {
        glUseProgram(render->proc_shader);

        set_uniform1f(render->proc_shader, "time", glfwGetTime());
        set_uniform2f(render->proc_shader, "resolution", render->f_width, render->f_height);
        set_uniform2f(render->proc_shader, "display", render->d_width, render->d_height);

        if (cb != NULL) (void)(*cb)(render, obj);

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

bool render_draw_frame(CanvasState* render) {
    if (!render->use_display) return false;

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
        glBindTexture(GL_TEXTURE_2D, render->out_tex[1]);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return !get_gl_error(__LINE__);
}

bool render_pull_frame(CanvasState* render) {
    if (!render->use_output)
        if (!load_output(render, render->frame)) return false;

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

bool start_render2(CanvasState* render, State* state) {
    render->f_height = state->height;
    render->f_width = state->width;

    render->frame = av_frame_alloc();
    render->frame->width = state->width;
    render->frame->height = state->height;
    render->frame->format = state->out_format;
    render->frame->pts = 0;
    if (av_frame_get_buffer(render->frame, 0) < 0){
        printf("[RENDER] Couldn't allocate output frame.\n");
        return false;
    }

    if (!load_default(render)) return false;

    if (state->source & DISPLAY || state->sink & DISPLAY)
        if (!load_display(render)) return false;
    
    return true;
}

void render_print_meta2(CanvasState* render) {
    int tex_size = render->in_planes + render->out_planes + 2;
    printf("[RENDER] Number of Textures: %d\n", tex_size);
}
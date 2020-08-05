#include "kimera/render/opengl/backend.hpp"

namespace Kimera {

OpenGL::OpenGL() {
    std::cout << "[BACKEND] Initializing OpenGL backend." << std::endl;
}

OpenGL::~OpenGL() {
    std::cout << "[BACKEND] Exiting OpenGL backend." << std::endl;
}

void OpenGL::PrintMeta() {
    std::cout << "meta" << std::endl;
}

std::vector<PixelFormat> OpenGL::GetInputFormats() {
    return InputFormats;
}

std::vector<PixelFormat> OpenGL::GetOutputFormats() {
    return OutputFormats;
}

bool OpenGL::LoadInput(PixelFormat) {}
bool OpenGL::LoadDisplay() {}
bool OpenGL::LoadFilter() {}
bool OpenGL::LoadOutput(PixelFormat) {}

bool OpenGL::Push(AVFrame*) {}
bool OpenGL::Draw() {}
bool OpenGL::Filter() {}
AVFrame* OpenGL::Pull() {}

} // namespace Kimera

/*
void render_print_meta(RenderState* render) {
    printf(".   [Render Meta]\n");
    printf("├── Window Mode:   %s\n", render_mode_query(render));
    printf("├── Frame Size:    %dx%d\n", render->f_size.w, render->f_size.h);
    printf("├── Device Size:   %dx%d\n", render->d_size.w, render->d_size.h);
    printf("└──.   [EGL Meta]\n");
    printf("│  ├── APIs:       %s\n", egl_query(render, EGL_CLIENT_APIS));
    printf("│  ├── Version:    %s\n", egl_query(render, EGL_VERSION));
    printf("│  └── Vendor:     %s\n", egl_query(render, EGL_VENDOR));
    printf("└──.   [GL Meta]\n");
    printf("   ├── Renderer:   %s\n", gl_query(GL_RENDERER));
    printf("   ├── Version:    %s\n", gl_query(GL_VERSION));
    printf("   ├── Vendor:     %s\n", gl_query(GL_VENDOR));
    printf("   └── GLSL Ver.:  %s\n", gl_query(GL_SHADING_LANGUAGE_VERSION));
}

void render_print_ext(RenderState* render) {
    printf(".   [Render Extensions]\n");
    printf("├── EGL Extensions: %s\n", egl_query(render, EGL_EXTENSIONS));
    printf("└── GL Extensions:  %s\n", gl_query(GL_EXTENSIONS));
}

float render_time(RenderState* render) {
    return (float)((mticks() - render->time)/1000);
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
*/

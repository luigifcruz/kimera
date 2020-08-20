#include "kimera/render/opengl/backend.hpp"

namespace Kimera {

bool OpenGL::GetErrorGL(int line) {
    int error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("[RENDER] GL returned an error #%d at line #%d.\n", error, line);
        return true;
    }
    return false;
}

bool OpenGL::GetErrorEGL(int line) {
    int error = eglGetError();
    if (error != EGL_SUCCESS) {
        printf("[RENDER] EGL returned an error #%d at line #%d.\n", error, line);
        return true;
    }
    return false;
}

void OpenGL::CreateTexture(unsigned int id, Format fmt) {
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, fmt.pix, fmt.w, fmt.h, 0, fmt.pix, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGL::SetDrawBuffer(unsigned int attachment) {
    unsigned int targets[1] = { GL_COLOR_ATTACHMENT0 };
    targets[0] = attachment;
    glDrawBuffers(1, targets);
}

OpenGL::OpenGL() {
    std::cout << "[BACKEND] Initializing OpenGL backend." << std::endl;

    // Generating Buffers
    glGenBuffers(1, &vertex_buffer);
    glGenBuffers(1, &index_buffer);
    glGenFramebuffers(1, &frame_buffer);

    // Loading Vertex Buffer
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Loading Indices Buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Loading framebuffer for internal textures
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

    glGenTextures(MAX_PROC, &proc_tex[0]);
    for (unsigned int i = 0; i < MAX_PROC; i++)
        CreateTexture(proc_tex[i], f_size);
    SetDrawBuffer(GL_COLOR_ATTACHMENT0);
    proc_index = 0;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (!GetErrorGL(__LINE__)) throw "error";
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

bool OpenGL::LoadInput(PixelFormat) {
    std::cout << "Loading input my dude..." << std::endl;
    return true;
}

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
}
*/

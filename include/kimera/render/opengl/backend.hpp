#ifndef OPENGL_BACKEND_H
#define OPENGL_BACKEND_H

#define MAX_PROC 2
#define MAX_PLANES 3

extern "C" {
#include "glad/glad.h"
#include "glad/glad_egl.h"

#if   defined(KIMERA_MACOS)
    #define GLFW_EXPOSE_NATIVE_COCOA
    #include <sys/time.h>
#elif defined(KIMERA_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
    #define GLFW_EXPOSE_NATIVE_WAYLAND
    #include <unistd.h>
    #include <sys/time.h>
#elif defined(KIMERA_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <Windows.h>
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <libavcodec/avcodec.h>
}

#include "kimera/render/backend.hpp"
#include "kimera/state.hpp"

#include <iostream>
#include <vector>

namespace Kimera {

static const int egl_attr[] = {
    EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_DEPTH_SIZE, 16,
    EGL_NONE
};

static const int egl_ctx_attr[] = {
    EGL_CONTEXT_MAJOR_VERSION, 2,
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE
};

static const int egl_pbuf_attr[] = {
    EGL_WIDTH, 640,
    EGL_HEIGHT, 480,
    EGL_NONE,
};

static const float vertices[] = {
     1.0f,  1.0f, 0.0f,  1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,  1.0f, 1.0f,
    -1.0f, -1.0f, 0.0f,  0.0f, 1.0f,
    -1.0f,  1.0f, 0.0f,  0.0f, 0.0f
};

static const unsigned int indices[] = {
    0, 1, 3,
    1, 2, 3
};

typedef struct {
    int w;
    int h;
    GLenum pix;
} Format;

typedef struct {
    GLFWwindow*  adapter;
    GLenum       api;
    EGLConfig    config;
    EGLDisplay   display;
    EGLSurface   surface;
    EGLContext   context;
} DeviceState;

class OpenGL : public Backend {
public:
    OpenGL();
    ~OpenGL();

    void PrintMeta();

    std::vector<PixelFormat> GetInputFormats();
    std::vector<PixelFormat> GetOutputFormats();

    bool LoadInput(PixelFormat);
    bool LoadDisplay();
    bool LoadFilter();
    bool LoadOutput(PixelFormat);

    bool Push(AVFrame*);
    bool Draw();
    bool Filter();
    AVFrame* Pull();

private:
    std::vector<PixelFormat> InputFormats = {
        AV_PIX_FMT_YUV420P,
        AV_PIX_FMT_BGRA
    };

    std::vector<PixelFormat> OutputFormats = {
        AV_PIX_FMT_YUV420P,
    };

    bool use_opengles;
    bool use_display;

    bool display_ready;
    bool input_ready;
    bool output_ready;
    bool process_ready;

    int pts;
    double time;

    PixelFormat in_format;
    PixelFormat out_format;

    Format f_size;
    Format d_size;

    Format in_size[MAX_PLANES];
    Format out_size[MAX_PLANES];

    unsigned int vertex_buffer;
    unsigned int index_buffer;
    unsigned int frame_buffer;

    unsigned int in_shader;
    unsigned int out_shader;
    unsigned int proc_shader;
    unsigned int disp_shader;

    unsigned int in_planes;
    unsigned int out_planes;

    unsigned int proc_index;

    unsigned int in_tex[MAX_PLANES];
    unsigned int out_tex[MAX_PLANES];
    unsigned int proc_tex[MAX_PROC];

    bool GetErrorGL(int);
    bool GetErrorEGL(int);

    void CreateTexture(unsigned int, Format);
    void SetDrawBuffer(unsigned int);
};

} // namespace Kimera

#endif

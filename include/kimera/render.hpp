#ifndef RENDER_H
#define RENDER_H

//
// Includes
//

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
}

#include "kimera/state.hpp"

#include <iostream>

//
// Global Constant Variables
//

static const enum AVPixelFormat input_formats[] = {
    AV_PIX_FMT_YUV420P,
    AV_PIX_FMT_BGRA
};

static const enum AVPixelFormat output_formats[] = {
    AV_PIX_FMT_YUV420P
};

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

//
// Typedef Variables
//

typedef struct {
    int w;
    int h;
    GLenum pix;
} Resolution;

typedef struct {
    GLFWwindow*  adapter;
    GLenum       api;
    EGLConfig    config;
    EGLDisplay   display;
    EGLSurface   surface;
    EGLContext   context;
} DeviceState;

typedef struct {
    State*         state;
    DeviceState*   device;
    AVFrame*       frame;

    enum AVPixelFormat in_format;
    enum AVPixelFormat out_format;

    ResamplerState* resampler;

    bool use_opengles;
    bool use_display;

    bool display_ready;
    bool input_ready;
    bool output_ready;
    bool process_ready;

    int pts;
    double time;

    Resolution f_size;
    Resolution d_size;

    Resolution in_size[MAX_PLANES];
    Resolution out_size[MAX_PLANES];

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
} RenderState;

bool load_display(RenderState* render);

bool render_draw_frame(RenderState* render);

bool load_process(RenderState* render);

bool render_proc_frame(RenderState* render);

bool load_output(RenderState* render);

bool render_pull_frame(RenderState* render);

bool load_input(RenderState* render, AVFrame* frame);

bool render_push_frame(RenderState* render, AVFrame* frame);

unsigned int punch_framebuffer(RenderState* render);

unsigned int get_framebuffer(RenderState* render);

unsigned int get_last_framebuffer(RenderState* render);

float render_time(RenderState* render);

bool render_proc_callback(RenderState* render, void(*cb)(RenderState*, void*), void* obj);

const char* egl_query(RenderState* render, int name);

const char* gl_query(GLenum name);

const char* render_mode_query(RenderState* render);

const char* render_api_query(RenderState* render);

void render_print_meta(RenderState* render);

void render_print_ext(RenderState* render);

RenderState* init_render();

bool open_render(RenderState* render, State* state);

void close_render(RenderState* render);

bool load_default(RenderState* render);

size_t get_file_size(FILE* fp);

char* open_shader(char* filepath);

bool get_gl_error(int line);

bool get_egl_error(int line);

unsigned int compile_shader(unsigned int type, char* code_str);

unsigned int load_shader(int type, char* vs_str, char* fs_str);

void bind_framebuffer_tex(unsigned int atch_id, unsigned int tex_id);

bool get_planes_size(AVFrame* frame, Resolution* size, unsigned int* planes);

void fill_texture(Resolution size, uint8_t* data);

void read_texture(Resolution size, uint8_t* data);

void create_texture(unsigned int id, Resolution size);

void set_uniform4f(int program, char* name, float v0, float v1, float v2, float v3);

void set_uniform3f(int program, char* name, float v0, float v1, float v2);

void set_uniform2f(int program, char* name, float v0, float v1);

void set_uniform1f(int program, char* name, float v0);

void set_uniform1i(int program, char* name, int v0);

void set_draw_buffer(GLenum attachment);

double mticks();

bool is_format_supported(enum AVPixelFormat format, const enum AVPixelFormat formats[], int size);

DeviceState* init_device();

void close_device(DeviceState*);

bool open_device(RenderState* render);

bool device_render(RenderState* render);

#endif

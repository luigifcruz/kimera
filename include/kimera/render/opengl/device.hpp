#ifndef RENDER_BACKEND_H
#define RENDER_BACKEND_H

#include "kimera/state.hpp"

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

void key_callback(GLFWwindow*, int, int, int, int);

class Device {
public:
    GLenum       api;
    GLFWwindow*  adapter = nullptr;
    EGLConfig    config  = nullptr;
    EGLDisplay   display = nullptr;
    EGLSurface   surface = nullptr;
    EGLContext   context = nullptr;

    Device(State& state, int, int, bool);
    ~Device();

    const char* Query(int);
    bool GetError(std::string);
    bool Step(int*, int*);

private:
    State& state;
    std::string window_name;
};

} // namespace Kimera

#endif

#ifndef RENDER_STATE_H
#define RENDER_STATE_H

#include "glad/glad.h"
#include "glad/glad_egl.h"

#ifdef KIMERA_MACOS
#define GLFW_EXPOSE_NATIVE_COCOA
#elif defined KIMERA_UNIX
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_WAYLAND
#include <unistd.h>
#elif defined KIMERA_WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <Windows.h>
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

typedef enum {
    WINDOWED = 0,
    HEADLESS = 1,
} RenderMode;

typedef struct {
    int         width;
    int         height;
    RenderMode  mode;
    GLFWwindow* adapter;
    GLenum      api;
    EGLConfig   config;
    EGLDisplay  display;
    EGLSurface  surface;
    EGLContext  context;
} RenderState;

const int egl_attr[] = {
    EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_DEPTH_SIZE, 16,
    //EGL_SAMPLES, 4,
    EGL_NONE
};

const int egl_ctx_attr[] = {
    EGL_CONTEXT_MAJOR_VERSION, 3,
    EGL_CONTEXT_CLIENT_VERSION, 3,
    EGL_NONE
};

const int egl_pbuf_attr[] = {
    EGL_WIDTH, 640,
    EGL_HEIGHT, 480,
    EGL_NONE,
};

#endif
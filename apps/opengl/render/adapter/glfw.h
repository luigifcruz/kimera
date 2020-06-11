#ifndef RENDER_WINDOW_GLFW_H
#define RENDER_WINDOW_GLFW_H

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

#include "../state.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

typedef struct {
    GLFWwindow* display;
} WindowState;

void* alloc_adapter() {
    WindowState* state = (WindowState*)malloc(sizeof(WindowState));
    state->display = NULL;
    return state;
}

void free_adapter(void* window_ptr) {
    WindowState* window = (WindowState*)window_ptr;

    if (window->display)
        glfwTerminate();
    free(window);
}

bool adapter_poll_events(void* window_ptr) {
    WindowState* window = (WindowState*)window_ptr;

    glfwPollEvents();
    return !glfwWindowShouldClose(window->display);
}

bool load_adapter(RenderState* render) {
    render->display = eglGetDisplay(EGL_NO_DISPLAY);
    return true;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    glScissor(0, 0, width, height);
}

bool create_adapter(RenderState* render) {
    WindowState* window = (WindowState*)render->adapter;
    
    if (!glfwInit()) {
        printf("[RENDER] Can't initiate GLFW.\n");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    if (!(window->display = glfwCreateWindow(
        render->width, render->height, "RENDER", NULL, NULL))) {
        printf("[RENDER] Can't create GLFW window.\n");
        glfwTerminate();
        return false;
    }

    glfwSetFramebufferSizeCallback(window->display, framebuffer_size_callback);

    EGLNativeWindowType surface;
    
    #ifdef KIMERA_MACOS
        surface = glfwGetCocoaWindow(window->display);
    #elif defined KIMERA_UNIX
        surface = glfwGetX11Window(window->display);
    #elif defined KIMERA_WIN32
        surface = glfwGetWin32Window(window->display);
    #endif

    render->surface = eglCreateWindowSurface(render->display, render->config, surface, 0);
    if (!render->surface) {
        printf("[RENDER] Failed to create window surface.\n");
        get_egl_error(__LINE__);
        return false;
    };

    return true;
}

#endif
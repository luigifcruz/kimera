#ifndef RENDER_H
#define RENDER_H

#include <stdlib.h>
#include <stdio.h>

#include "helper.h"
#include "state.h"
#include "kimera/state.h"

RenderState* alloc_render() {
    RenderState* state = (RenderState*)malloc(sizeof(RenderState));
    state->display = NULL;
    state->surface = NULL;
    state->context = NULL;
    state->adapter = NULL;
    return state;
}

void free_render(RenderState* render) {
    if (render->surface)
        eglDestroySurface(render->display, render->surface);
    if (render->context)
        eglDestroyContext(render->display, render->context);
    if (render->display)
        eglTerminate(render->display);
    glfwTerminate();
    free(render);
}

bool start_render(RenderState* render, State* state) {
    render->width = state->width;
    render->height = state->height;
    
    if (!gladLoadEGL()) {
        printf("[RENDER] Failed to initialize EGL.\n");
        return false;
    }

    render->display = eglGetDisplay(EGL_NO_DISPLAY);
    
    if (!eglInitialize(render->display, NULL, NULL)) {
        printf("[RENDER] Failed to initialize EGL.\n");
        get_egl_error(__LINE__);
        return false;
    }

    int num_config;
    if (!eglChooseConfig(render->display, egl_attr, &render->config, 1, &num_config)) {
        printf("[RENDER] Failed to config EGL.\n");
        get_egl_error(__LINE__);
        return false;
    }

    if (!eglBindAPI(render->api)) {
        printf("[RENDER] Failed to bind API (%d) to EGL.\n", render->mode);
        get_egl_error(__LINE__);
        return false;
    }

    if (render->mode == HEADLESS) {
        if (!(render->surface = eglCreatePbufferSurface(render->display, render->config, egl_pbuf_attr))) {
            printf("[RENDER] Error creating pbuffer surface.\n");
            get_egl_error(__LINE__);
            return false;
        }
    }

    if (render->mode == WINDOWED) {
        if (!glfwInit()) {
            printf("[RENDER] Can't initiate GLFW.\n");
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        if (!(render->adapter = glfwCreateWindow(
            render->width, render->height, "RENDER", NULL, NULL))) {
            printf("[RENDER] Can't create GLFW window.\n");
            glfwTerminate();
            return false;
        }

        EGLNativeWindowType surface;
    
        #ifdef KIMERA_MACOS
            surface = glfwGetCocoaWindow(render->adapter);
        #elif defined KIMERA_UNIX
            surface = glfwGetX11Window(render->adapter);
        #elif defined KIMERA_WIN32
            surface = glfwGetWin32Window(render->adapter);
        #endif

        render->surface = eglCreateWindowSurface(render->display, render->config, surface, 0);
        if (!render->surface) {
            printf("[RENDER] Failed to create window surface.\n");
            get_egl_error(__LINE__);
            return false;
        };
    }

    render->context = eglCreateContext(render->display, render->config, EGL_NO_CONTEXT, egl_ctx_attr);
    if (!render->context) {
        printf("[RENDER] Failed to create context.\n");
        get_egl_error(__LINE__);
        return false;
    };

    if (!eglMakeCurrent(render->display, render->surface, render->surface, render->context)) {
        printf("[RENDER] Failed to make current surface.\n");
        get_egl_error(__LINE__);
        return false;
    }

    int response = 0;

    if (render->api == EGL_OPENGL_API)
        response = gladLoadGLLoader((GLADloadproc)eglGetProcAddress);
    if (render->api == EGL_OPENGL_ES_API)
        response = gladLoadGLES2Loader((GLADloadproc)eglGetProcAddress);

    if (!response) {
        printf("[RENDER] Failed to initialize GL/GLES.\n");
        return false;
    }

    if (get_gl_error(__LINE__) || get_egl_error(__LINE__)) return false;

    return true;
}

void render_push_frame(RenderState* render) {
}

void render_pull_frame(RenderState* render) {
}

bool render_commit_frame(RenderState* render) {
    if (get_gl_error(__LINE__)) {
        printf("[RENDER] Failed to commit userspace code.\n");
        return false;
    }
    
    if (get_gl_error(__LINE__)) {
        printf("[RENDER] Failed to commit internal render code.\n");
        return false;
    }

    if (render->mode == WINDOWED) {
        eglSwapBuffers(render->display, render->surface);
        if (get_egl_error(__LINE__)) return false;
        
        glfwPollEvents();
        glfwGetWindowSize(render->adapter, &render->width, &render->height);
        return !glfwWindowShouldClose(render->adapter);
    }
    return true;
}

#endif
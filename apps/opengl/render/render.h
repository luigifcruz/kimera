#ifndef RENDER_H
#define RENDER_H

#include <stdlib.h>
#include <stdio.h>

#include "state.h"
#include "helper.h"

#include "kimera/state.h"

#ifdef KIMERA_WINDOW_GLFW
#include "adapter/glfw.h"
#elif defined KIMERA_WINDOW_X11
#include "adapter/x11.h"
#endif

RenderState* alloc_render() {
    RenderState* state = (RenderState*)malloc(sizeof(RenderState));
    state->display = NULL;
    state->surface = NULL;
    state->context = NULL;
    state->adapter = alloc_adapter();
    return state;
}

void free_render(RenderState* render) {
    if (render->surface)
        eglDestroySurface(render->display, render->surface);
    if (render->context)
        eglDestroyContext(render->display, render->context);
    if (render->display)
        eglTerminate(render->display);
    if (render->adapter)
        free_adapter(render->adapter);
    free(render);
}

bool start_render(RenderState* render, State* state) {
    render->width = state->width;
    render->height = state->height;
    
    if (!gladLoadEGL()) {
        printf("[RENDER] Failed to initialize EGL.\n");
        return false;
    }

    if (render->mode == HEADLESS)
        render->display = eglGetDisplay(EGL_NO_DISPLAY);
    
    if (render->mode == WINDOWED)
        if (!load_adapter(render)) return false;

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

    if (render->mode == WINDOWED)
        if (!create_adapter(render)) return false;

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

    render->time = mticks();

    return true;
}

float render_get_time(RenderState* render) {
    return (float)((mticks() - render->time)/1000);
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
        return adapter_poll_events(render->adapter);
    }
    return true;
}

#endif
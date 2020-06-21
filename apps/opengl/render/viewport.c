#include "render.h"

bool open_viewport(RenderState* render) {
    ViewportState* view = render->view;

    view->api = EGL_OPENGL_API;
    if (render->use_opengles)
        view->api = EGL_OPENGL_ES_API;

    if (!gladLoadEGL()) {
        printf("[RENDER] Failed to initialize EGL.\n");
        return false;
    }

    view->display = eglGetDisplay(EGL_NO_DISPLAY);
    
    if (!eglInitialize(view->display, NULL, NULL)) {
        printf("[RENDER] Failed to initialize EGL.\n");
        get_egl_error(__LINE__);
        return false;
    }

    int num_config;
    if (!eglChooseConfig(view->display, egl_attr, &view->config, 1, &num_config)) {
        printf("[RENDER] Failed to config EGL.\n");
        get_egl_error(__LINE__);
        return false;
    }

    if (!eglBindAPI(view->api)) {
        printf("[RENDER] Failed to bind API (%s) to EGL.\n", render_api_query(render));
        get_egl_error(__LINE__);
        return false;
    }

    if (!render->use_display) {
        if (!(view->surface = eglCreatePbufferSurface(view->display, view->config, egl_pbuf_attr))) {
            printf("[RENDER] Error creating pbuffer surface.\n");
            get_egl_error(__LINE__);
            return false;
        }
    } else {
        if (!glfwInit()) {
            printf("[RENDER] Can't initiate GLFW.\n");
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        if (!(view->adapter = glfwCreateWindow(
            render->d_width, render->d_height, "RENDER", NULL, NULL))) {
            printf("[RENDER] Can't create GLFW window.\n");
            glfwTerminate();
            return false;
        }

        EGLNativeWindowType surface = 0;
    
        #if   defined(KIMERA_MACOS)
            surface = glfwGetCocoaWindow(view->adapter);
        #elif defined(KIMERA_LINUX)
            surface = glfwGetX11Window(view->adapter);
        #elif defined(KIMERA_WINDOWS)
            surface = glfwGetWin32Window(view->adapter);
        #endif

        view->surface = eglCreateWindowSurface(view->display, view->config, surface, 0);
        if (!view->surface) {
            printf("[RENDER] Failed to create window surface.\n");
            get_egl_error(__LINE__);
            return false;
        };
    }

    view->context = eglCreateContext(view->display, view->config, EGL_NO_CONTEXT, egl_ctx_attr);
    if (!view->context) {
        printf("[RENDER] Failed to create context.\n");
        get_egl_error(__LINE__);
        return false;
    };

    if (!eglMakeCurrent(view->display, view->surface, view->surface, view->context)) {
        printf("[RENDER] Failed to make current surface.\n");
        get_egl_error(__LINE__);
        return false;
    }

    int response = 0;

    if (view->api == EGL_OPENGL_API)
        response = gladLoadGLLoader((GLADloadproc)eglGetProcAddress);
    if (view->api == EGL_OPENGL_ES_API)
        response = gladLoadGLES2Loader((GLADloadproc)eglGetProcAddress);

    if (!response) {
        printf("[RENDER] Failed to initialize GL/GLES.\n");
        return false;
    }

    if (get_gl_error(__LINE__) || get_egl_error(__LINE__)) return false;

    return true;
}

bool viewport_render(RenderState* render) {
    if (!render->use_display)
        return true;

    eglSwapBuffers(render->view->display, render->view->surface);
    if (get_egl_error(__LINE__)) return false;
    
    glfwPollEvents();
    glfwGetWindowSize(render->view->adapter, &render->d_width, &render->d_height);
    return !glfwWindowShouldClose(render->view->adapter);
}
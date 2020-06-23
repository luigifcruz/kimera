#include "kimera/render.h"

DeviceState* init_device() {
    DeviceState* state = (DeviceState*)malloc(sizeof(DeviceState));
    return state;
}

void close_device(DeviceState* device) {
    if (device->surface)
        eglDestroySurface(device->display, device->surface);
    if (device->context)
        eglDestroyContext(device->display, device->context);
    if (device->display)
        eglTerminate(device->display);
    glfwTerminate();
}

bool open_device(RenderState* render) {
    DeviceState* device = render->device;

    device->api = EGL_OPENGL_API;
    if (render->use_opengles)
        device->api = EGL_OPENGL_ES_API;

    if (!gladLoadEGL()) {
        printf("[RENDER] Failed to initialize EGL.\n");
        return false;
    }

    device->display = eglGetDisplay(EGL_NO_DISPLAY);
    
    if (!eglInitialize(device->display, NULL, NULL)) {
        printf("[RENDER] Failed to initialize EGL.\n");
        get_egl_error(__LINE__);
        return false;
    }

    int num_config;
    if (!eglChooseConfig(device->display, egl_attr, &device->config, 1, &num_config)) {
        printf("[RENDER] Failed to config EGL.\n");
        get_egl_error(__LINE__);
        return false;
    }

    if (!eglBindAPI(device->api)) {
        printf("[RENDER] Failed to bind API (%s) to EGL.\n", render_api_query(render));
        get_egl_error(__LINE__);
        return false;
    }

    if (!render->use_display) {
        if (!(device->surface = eglCreatePbufferSurface(device->display, device->config, egl_pbuf_attr))) {
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

        if (!(device->adapter = glfwCreateWindow(
            render->d_size.w, render->d_size.h, "RENDER", NULL, NULL))) {
            printf("[RENDER] Can't create GLFW window.\n");
            glfwTerminate();
            return false;
        }

        EGLNativeWindowType surface = 0;
    
        #if   defined(KIMERA_MACOS)
            surface = glfwGetCocoaWindow(device->adapter);
        #elif defined(KIMERA_LINUX)
            surface = glfwGetX11Window(device->adapter);
        #elif defined(KIMERA_WINDOWS)
            surface = glfwGetWin32Window(device->adapter);
        #endif

        device->surface = eglCreateWindowSurface(device->display, device->config, surface, 0);
        if (!device->surface) {
            printf("[RENDER] Failed to create window surface.\n");
            get_egl_error(__LINE__);
            return false;
        };
    }

    device->context = eglCreateContext(device->display, device->config, EGL_NO_CONTEXT, egl_ctx_attr);
    if (!device->context) {
        printf("[RENDER] Failed to create context.\n");
        get_egl_error(__LINE__);
        return false;
    };

    if (!eglMakeCurrent(device->display, device->surface, device->surface, device->context)) {
        printf("[RENDER] Failed to make current surface.\n");
        get_egl_error(__LINE__);
        return false;
    }

    int response = 0;

    if (device->api == EGL_OPENGL_API)
        response = gladLoadGLLoader((GLADloadproc)eglGetProcAddress);
    if (device->api == EGL_OPENGL_ES_API)
        response = gladLoadGLES2Loader((GLADloadproc)eglGetProcAddress);

    if (!response) {
        printf("[RENDER] Failed to initialize GL/GLES.\n");
        return false;
    }

    if (get_gl_error(__LINE__) || get_egl_error(__LINE__)) return false;

    return true;
}

bool device_render(RenderState* render) {
    if (!render->use_display)
        return true;

    eglSwapBuffers(render->device->display, render->device->surface);
    if (get_egl_error(__LINE__)) return false;
    
    glfwPollEvents();
    glfwGetWindowSize(render->device->adapter, &render->d_size.w, &render->d_size.h);
    return !glfwWindowShouldClose(render->device->adapter);
}
#include "kimera/render/opengl/device.hpp"

namespace Kimera {

void key_callback(GLFWwindow*, int key, int, int action, int) {
    if (key == GLFW_KEY_Q && action == GLFW_RELEASE)
        raise(SIGINT);

    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
        raise(SIGINT);
}

Device::Device(State& state, int width, int height, bool use_display) : state(state) {
    switch (state.mode) {
        case Mode::TRANSMITTER:
            window_name = "Kimera - Transmitter Display";
            break;
        case Mode::RECEIVER:
            window_name = "Kimera - Receiver Display";
            break;
        case Mode::NONE:
            window_name = "Kimera - No Receiver";
            break;
    }

    api = EGL_OPENGL_ES_API;

    if (!gladLoadEGL()) {
        printf("[EGL] Failed to initialize EGL.\n");
        throw "error";
    }

    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        printf("[EGL] Failed to get device display.\n");
        GetError("Device::Device()");
        throw "error";
    }

    if (!eglInitialize(display, NULL, NULL)) {
        printf("[EGL] Failed to initialize EGL.\n");
        GetError("Device::Device()");
        throw "error";
    }

    int num_config;
    if (!eglChooseConfig(display, egl_attr, &config, 1, &num_config)) {
        printf("[EGL] Failed to config EGL.\n");
        GetError("Device::Device()");
        throw "error";
    }

    if (!eglBindAPI(api)) {
        printf("[EGL] Failed to bind API to EGL.\n");
        GetError("Device::Device()");
        throw "error";
    }

    if (!use_display) {
        if (!(surface = eglCreatePbufferSurface(display, config, egl_pbuf_attr))) {
            printf("[EGL] Error creating pbuffer surface.\n");
            GetError("Device::Device()");
            throw "error";
        }
    } else {
        if (!glfwInit()) {
            printf("[EGL] Can't initiate GLFW.\n");
            throw "error";
        }

        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        if (!(adapter = glfwCreateWindow(width, height, window_name.c_str(), NULL, NULL))) {
            printf("[EGL] Can't create GLFW window.\n");
            glfwTerminate();
            throw "error";
        }

        EGLNativeWindowType native_surface = 0;

        #if   defined(KIMERA_MACOS)
            native_surface = glfwGetCocoaWindow(adapter);
        #elif defined(KIMERA_LINUX)
            native_surface = glfwGetX11Window(adapter);
        #elif defined(KIMERA_WINDOWS)
            native_surface = glfwGetWin32Window(adapter);
        #endif

        glfwSetKeyCallback(adapter, key_callback);

        surface = eglCreateWindowSurface(display, config, native_surface, 0);
        if (!surface) {
            printf("[EGL] Failed to create window surface.\n");
            GetError("Device::Device()");
            throw "error";
        };
    }

    if (!(context = eglCreateContext(display, config, EGL_NO_CONTEXT, egl_ctx_attr))) {
        printf("[EGL] Failed to create context.\n");
        GetError("Device::Device()");
        throw "error";
    };

    if (!eglMakeCurrent(display, surface, surface, context)) {
        printf("[EGL] Failed to make current surface.\n");
        GetError("Device::Device()");
        throw "error";
    }

    int response = 0;

    if (api == EGL_OPENGL_API)
        response = gladLoadGLLoader((GLADloadproc)eglGetProcAddress);
    if (api == EGL_OPENGL_ES_API)
        response = gladLoadGLES2Loader((GLADloadproc)eglGetProcAddress);

    if (!response) {
        printf("[EGL] Failed to initialize GL/GLES.\n");
        throw "error";
    }

    if (GetError("Device::Device()")) throw "error";
}

Device::~Device() {
    if (surface) eglDestroySurface(display, surface);
    if (context) eglDestroyContext(display, context);
    if (display) eglTerminate(display);
    if (adapter) glfwDestroyWindow(adapter);
    glfwTerminate();
}

bool Device::GetError(std::string line) {
    int error = eglGetError();
    if (error != EGL_SUCCESS) {
        std::cout << "[EGL] EGL returned an error #" << error
                  << " at " << line << "." << std::endl;
        return true;
    }
    return false;
}

// name: EGL_CLIENT_APIS, EGL_VENDOR, EGL_VERSION, EGL_EXTENSIONS
const char* Device::Query(int name) {
    return eglQueryString(display, name);
}

bool Device::Step(int* w_width, int* w_height) {
    eglSwapBuffers(display, surface);
    if (GetError("Device::Step()")) return false;

    glfwPollEvents();
    glfwGetWindowSize(adapter, w_width, w_height);
    return !glfwWindowShouldClose(adapter);
}

} // namespace Kimera

#include "kimera/render.h"

// name: EGL_CLIENT_APIS, EGL_VENDOR, EGL_VERSION, EGL_EXTENSIONS
const char* egl_query(RenderState* render, int name) {
    return eglQueryString(render->device->display, name);
}

// name: GL_VENDOR, GL_RENDERER, GL_VERSION, GL_SHADING_LANGUAGE_VERSION, GL_EXTENSIONS
const char* gl_query(GLenum name) {
    return (const char*)glGetString(name);
}

const char* render_mode_query(RenderState* render) {
    return (render->use_display) ? "WINDOWED" : "HEADLESS";
}

const char* render_api_query(RenderState* render) {
    return (render->use_opengles) ? "OpenGL ES" : "OpenGL";
}

void render_print_meta(RenderState* render) {
    printf(".   [Render Meta]\n");
    printf("├── Window Mode:   %s\n", render_mode_query(render));
    printf("├── Frame Size:    %dx%d\n", render->f_size.w, render->f_size.h);
    printf("├── Device Size:   %dx%d\n", render->d_size.w, render->d_size.h);
    printf("└──.   [EGL Meta]\n");
    printf("│  ├── APIs:       %s\n", egl_query(render, EGL_CLIENT_APIS));
    printf("│  ├── Version:    %s\n", egl_query(render, EGL_VERSION));
    printf("│  └── Vendor:     %s\n", egl_query(render, EGL_VENDOR));
    printf("└──.   [GL Meta]\n");
    printf("   ├── Renderer:   %s\n", gl_query(GL_RENDERER));
    printf("   ├── Version:    %s\n", gl_query(GL_VERSION));
    printf("   ├── Vendor:     %s\n", gl_query(GL_VENDOR));
    printf("   └── GLSL Ver.:  %s\n", gl_query(GL_SHADING_LANGUAGE_VERSION));
}

void render_print_ext(RenderState* render) {
    printf(".   [Render Extensions]\n");
    printf("├── EGL Extensions: %s\n", egl_query(render, EGL_EXTENSIONS));
    printf("└── GL Extensions:  %s\n", gl_query(GL_EXTENSIONS));
}

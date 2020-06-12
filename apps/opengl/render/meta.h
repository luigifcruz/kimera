#ifndef RENDER_META_H
#define RENDER_META_H

#include <stdio.h>

#include "state.h"

// name: EGL_CLIENT_APIS, EGL_VENDOR, EGL_VERSION, EGL_EXTENSIONS
const char* render_egl_query(RenderState* render, int name) {
    return eglQueryString(render->display, name);
}

// name: GL_VENDOR, GL_RENDERER, GL_VERSION, GL_SHADING_LANGUAGE_VERSION, GL_EXTENSIONS
const char* render_gl_query(GLenum name) {
    return (const char*)glGetString(name);
}

const char* render_mode_query(RenderState* render) {
    return (render->mode == WINDOWED) ? "WINDOWED" : "HEADLESS";
}

void render_print_meta(RenderState* render) {
    printf(".   [Render Meta]\n");
    printf("├── Window Mode:   %s\n", render_mode_query(render));
    printf("├── Viewport Size: %dx%d\n", render->width, render->height);
    printf("└──.   [EGL Meta]\n");
    printf("│  ├── APIs:       %s\n", render_egl_query(render, EGL_CLIENT_APIS));
    printf("│  ├── Version:    %s\n", render_egl_query(render, EGL_VERSION));
    printf("│  └── Vendor:     %s\n", render_egl_query(render, EGL_VENDOR));
    printf("└──.   [GL Meta]\n");
    printf("   ├── Renderer:   %s\n", render_gl_query(GL_RENDERER));
    printf("   ├── Version:    %s\n", render_gl_query(GL_VERSION));
    printf("   ├── Vendor:     %s\n", render_gl_query(GL_VENDOR));
    printf("   └── GLSL Ver.:  %s\n", render_gl_query(GL_SHADING_LANGUAGE_VERSION));
}

void render_print_ext(RenderState* render) {
    printf(".   [Render Extensions]\n");
    printf("├── EGL Extensions: %s\n", render_egl_query(render, EGL_EXTENSIONS));
    printf("└── GL Extensions:  %s\n", render_gl_query(GL_EXTENSIONS));
}

#endif
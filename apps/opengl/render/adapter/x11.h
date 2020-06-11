#ifndef RENDER_WINDOW_X11_H
#define RENDER_WINDOW_X11_H

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>

#include "../glad/glad_egl.h"
#include "../types.h"

typedef struct {
    Display* display;
    Window surface;
} WindowState;

void* alloc_window() {
    WindowState* state = (WindowState*)malloc(sizeof(WindowState));
    state->display = NULL;
    return state;
}

void free_window(void* window_ptr) {
    WindowState* window = (WindowState*)window_ptr;
    XDestroyWindow(window->display, window->surface);
    free(window);
}

bool window_poll_events(void* window_ptr) {
    WindowState* window = (WindowState*)window_ptr;

    return true;
}

bool load_window(RenderState* render) {
    WindowState* window = (WindowState*)render->window;

	if (!(window->display = XOpenDisplay(NULL))) {
        printf("[RENDER] Can't open X11 display.\n");
		return false;
	}

    render->display = eglGetDisplay(window->display);
	if (render->display == EGL_NO_DISPLAY) {
        printf("[RENDER] Can't get X11 display to EGL.\n");
		return false;
	}

    return true;
}

bool create_window(RenderState* render) {
    WindowState* window = (WindowState*)render->window;
    
    EGLint visual_id;
    if (!eglGetConfigAttrib(render->display, render->config, EGL_NATIVE_VISUAL_ID, &visual_id)) {
        printf("[RENDER] Can't get EGL configuration.\n");
        return false;
    }

    int num_visuals;
    XVisualInfo *info;
    XVisualInfo visual_template;
    visual_template.visualid = visual_id;
    if (!(info = XGetVisualInfo(window->display, VisualIDMask, &visual_template, &num_visuals))) {
        printf("[RENDER Can't get X11 visual information.\n");
        return false;
    }

    int screen = DefaultScreen(window->display);
    Window root = RootWindow(window->display, screen);

    XSetWindowAttributes attr;
    attr.background_pixel = 0;
	attr.border_pixel = 0;
	attr.colormap = XCreateColormap(window->display, root, info->visual, AllocNone);
	attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;

	unsigned long mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;
    window->surface = XCreateWindow(window->display, root, 0, 0, render->state->width, render->state->height,
		                            0, info->depth, InputOutput, info->visual, mask, &attr);
    if (!window->surface) {
        printf("[RENDER] Can't create X11 window.\n");
        return false;
    }

    XSizeHints hints;
    hints.width = render->state->width;
    hints.height = render->state->height;
    hints.flags = USSize;

    const char* name = "GL Render Monitor";
    XSetNormalHints(window->display, window->surface, &hints);
	XSetStandardProperties(window->display, window->surface, name, name, None, NULL, 0, &hints);

    render->surface = eglCreateWindowSurface(render->display, render->config, window->surface, 0);
    if (!render->surface) {
        printf("[RENDER] Failed to create window surface.\n");
        get_egl_error(__LINE__);
        return false;
    }

    XFree(info);
    XMapWindow(window->display, window->surface);
    XFlush(window->display);

    return true;
}

#endif
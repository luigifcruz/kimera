#include "kimera/loopback/linux/xcb.h"

void init_xcb(LoopbackState* loopback) {
    loopback->xcb = (XCBState*)malloc(sizeof(XCBState));
}

void close_xcb(LoopbackState* loopback) {
    xcb_disconnect(loopback->xcb->connection);
}

bool init_xcb_source(LoopbackState* loopback, State* state) {
    init_xcb(loopback);

    int error;
    loopback->xcb->connection = xcb_connect(state->loopback, NULL);
    if ((error = xcb_connection_has_error(loopback->xcb->connection))) {
        printf("[LOOPBACK] Cannot open selected display %s, error %d.\n", state->loopback, error);
        return false;
    }

    const xcb_setup_t *setup = xcb_get_setup(loopback->xcb->connection);
    loopback->xcb->screen = xcb_setup_roots_iterator(setup).data;
    loopback->xcb->img = NULL;
    
    AVRational time_base = (AVRational){ 1, state->framerate };
    loopback->frame_duration = av_rescale_q(1, time_base, AV_TIME_BASE_Q);

    state->in_format = 	AV_PIX_FMT_RGB32;

    loopback->frame = av_frame_alloc();
    loopback->frame->width = loopback->xcb->screen->width_in_pixels;
    loopback->frame->height = loopback->xcb->screen->height_in_pixels;
    loopback->frame->format = state->in_format;
    loopback->frame->pts = 0;
    if (av_frame_get_buffer(loopback->frame, 0) < 0){
        printf("[LOOPBACK] Couldn't allocate frame.\n");
        return false;
    }

    return true;
}

bool pull_xcb_frame(LoopbackState* loopback, State* state) {
    xcb_get_image_cookie_t gi_cookie;
    xcb_generic_error_t *e = NULL;

    if (loopback->xcb->img)
        free(loopback->xcb->img);

    int64_t curtime = av_gettime();
    int64_t delay = (loopback->last_frame + loopback->frame_duration) - curtime;

    if (delay > 0)
        av_usleep(delay);

    gi_cookie = xcb_get_image(loopback->xcb->connection, XCB_IMAGE_FORMAT_Z_PIXMAP, loopback->xcb->screen->root, 0, 0, 
                              loopback->xcb->screen->width_in_pixels, loopback->xcb->screen->height_in_pixels, ~0);
    loopback->xcb->img = xcb_get_image_reply(loopback->xcb->connection, gi_cookie, &e);
    if (e) {
        printf("[LOOPBACK] Error getting XCB image.\n");
        return false;
    }

    loopback->last_frame = av_gettime();
    loopback->frame->pts += 1;

    if (av_image_fill_arrays(
        loopback->frame->data, loopback->frame->linesize,
        xcb_get_image_data(loopback->xcb->img), loopback->frame->format,
        loopback->xcb->screen->width_in_pixels, loopback->xcb->screen->height_in_pixels, 1) < 0) {
        return false;
    }

    return true;
}
#include "loopback.h"

bool init_xcb_source(LoopbackState* loopback, State* state) {
    loopback->buffer = NULL;

    int error;
    loopback->connection = xcb_connect(state->loopback, NULL);
    if ((error = xcb_connection_has_error(loopback->connection))) {
        printf("[LOOPBACK] Cannot open selected display %s, error %d.\n", state->loopback, error);
        return false;
    }

    const xcb_setup_t *setup = xcb_get_setup(loopback->connection);
    loopback->screen = xcb_setup_roots_iterator(setup).data;

    if (loopback->screen->width_in_pixels != state->width ||
        loopback->screen->height_in_pixels != state->height) {
        printf("Screen size (%dx%d) mismatch from configuration file.\n",
               loopback->screen->width_in_pixels, loopback->screen->height_in_pixels);
        return false;
    }

    AVRational time_base = (AVRational){ 1, state->framerate };
    loopback->frame_duration = av_rescale_q(1, time_base, AV_TIME_BASE_Q);

    state->in_format = AV_PIX_FMT_0RGB32;

    loopback->frame = av_frame_alloc();
    loopback->frame->width = state->width;
    loopback->frame->height = state->height;
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
    
    free(loopback->img);

    int64_t curtime = av_gettime();
    int64_t delay = (loopback->last_frame + loopback->frame_duration) - curtime;

    if (delay > 0)
        av_usleep(delay);

    gi_cookie = xcb_get_image(loopback->connection, XCB_IMAGE_FORMAT_Z_PIXMAP, loopback->screen->root, 0, 0, 
                              loopback->screen->width_in_pixels, loopback->screen->height_in_pixels, ~0);
    loopback->img = xcb_get_image_reply(loopback->connection, gi_cookie, &e);
    if (e) {
        printf("[LOOPBACK] Error getting XCB image.\n");
        return false;
    }

    loopback->last_frame = av_gettime();
    loopback->frame->pts += 1;

    if (av_image_fill_arrays(loopback->frame->data, loopback->frame->linesize,
            xcb_get_image_data(loopback->img), loopback->frame->format,
            state->width, state->height, 1) < 0) {
        return false;
    }

    return true;
}
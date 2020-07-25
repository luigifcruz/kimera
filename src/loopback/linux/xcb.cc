#include "kimera/loopback/linux.hpp"

XCB::XCB(State& state) : state(state) {}

XCB::~XCB() {
    xcb_disconnect(this->connection);
}

bool XCB::SetSource() {
    int error;
    this->connection = xcb_connect(state.loopback.c_str(), NULL);
    if ((error = xcb_connection_has_error(this->connection))) {
        printf("[LOOPBACK] Cannot open selected display %s, error %d.\n", state.loopback.c_str(), error);
        return false;
    }

    const xcb_setup_t *setup = xcb_get_setup(this->connection);
    this->screen = xcb_setup_roots_iterator(setup).data;
    this->img = NULL;

    AVRational time_base = (AVRational){ 1, state.framerate };
    frame_duration = av_rescale_q(1, time_base, AV_TIME_BASE_Q);

    state.in_format = 	AV_PIX_FMT_BGRA;

    frame = av_frame_alloc();
    frame->width = screen->width_in_pixels;
    frame->height = screen->height_in_pixels;
    frame->format = state.in_format;
    frame->pts = 0;
    if (av_frame_get_buffer(frame, 0) < 0){
        printf("[LOOPBACK] Couldn't allocate frame.\n");
        return false;
    }

    return true;
}

AVFrame* XCB::Pull() {
    xcb_get_image_cookie_t gi_cookie;
    xcb_generic_error_t *e = NULL;

    if (this->img)
        free(this->img);

    int64_t curtime = av_gettime();
    int64_t delay = (last_frame + frame_duration) - curtime;

    if (delay > 0)
        av_usleep(delay);

    gi_cookie = xcb_get_image(this->connection, XCB_IMAGE_FORMAT_Z_PIXMAP, this->screen->root, 0, 0,
                              this->screen->width_in_pixels, this->screen->height_in_pixels, ~0);
    this->img = xcb_get_image_reply(this->connection, gi_cookie, &e);
    if (e) {
        printf("[LOOPBACK] Error getting XCB image.\n");
        return NULL;
    }

    last_frame = av_gettime();
    frame->pts += 1;

    if (av_image_fill_arrays(
        frame->data, frame->linesize,
        xcb_get_image_data(this->img), (AVPixelFormat)frame->format,
        this->screen->width_in_pixels, this->screen->height_in_pixels, 1) < 0) {
        return NULL;
    }

    return frame;
}

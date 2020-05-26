#include "loopback.h"

bool open_loopback_sink(LoopbackState* loopback, State* state) {
    loopback->buffer = NULL;
    
	if ((loopback->dev_fd = open(state->loopback, O_RDWR)) < 0) {
        printf("[LOOPBACK] Couldn't open interface.\n");
        return false;
	}

    unsigned int frame_size = 0;
    switch (state->out_format) {
    case AV_PIX_FMT_YUV420P:
        frame_size = (state->width*state->height*3/2);
        break;
    default:
        return false;
    }

	loopback->format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	loopback->format.fmt.pix.width = state->width;
	loopback->format.fmt.pix.height = state->height;
	loopback->format.fmt.pix.sizeimage = frame_size;
	loopback->format.fmt.pix.field = V4L2_FIELD_NONE;
    
	if (ioctl(loopback->dev_fd, VIDIOC_S_FMT, &loopback->format) < 0) {
        printf("[LOOPBACK] Couldn't open interface.\n");
        return false;
	}

    loopback->buffer = (char*)malloc(frame_size);
    if (!loopback->buffer) {
        printf("[LOOPBACK] Couldn't allocate loopback buffer.\n");
        return false;
    }

	return true;
}

bool init_xcb(LoopbackState* loopback, State* state) {
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

bool init_loopback(LoopbackState* loopback, State* state) {
    loopback->buffer = NULL;

	if ((loopback->dev_fd = open(state->loopback, O_RDWR)) < 0) {
        printf("[LOOPBACK] Couldn't open loopback interface.\n");
        return false;
	}

    state->in_format = v4l_to_ff(
        find_v4l_format(loopback->dev_fd, ff_to_v4l(state->in_format)));

    loopback->format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	loopback->format.fmt.pix.width = state->width;
	loopback->format.fmt.pix.height = state->height;
    loopback->format.fmt.pix.field = V4L2_FIELD_ANY;    
    loopback->format.fmt.pix.pixelformat = ff_to_v4l(state->in_format);

	if (ioctl(loopback->dev_fd, VIDIOC_S_FMT, &loopback->format) < 0) {
        printf("[LOOPBACK] Couldn't open interface.\n");
        return false;
	}

    if (loopback->format.fmt.pix.field == V4L2_FIELD_INTERLACED) {
        printf("[LOOPBACK] The V4L2 driver changed field to interleaved.\n");
        return false;
    }
    
    loopback->req.count = 1;
    loopback->req.memory = V4L2_MEMORY_MMAP;
    loopback->req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(loopback->dev_fd, VIDIOC_REQBUFS, &loopback->req) < 0) {
        printf("[LOOPBACK] Couldn't request the buffer.\n");
        return false;
	}

    loopback->info.index = 0;
    loopback->info.memory = V4L2_MEMORY_MMAP;
    loopback->info.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(loopback->dev_fd, VIDIOC_QUERYBUF, &loopback->info) < 0) {
        printf("[LOOPBACK] Couldn't query the buffer.\n");
        return false;
	}

    loopback->buffer = mmap(
        NULL,
        loopback->info.length,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        loopback->dev_fd,
        loopback->info.m.offset);

    if (loopback->buffer == MAP_FAILED) {
        printf("[LOOPBACK] Failed to allocate buffer.\n");
        return false;
    }

    memset(loopback->buffer, 0, loopback->info.length);

    if (ioctl(loopback->dev_fd, VIDIOC_QBUF, &loopback->info) < 0) {
        printf("[LOOPBACK] Error queueing stream.\n");
        return false;
    }

    if (ioctl(loopback->dev_fd, VIDIOC_STREAMON, &loopback->info.type) < 0) {
        printf("[LOOPBACK] Error starting the stream.\n");
        return false;
    }

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

bool loopback_push_frame(LoopbackState* loopback, AVFrame* frame) {
    if (av_image_copy_to_buffer((uint8_t*)loopback->buffer, loopback->info.length,
            (const uint8_t* const*)frame->data, frame->linesize,
            frame->format, frame->width, frame->height, 1) < 0) {
        return false;
    }
    
    if (write(loopback->dev_fd, loopback->buffer, loopback->info.length) < 0) {
        return false;
    }
 
    return true;
}

inline bool open_loopback_source(LoopbackState* loopback, State* state) {
    if (state->source & DISPLAY)
        return init_xcb(loopback, state);
    if (state->source & LOOPBACK)
        return init_loopback(loopback, state);
    return false;
}

bool load_xcb_frame(LoopbackState* loopback, State* state) {
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

bool load_loopback_frame(LoopbackState* loopback) {
    if (ioctl(loopback->dev_fd, VIDIOC_DQBUF, &loopback->info) < 0) {
        printf("[LOOPBACK] Error dequeuing stream.\n");
        return false;
    }

    if (av_image_fill_arrays(loopback->frame->data, loopback->frame->linesize,
            (uint8_t*)loopback->buffer, loopback->frame->format,
            loopback->frame->width, loopback->frame->height, 1) < 0) {
        return false;
    }

    loopback->frame->pts += 1;

    if (ioctl(loopback->dev_fd, VIDIOC_QBUF, &loopback->info) < 0) {
        printf("[LOOPBACK] Error queueing stream.\n");
        return false;
    }

    return true;
}

inline bool loopback_pull_frame(LoopbackState* loopback, State* state) {
    if (state->source & DISPLAY)
        return load_xcb_frame(loopback, state);
    if (state->source & LOOPBACK)
        return load_loopback_frame(loopback);
    return false;
}

void close_loopback(LoopbackState* loopback) {
    if (loopback->format.type == V4L2_BUF_TYPE_VIDEO_CAPTURE) {
        ioctl(loopback->dev_fd, VIDIOC_STREAMOFF, &loopback->info.type);
        if (loopback->frame)
            av_frame_free(&loopback->frame);
    } else {
        if (loopback->buffer)
            free(loopback->buffer);
    }
}
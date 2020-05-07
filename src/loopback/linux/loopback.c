#include "loopback.h"

bool open_loopback_sink(LoopbackState* loopback, State* state) {
    loopback->buffer = NULL;
    
	if ((loopback->dev_fd = open(state->loopback, O_RDWR)) < 0) {
        printf("[LOOPBACK] Couldn't open interface.\n");
        return false;
	}

	loopback->format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	if (ioctl(loopback->dev_fd, VIDIOC_G_FMT, state->format) < 0) {
        printf("[LOOPBACK] Couldn't open interface.\n");
        return false;
	}

	loopback->format.fmt.pix.width = state->width;
	loopback->format.fmt.pix.height = state->height;

    switch (state->format) {
    case AV_PIX_FMT_YUV420P:
        loopback->format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
        break;
    default:
        printf("[LOOPBACK] Selected pixel format not supported by Linux loopback.\n");
        return false;
    }
    
	loopback->format.fmt.pix.sizeimage = state->frame_size;
	loopback->format.fmt.pix.field = V4L2_FIELD_NONE;
	if (ioctl(loopback->dev_fd, VIDIOC_S_FMT, &loopback->format) < 0) {
        printf("[LOOPBACK] Couldn't open interface.\n");
        return false;
	}

    loopback->buffer = (char*)malloc(state->frame_size);
    if (!loopback->buffer) {
        printf("[LOOPBACK] Couldn't allocate loopback buffer.\n");
        return false;
    }

	return true;
}

bool open_loopback_source(LoopbackState* loopback, State* state) {
    loopback->buffer = NULL;

	if ((loopback->dev_fd = open(state->loopback, O_RDWR)) < 0) {
        printf("[LOOPBACK] Couldn't open interface.\n");
        return false;
	}

    loopback->format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	loopback->format.fmt.pix.width = state->width;
	loopback->format.fmt.pix.height = state->height;

    switch (state->format) {
    case AV_PIX_FMT_YUV420P:
        loopback->format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
        break;
    default:
        printf("[LOOPBACK] Selected pixel format not supported by Linux loopback.\n");
        return false;
    }
	
	loopback->format.fmt.pix.sizeimage = state->frame_size;
	if (ioctl(loopback->dev_fd, VIDIOC_S_FMT, &loopback->format) < 0) {
        printf("[LOOPBACK] Couldn't open interface.\n");
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

    if (ioctl(loopback->dev_fd, VIDIOC_STREAMON, &loopback->info.type) < 0) {
        printf("[LOOPBACK] Error starting the stream.\n");
        return false;
    }

    loopback->frame = av_frame_alloc();
    loopback->frame->width = state->width;
    loopback->frame->height = state->height;
    loopback->frame->format = state->format;
    loopback->frame->pts = 0;
    if (av_frame_get_buffer(loopback->frame, 0) < 0){
        printf("[LOOPBACK] Couldn't allocate frame.\n");
        return false;
    }

	return true;
}

bool loopback_push_frame(LoopbackState* loopback, AVFrame* frame) {
    size_t y_len = (frame->linesize[0] * frame->height);
    size_t u_len = (frame->linesize[1] * frame->height) / 2;
    size_t v_len = (frame->linesize[2] * frame->height) / 2;
    size_t len = y_len + u_len + v_len;

    memcpy(loopback->buffer, frame->data[0], y_len);
    memcpy(loopback->buffer + y_len, frame->data[1], u_len);
    memcpy(loopback->buffer + u_len + y_len, frame->data[2], v_len);

    if (write(loopback->dev_fd, loopback->buffer, len) < 0) {
        return false;
    }

    return true;
}

bool loopback_pull_frame(LoopbackState* loopback) {
    if (ioctl(loopback->dev_fd, VIDIOC_DQBUF, &loopback->info) < 0) {
        printf("[LOOPBACK] Error dequeuing stream.\n");
        return false;
    }

    size_t y_len = (loopback->frame->linesize[0] * loopback->frame->height);
    size_t u_len = (loopback->frame->linesize[1] * loopback->frame->height) / 2;
    size_t v_len = (loopback->frame->linesize[2] * loopback->frame->height) / 2;

    memcpy(loopback->frame->data[0], loopback->buffer, y_len);
    memcpy(loopback->frame->data[1], loopback->buffer + y_len, u_len);
    memcpy(loopback->frame->data[2], loopback->buffer + u_len + y_len, v_len);

    loopback->frame->pts += 1;

    if (ioctl(loopback->dev_fd, VIDIOC_QBUF, &loopback->info) < 0) {
        printf("[LOOPBACK] Error queueing stream.\n");
        return false;
    }

    return true;
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
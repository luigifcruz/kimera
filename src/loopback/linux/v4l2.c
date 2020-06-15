#include "kimera/loopback/linux.h"

void init_v4l2(LoopbackState* loopback) {
    loopback->v4l2 = (V4L2State*)malloc(sizeof(V4L2State));
    loopback->v4l2->buffer = NULL;
}

void close_v4l2(LoopbackState* loopback, State* state) {
    if (state->mode & TRANSMITTER) {
        ioctl(loopback->v4l2->dev_fd, VIDIOC_STREAMOFF, &loopback->v4l2->info.type);
    }

    if (state->mode & RECEIVER) {
        if (loopback->v4l2->buffer)
            free(loopback->v4l2->buffer);
    }
}

bool init_v4l2_source(LoopbackState* loopback, State* state) {
    init_v4l2(loopback);

	if ((loopback->v4l2->dev_fd = open(state->loopback, O_RDWR)) < 0) {
        printf("[LOOPBACK] Couldn't open loopback interface.\n");
        return false;
	}

    state->in_format = v4l_to_ff(
        find_v4l_format(loopback->v4l2->dev_fd, ff_to_v4l(state->in_format)));

    loopback->v4l2->format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	loopback->v4l2->format.fmt.pix.width = state->width;
	loopback->v4l2->format.fmt.pix.height = state->height;
    loopback->v4l2->format.fmt.pix.field = V4L2_FIELD_ANY;    
    loopback->v4l2->format.fmt.pix.pixelformat = ff_to_v4l(state->in_format);

	if (ioctl(loopback->v4l2->dev_fd, VIDIOC_S_FMT, &loopback->v4l2->format) < 0) {
        printf("[LOOPBACK] Couldn't open interface.\n");
        return false;
	}

    if (loopback->v4l2->format.fmt.pix.field == V4L2_FIELD_INTERLACED) {
        printf("[LOOPBACK] The V4L2 driver changed field to interleaved.\n");
        return false;
    }
    
    loopback->v4l2->req.count = 1;
    loopback->v4l2->req.memory = V4L2_MEMORY_MMAP;
    loopback->v4l2->req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(loopback->v4l2->dev_fd, VIDIOC_REQBUFS, &loopback->v4l2->req) < 0) {
        printf("[LOOPBACK] Couldn't request the buffer.\n");
        return false;
	}

    loopback->v4l2->info.index = 0;
    loopback->v4l2->info.memory = V4L2_MEMORY_MMAP;
    loopback->v4l2->info.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(loopback->v4l2->dev_fd, VIDIOC_QUERYBUF, &loopback->v4l2->info) < 0) {
        printf("[LOOPBACK] Couldn't query the buffer.\n");
        return false;
	}

    loopback->v4l2->buffer = mmap(
        NULL, loopback->v4l2->info.length,
        PROT_READ | PROT_WRITE, MAP_SHARED,
        loopback->v4l2->dev_fd, loopback->v4l2->info.m.offset);

    if (loopback->v4l2->buffer == MAP_FAILED) {
        printf("[LOOPBACK] Failed to allocate buffer.\n");
        return false;
    }

    memset(loopback->v4l2->buffer, 0, loopback->v4l2->info.length);

    if (ioctl(loopback->v4l2->dev_fd, VIDIOC_QBUF, &loopback->v4l2->info) < 0) {
        printf("[LOOPBACK] Error queueing stream.\n");
        return false;
    }

    if (ioctl(loopback->v4l2->dev_fd, VIDIOC_STREAMON, &loopback->v4l2->info.type) < 0) {
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

bool pull_v4l2_frame(LoopbackState* loopback) {
    if (ioctl(loopback->v4l2->dev_fd, VIDIOC_DQBUF, &loopback->v4l2->info) < 0) {
        printf("[LOOPBACK] Error dequeuing stream.\n");
        return false;
    }

    if (av_image_fill_arrays(loopback->frame->data, loopback->frame->linesize,
            (uint8_t*)loopback->v4l2->buffer, loopback->frame->format,
            loopback->frame->width, loopback->frame->height, 1) < 0) {
        return false;
    }

    loopback->frame->pts += 1;

    if (ioctl(loopback->v4l2->dev_fd, VIDIOC_QBUF, &loopback->v4l2->info) < 0) {
        printf("[LOOPBACK] Error queueing stream.\n");
        return false;
    }

    return true;
}

bool init_v4l2_sink(LoopbackState* loopback, State* state) {
    init_v4l2(loopback);
    
	if ((loopback->v4l2->dev_fd = open(state->loopback, O_RDWR)) < 0) {
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

	loopback->v4l2->format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	loopback->v4l2->format.fmt.pix.width = state->width;
	loopback->v4l2->format.fmt.pix.height = state->height;
	loopback->v4l2->format.fmt.pix.sizeimage = frame_size;
	loopback->v4l2->format.fmt.pix.field = V4L2_FIELD_NONE;

	if (ioctl(loopback->v4l2->dev_fd, VIDIOC_S_FMT, &loopback->v4l2->format) < 0) {
        printf("[LOOPBACK] Couldn't open interface.\n");
        return false;
	}

    loopback->v4l2->buffer = (char*)malloc(frame_size);
    if (!loopback->v4l2->buffer) {
        printf("[LOOPBACK] Couldn't allocate loopback buffer.\n");
        return false;
    }

	return true;
}

bool push_v4l2_frame(LoopbackState* loopback, AVFrame* frame) {
    if (av_image_copy_to_buffer((uint8_t*)loopback->v4l2->buffer, loopback->v4l2->info.length,
            (const uint8_t* const*)frame->data, frame->linesize,
            frame->format, frame->width, frame->height, 1) < 0) {
        return false;
    }
    
    if (write(loopback->v4l2->dev_fd, loopback->v4l2->buffer, loopback->v4l2->info.length) < 0) {
        return false;
    }
 
    return true;
}
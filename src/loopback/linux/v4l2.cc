#include "kimera/loopback/linux.hpp"

V4L2::V4L2(Kimera* state) {
    this->state = state;
}

V4L2::~V4L2() {
    if (this->state->mode & TRANSMITTER) {
        ioctl(this->dev_fd, VIDIOC_STREAMOFF, &this->info.type);
        if (frame)
            av_frame_free(&frame);
    }

    if (this->state->mode & RECEIVER) {
        if (this->buffer)
            free(this->buffer);
    }
}

bool V4L2::SetSource() {
	if ((this->dev_fd = open(state->loopback, O_RDWR)) < 0) {
        printf("[LOOPBACK] Couldn't open loopback interface.\n");
        return false;
	}

    state->in_format = v4l_to_ff(
        find_v4l_format(this->dev_fd, ff_to_v4l(state->in_format)));

    this->format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	this->format.fmt.pix.width = state->width;
	this->format.fmt.pix.height = state->height;
    this->format.fmt.pix.field = V4L2_FIELD_ANY;
    this->format.fmt.pix.pixelformat = ff_to_v4l(state->in_format);

	if (ioctl(this->dev_fd, VIDIOC_S_FMT, &this->format) < 0) {
        printf("[LOOPBACK] Couldn't open interface.\n");
        return false;
	}

    if (this->format.fmt.pix.field == V4L2_FIELD_INTERLACED) {
        printf("[LOOPBACK] The V4L2 driver changed field to interleaved.\n");
        return false;
    }

    this->req.count = 1;
    this->req.memory = V4L2_MEMORY_MMAP;
    this->req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(this->dev_fd, VIDIOC_REQBUFS, &this->req) < 0) {
        printf("[LOOPBACK] Couldn't request the buffer.\n");
        return false;
	}

    this->info.index = 0;
    this->info.memory = V4L2_MEMORY_MMAP;
    this->info.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(this->dev_fd, VIDIOC_QUERYBUF, &this->info) < 0) {
        printf("[LOOPBACK] Couldn't query the buffer.\n");
        return false;
	}

    this->buffer = (char*)mmap(
        (void*)NULL, this->info.length,
        PROT_READ | PROT_WRITE, MAP_SHARED,
        this->dev_fd, this->info.m.offset);

    if (this->buffer == MAP_FAILED) {
        printf("[LOOPBACK] Failed to allocate buffer.\n");
        return false;
    }

    memset(this->buffer, 0, this->info.length);

    if (ioctl(this->dev_fd, VIDIOC_QBUF, &this->info) < 0) {
        printf("[LOOPBACK] Error queueing stream.\n");
        return false;
    }

    if (ioctl(this->dev_fd, VIDIOC_STREAMON, &this->info.type) < 0) {
        printf("[LOOPBACK] Error starting the stream.\n");
        return false;
    }

    this->frame = av_frame_alloc();
    this->frame->width = state->width;
    this->frame->height = state->height;
    this->frame->format = state->in_format;
    this->frame->pts = 0;
    if (av_frame_get_buffer(frame, 0) < 0){
        printf("[LOOPBACK] Couldn't allocate frame.\n");
        return false;
    }

	return true;
}

AVFrame* V4L2::Pull() {
    if (ioctl(this->dev_fd, VIDIOC_DQBUF, &this->info) < 0) {
        printf("[LOOPBACK] Error dequeuing stream.\n");
        return NULL;
    }

    if (av_image_fill_arrays(frame->data, frame->linesize, (uint8_t*)buffer,
                             (AVPixelFormat)frame->format, frame->width, frame->height, 1) < 0) {
        return NULL;
    }

    frame->pts += 1;

    if (ioctl(dev_fd, VIDIOC_QBUF, &info) < 0) {
        printf("[LOOPBACK] Error queueing stream.\n");
        return NULL;
    }

    return frame;
}

bool V4L2::SetSink() {
	if ((dev_fd = open(state->loopback, O_RDWR)) < 0) {
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

	format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	format.fmt.pix.width = state->width;
	format.fmt.pix.height = state->height;
	format.fmt.pix.sizeimage = frame_size;
    format.fmt.pix.field = V4L2_FIELD_NONE;

	if (ioctl(dev_fd, VIDIOC_S_FMT, &format) < 0) {
        printf("[LOOPBACK] Couldn't open interface.\n");
        return false;
	}

    if (!(buffer = (char*)malloc(frame_size))) {
        printf("[LOOPBACK] Couldn't allocate loopback buffer.\n");
        return false;
    }

	return true;
}

bool V4L2::Push(AVFrame* frame) {
    if (av_image_copy_to_buffer((uint8_t*)buffer, info.length, (const uint8_t* const*)frame->data,
                                frame->linesize, (AVPixelFormat)frame->format, frame->width, frame->height, 1) < 0) {
        return false;
    }

    if (write(this->dev_fd, this->buffer, this->info.length) < 0) {
        return false;
    }

    return true;
}

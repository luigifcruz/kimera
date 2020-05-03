#include "loopback.h"

bool open_loopback(LoopbackState* state, char* interface) {
	state->dev_fd = open(interface, O_RDWR);
	if (state->dev_fd < 0) {
        printf("[LINUX_LOOPBACK] Couldn't open interface.\n");
        return false;
	}

    state->format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	state->format.fmt.pix.width = WIDTH;
	state->format.fmt.pix.height = HEIGHT;
	state->format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
	state->format.fmt.pix.sizeimage = FRAME_BUFFER_SIZE_YUV420;
	if (ioctl(state->dev_fd, VIDIOC_S_FMT, &state->format) < 0){
        printf("[LINUX_LOOPBACK] Couldn't open interface.\n");
        return false;
	}
    
    state->req.count = 1;
    state->req.memory = V4L2_MEMORY_MMAP;
    state->req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(state->dev_fd, VIDIOC_REQBUFS, &state->req) < 0){
        printf("[LINUX_LOOPBACK] Couldn't request the buffer.\n");
        return false;
	}

    state->info.index = 0;
    state->info.memory = V4L2_MEMORY_MMAP;
    state->info.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(state->dev_fd, VIDIOC_QUERYBUF, &state->info) < 0){
        printf("[LINUX_LOOPBACK] Couldn't query the buffer.\n");
        return false;
	}

    state->buffer = mmap(
        NULL,
        state->info.length,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        state->dev_fd,
        state->info.m.offset);

    if (state->buffer == MAP_FAILED) {
        printf("[LINUX_LOOPBACK] Failed to allocate buffer.\n");
        return false;
    }

    memset(state->buffer, 0, state->info.length);

    if (ioctl(state->dev_fd, VIDIOC_STREAMON, &state->info.type) < 0) {
        printf("[LINUX_LOOPBACK] Error starting the stream.\n");
        return false;
    }

	return true;
}

void close_loopback(LoopbackState* state) {
    if (state->buffer)
        free(state->buffer);
    ioctl(state->dev_fd, VIDIOC_STREAMOFF, &state->info.type);
}

bool loopback_pull_frame(LoopbackState* state) {
    if (ioctl(state->dev_fd, VIDIOC_DQBUF, &state->info) < 0) {
        printf("[LINUX_LOOPBACK] Error dequeuing stream.\n");
        return false;
    }

    state->info.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    state->info.memory = V4L2_MEMORY_MMAP;

    if (ioctl(state->dev_fd, VIDIOC_QBUF, &state->info) < 0) {
        printf("[LINUX_LOOPBACK] Error queueing stream.\n");
        return false;
    }

    return true;
}
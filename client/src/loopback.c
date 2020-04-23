#include "loopback.h"

int open_loopback(char* interface) {
    struct v4l2_format v;

	int dev_fd = open(interface, O_RDWR);
	if (dev_fd == -1) {
        printf("Couldn't open interface.\n");
        return -1;
	}
	v.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	if (ioctl(dev_fd, VIDIOC_G_FMT, &v) == -1){
        printf("Couldn't open interface.\n");
        return -1;
	}
	v.fmt.pix.width = 1920;
	v.fmt.pix.height = 1080;
	v.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
	v.fmt.pix.sizeimage = 3110400;
	v.fmt.pix.field = V4L2_FIELD_NONE;
	if (ioctl(dev_fd, VIDIOC_S_FMT, &v) == -1){
        printf("Couldn't open interface.\n");
        return -1;
	}

	return dev_fd;
}

void loopback_push_frame(int fd, DecoderState* decoder, AVFrame* frame) {
/*
    size_t y_len = (frame->linesize[0] * frame->height);
    size_t u_len = (frame->linesize[1] * frame->height) / 2;
    size_t v_len = (frame->linesize[2] * frame->height) / 2;
    size_t len = y_len + u_len + v_len;

    char* buf = (char*)malloc(len);

    memcpy(buf, frame->data[0], y_len);
    memcpy(buf + y_len, frame->data[1], u_len);
    memcpy(buf + u_len + y_len, frame->data[2], v_len);

    write(fd, frame->data[0], len);
    printf("Y %ld U %ld V %ld == %ld %d %d\n", y_len, u_len, v_len, len, frame->format, AV_PIX_FMT_YUVJ420P);
*/
    char* buf = NULL;
    int len = -1;

    if((len = convert_frame(decoder, frame, buf)) > 0) {
        printf("A %d\n", len);
        write(fd, buf, len);
        free(buf);
    }
}
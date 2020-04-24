#include "resample.h"

int convert_frame(ResampleState* decoder, AVFrame* frame, char* buf) {
    if (decoder->sws_ctx == NULL) {
        decoder->sws_ctx = sws_getContext(frame->width, frame->height, frame->format,
                                          frame->width, frame->height, 0,
                                          SWS_FAST_BILINEAR, NULL, NULL, NULL);
    }
   size_t y_len = (frame->linesize[0] * frame->height);
    size_t u_len = (frame->linesize[1] * frame->height) / 2;
    size_t v_len = (frame->linesize[2] * frame->height) / 2;
    size_t len = ((frame->linesize[0] * frame->height)     +
                 (frame->linesize[1] * frame->height / 2)  +
                 (frame->linesize[2] * frame->height / 2));

    uint8_t* a = (uint8_t*)malloc(y_len);
    uint8_t* b = (uint8_t*)malloc(u_len);
    uint8_t* c = (uint8_t*)malloc(v_len);
    int dest_linesize[4] = { frame->linesize[0]+frame->linesize[1]+frame->linesize[2], 0,0,0 };
    uint8_t* dest[4] = { a, NULL, NULL, NULL };
    printf("1\n");
    sws_scale(decoder->sws_ctx, frame->data, frame->linesize, 0, frame->height, dest, dest_linesize);
    printf("2\n");
    return dest_linesize[0];
}
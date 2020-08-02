#ifndef RECEIVER_CONTEXT_H
#define RECEIVER_CONTEXT_H

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

typedef struct
{
  id receiver;
  int width;
  int height;

  pthread_mutex_t frame_lock;
  pthread_cond_t  frame_wait_cond;

  CMSampleBufferRef current_frame;
  AVCaptureVideoDataOutput *output;
  AVCaptureDevice* device;
  AVCaptureSession* session;
  AVCaptureScreenInput* displayInput;
  AVCaptureDeviceInput* cameraInput;
} ReceiverContext;

#endif
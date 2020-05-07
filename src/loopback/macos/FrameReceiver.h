#ifndef FRAME_RECEIVER_H
#define FRAME_RECEIVER_H

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

#import "ReceiverContext.h"

#include <pthread.h>

@interface FrameReceiver : NSObject
{
  ReceiverContext* _context;
}

- (id)initWithContext:(ReceiverContext*)context;

- (void)  captureOutput:(AVCaptureOutput *)captureOutput
  didOutputSampleBuffer:(CMSampleBufferRef)videoFrame
         fromConnection:(AVCaptureConnection *)connection;

@end

#endif
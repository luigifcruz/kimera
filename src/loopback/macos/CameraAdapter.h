#ifndef CAMERA_ADAPTER_H
#define CAMERA_ADAPTER_H

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

#import "ReceiverContext.h"
#import "FrameReceiver.h"

#include <pthread.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>

#include "config.h"

@interface CameraAdapter : NSObject
{
  ReceiverContext ctx;
}

- (bool) startCapture: (State*)state;
- (void) stopCapture;
- (bool) pullFrame: (AVFrame*)frame;

@end

#endif

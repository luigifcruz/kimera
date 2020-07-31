#ifndef CAMERA_ADAPTER_H
#define CAMERA_ADAPTER_H

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

#import "ReceiverContext.hpp"
#import "FrameReceiver.hpp"

#include <pthread.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>

#import "kimera/state.hpp"

@interface CameraAdapter : NSObject
{
  ReceiverContext ctx;
}

- (int) getFrameWidth;
- (int) getFrameHeight;
- (bool) startCapture: (Kimera::State*)state;
- (void) stopCapture;
- (bool) pullFrame: (AVFrame*)frame;

@end

#endif

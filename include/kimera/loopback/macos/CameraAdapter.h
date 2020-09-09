#ifndef CAMERA_ADAPTER_H
#define CAMERA_ADAPTER_H

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

#import "ReceiverContext.h"
#import "FrameReceiver.h"

#include <pthread.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>

@interface CameraAdapter : NSObject
{
  ReceiverContext ctx;
}

- (int) getFrameWidth;
- (int) getFrameHeight;
- (bool) startCapture:(bool)display d:(char*)device f:(int)fps w:(int)width h:(int)height;
- (void) stopCapture;
- (bool) pullFrame:(char*[])buffers linesize:(int[])linesize;

@end

#endif
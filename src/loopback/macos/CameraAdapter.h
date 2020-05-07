#ifndef CAMERA_ADAPTER_H
#define CAMERA_ADAPTER_H

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

#import "ReceiverContext.h"
#import "FrameReceiver.h"

#include <pthread.h>

@interface CameraAdapter : NSObject
{
  ReceiverContext ctx;
}

- (bool) startCapture;
- (void) stopCapture;

- (bool) YPlane: (void*)Y
         UPlane: (void*)U
         VPlane: (void*)V;

@end

#endif
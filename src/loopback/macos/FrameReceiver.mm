#import "kimera/loopback/macos/FrameReceiver.hpp"

@implementation FrameReceiver

- (id)initWithContext:(ReceiverContext*)context
{
    if (self = [super init]) {
        _context = context;
    }
    return self;
}

- (void)  captureOutput:(AVCaptureOutput *)captureOutput
  didOutputSampleBuffer:(CMSampleBufferRef)videoFrame
         fromConnection:(AVCaptureConnection *)connection
{
    pthread_mutex_lock(&_context->frame_lock);

    if (_context->current_frame != nil) {
        CFRelease(_context->current_frame);
    }

    _context->current_frame = (CMSampleBufferRef)CFRetain(videoFrame);

    pthread_cond_signal(&_context->frame_wait_cond);
    pthread_mutex_unlock(&_context->frame_lock);
}

@end

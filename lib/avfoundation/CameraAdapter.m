#import "CameraAdapter.h"

#import <AppKit/AppKit.h>

@implementation CameraAdapter

- (bool)startCapture
{
    NSError* error = nil;

    pthread_mutex_init(&ctx.frame_lock, NULL);
    pthread_cond_init(&ctx.frame_wait_cond, NULL);

    ctx.device = [AVCaptureDevice defaultDeviceWithMediaType: AVMediaTypeVideo];

    CGFloat desiredFPS = 25;
    int32_t maxWidth = 0;
    AVCaptureDeviceFormat *selectedFormat = nil;
    AVFrameRateRange *frameRateRange = nil;

    for (AVCaptureDeviceFormat *format in [ctx.device formats]) {
        for (AVFrameRateRange *range in format.videoSupportedFrameRateRanges) {
            CMFormatDescriptionRef desc = format.formatDescription;
            CMVideoDimensions dimensions = CMVideoFormatDescriptionGetDimensions(desc);
            int32_t width = dimensions.width;

            if (range.minFrameRate <= desiredFPS && desiredFPS <= range.maxFrameRate && width >= maxWidth) {
                selectedFormat = format;
                frameRateRange = range;
                maxWidth = width;
            }
        }
    }

    if ([ctx.device lockForConfiguration:nil]) {
        NSLog(@"selected format:%@", selectedFormat);
        ctx.device.activeFormat = selectedFormat;
        ctx.device.activeVideoMinFrameDuration = CMTimeMake(1, (int32_t)desiredFPS);
        ctx.device.activeVideoMaxFrameDuration = CMTimeMake(1, (int32_t)desiredFPS);
        [ctx.device unlockForConfiguration];
    }
    

    ctx.input = [AVCaptureDeviceInput deviceInputWithDevice: ctx.device error: &error];

    if (error) {
        NSLog(@"[LOOPBACK] Error opening the device.");
        return false;
    }

    ctx.output = [[AVCaptureVideoDataOutput alloc] init];

    NSNumber* pixel_format = [NSNumber numberWithInt:kCVPixelFormatType_420YpCbCr8Planar];
    NSDictionary* capture_dict = [NSDictionary dictionaryWithObject:pixel_format forKey:(id)kCVPixelBufferPixelFormatTypeKey];
    [ctx.output setVideoSettings:capture_dict];

    ctx.receiver = [[FrameReceiver alloc] initWithContext: &ctx];
    dispatch_queue_t queue = dispatch_queue_create("video_queue", NULL);
    [ctx.output setSampleBufferDelegate: ctx.receiver queue: queue];
    dispatch_release(queue);

    ctx.session = [[AVCaptureSession alloc] init];
    [ctx.session addInput: ctx.input];
    [ctx.session addOutput: ctx.output];
    [ctx.session startRunning];

    return true;
}

- (void)stopCapture
{
    pthread_mutex_destroy(&ctx.frame_lock);
    pthread_cond_destroy(&ctx.frame_wait_cond);
}

- (bool) YPlane: (void*)Y
         UPlane: (void*)U
         VPlane: (void*)V
{
    pthread_mutex_lock(&ctx.frame_lock);
    pthread_cond_wait(&ctx.frame_wait_cond, &ctx.frame_lock);

    if (ctx.current_frame != nil) {
        CVImageBufferRef image_buffer = CMSampleBufferGetImageBuffer(ctx.current_frame);

        int status = CVPixelBufferLockBaseAddress(image_buffer, 0);
        if (status != kCVReturnSuccess) {
            NSLog(@"[LOOPBACK] Could not lock base address.");
            return false;
        }
        
        if (CVPixelBufferIsPlanar(image_buffer)) {
            size_t bufferHeight = CVPixelBufferGetHeight(image_buffer);
            
            size_t bytesPerRow = CVPixelBufferGetBytesPerRowOfPlane(image_buffer, 0);
            char* ptr = CVPixelBufferGetBaseAddressOfPlane(image_buffer, 0);
            memcpy(Y, ptr, (bufferHeight*bytesPerRow));

            bytesPerRow = CVPixelBufferGetBytesPerRowOfPlane(image_buffer, 1);
            ptr = CVPixelBufferGetBaseAddressOfPlane(image_buffer, 1);
            memcpy(U, ptr, (bufferHeight*bytesPerRow)/2);

            bytesPerRow = CVPixelBufferGetBytesPerRowOfPlane(image_buffer, 2);
            ptr = CVPixelBufferGetBaseAddressOfPlane(image_buffer, 2);
            memcpy(V, ptr, (bufferHeight*bytesPerRow)/2);
        } else {
            NSLog(@"[LOOPBACK] Data buffer not planar.");
            return false;
        }
        
        CVPixelBufferUnlockBaseAddress(image_buffer, 0);
    }
    
    pthread_mutex_unlock(&ctx.frame_lock);
    return true;
}

@end

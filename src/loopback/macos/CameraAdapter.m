#import "CameraAdapter.h"

#import <AppKit/AppKit.h>

@implementation CameraAdapter

- (int)atoi: (char*)str
{
    if (!str)
        NSLog(@"Enter valid string");
    
	int result = 0;
    char *p = str;
	while ((*p>='0') && (*p<='9')) {
        result = result*10 + (*p - '0');
        p++;
    }
	return result;
}

- (bool)initDisplay: (State*)state
{
    uint32_t num_screens = 0;
    CGGetActiveDisplayList(0, NULL, &num_screens);

    NSUInteger selectedDevice = (NSUInteger)atoi(state->loopback);
    if (selectedDevice >= num_screens) {
        NSLog(@"[LOOPBACK] Selected screen (%lu) doesn't exist!\n", selectedDevice);
        return false;
    }

    ctx.displayInput = [[AVCaptureScreenInput alloc] initWithDisplayID: selectedDevice];
    ctx.displayInput.minFrameDuration = CMTimeMake(1, state->framerate);
    ctx.displayInput.capturesCursor = YES;
    ctx.displayInput.capturesMouseClicks = NO;

    return true;
}

- (bool)initCamera: (State*)state
{
    NSError* error = nil;
    NSArray * devices = [ AVCaptureDevice devicesWithMediaType: AVMediaTypeVideo ];
    NSUInteger selectedDevice = (NSUInteger)atoi(state->loopback);

    if (selectedDevice >= [devices count]) {
        NSLog(@"[LOOPBACK] Selected device (%lu) isn't available!\n", selectedDevice);
        NSLog(@"Available Devices:");

        NSUInteger count = 0;
        for (AVCaptureDevice * device in devices) {
            NSLog(@"    %lu: %@", count, [device localizedName]);
            count++;
        }
        return false;
    }
    ctx.device = devices[selectedDevice];

    AVCaptureDeviceFormat *selectedFormat = nil;
    AVFrameRateRange *frameRateRange = nil;

    for (AVCaptureDeviceFormat *format in [ctx.device formats]) {
        for (AVFrameRateRange *range in format.videoSupportedFrameRateRanges) {
            CMFormatDescriptionRef desc = format.formatDescription;
            CMVideoDimensions dimensions = CMVideoFormatDescriptionGetDimensions(desc);

            if (
                range.minFrameRate <= state->framerate  && 
                state->framerate <= range.maxFrameRate  && 
                dimensions.width == state->width        &&
                dimensions.height == state->height
            ) {
                selectedFormat = format;
                frameRateRange = range;
            }
        }
    }

    if (!selectedFormat || !frameRateRange) {
        NSLog(@"[LOOPBACK] Device doesn't support requested settings.");
        return false;
    }

    if ([ctx.device lockForConfiguration:nil]) {
        ctx.device.activeFormat = selectedFormat;
        ctx.device.activeVideoMinFrameDuration = CMTimeMake(1, frameRateRange.minFrameRate);
        ctx.device.activeVideoMaxFrameDuration = CMTimeMake(1, frameRateRange.maxFrameRate);
        [ctx.device unlockForConfiguration];
    }
    
    ctx.cameraInput = [AVCaptureDeviceInput deviceInputWithDevice: ctx.device error: &error];

    if (error) {
        NSLog(@"[LOOPBACK] Error opening device.");
        return false;
    }

    return true;
}

- (bool)startCapture: (State*)state
{
    pthread_mutex_init(&ctx.frame_lock, NULL);
    pthread_cond_init(&ctx.frame_wait_cond, NULL);

    ctx.output = [[AVCaptureVideoDataOutput alloc] init];

    NSNumber* pixel_format = [NSNumber numberWithInt:kCVPixelFormatType_420YpCbCr8Planar];
    NSDictionary* capture_dict = [NSDictionary dictionaryWithObject:pixel_format forKey:(id)kCVPixelBufferPixelFormatTypeKey];
    [ctx.output setVideoSettings:capture_dict];

    ctx.receiver = [[FrameReceiver alloc] initWithContext: &ctx];
    dispatch_queue_t queue = dispatch_queue_create("video_queue", NULL);
    [ctx.output setSampleBufferDelegate: ctx.receiver queue: queue];
    dispatch_release(queue);

    ctx.session = [[AVCaptureSession alloc] init];

    switch (state->source) {
        case DISPLAY:
            if ([self initDisplay:state]) {
                [ctx.session addInput: ctx.displayInput];
            } else {
                return false;
            }
            break;
        case LOOPBACK:
            if ([self initCamera:state]) {
                [ctx.session addInput: ctx.cameraInput];
            } else {
                return false;
            }
            break;
        default:
            NSLog(@"Source not an input device.\n");
            return false;
    }
    
    [ctx.session addOutput: ctx.output];
    [ctx.session startRunning];

    return true;
}

- (void)stopCapture
{
    pthread_mutex_destroy(&ctx.frame_lock);
    pthread_cond_destroy(&ctx.frame_wait_cond);
}

- (bool) pullFrame: (AVFrame*)frame
{
    pthread_mutex_lock(&ctx.frame_lock);
    pthread_cond_wait(&ctx.frame_wait_cond, &ctx.frame_lock);

    if (ctx.current_frame != nil) {
        CVImageBufferRef image_buffer = CMSampleBufferGetImageBuffer(ctx.current_frame);

        if (CVPixelBufferLockBaseAddress(image_buffer, 0) != kCVReturnSuccess) {
            NSLog(@"[LOOPBACK] Could not lock base address.");
            return false;
        }

        size_t plane_count = CVPixelBufferGetPlaneCount(image_buffer);
        for (size_t i = 0; i < plane_count; i++) {
            memcpy(
                frame->data[i],
                CVPixelBufferGetBaseAddressOfPlane(image_buffer, i),
                frame->linesize[i] * CVPixelBufferGetHeightOfPlane(image_buffer, i));
        }

        CVPixelBufferUnlockBaseAddress(image_buffer, 0);
    }
    
    pthread_mutex_unlock(&ctx.frame_lock);
    return true;
}

@end

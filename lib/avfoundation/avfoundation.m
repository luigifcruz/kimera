#import "avfoundation.h"
#import "CameraAdapter.h"

bool avfoundation_open_source(avfoundation_state* state) {
    state->adapter = [CameraAdapter new];
    return [(id)state->adapter startCapture];
}

void avfoundation_close_source(avfoundation_state* state) {
    [(id)state->adapter stopCapture];
}

bool avfoundation_pull_frame(avfoundation_state* state, void* Y, void* U, void* V) {
    return [(id)state->adapter YPlane:Y UPlane:U VPlane:V ];
}
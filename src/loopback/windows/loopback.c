#include "kimera/loopback/windows.h"

bool open_loopback_sink(LoopbackState* loopback, State* state) {
	printf("[LOOPBACK] Not supported on Windows yet.\n");
	return false;
}

bool loopback_push_frame(LoopbackState* loopback, AVFrame* frame) {
	return false;
}

bool open_loopback_source(LoopbackState* loopback, State* state) {
	printf("[LOOPBACK] Not supported on Windows yet.\n");
	return false;
}

bool loopback_pull_frame(LoopbackState* loopback, State* state) {
	return false;
}

void close_loopback(LoopbackState* loopback, State* state) {
	return;
}
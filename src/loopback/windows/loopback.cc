#include "kimera/loopback/windows.hpp"

namespace Kimera {

Loopback::Loopback(State& state) : state(state) {}
Loopback::~Loopback() {}

bool Loopback::LoadSink() {
	printf("[LOOPBACK] Source device supported on Windows yet.\n");
	return false;
}

bool Loopback::LoadSource() {
	printf("[LOOPBACK] Source device supported on Windows yet.\n");
	return false;
}

bool Loopback::Push(AVFrame* frame) {
	return false;
}

AVFrame* Loopback::Pull() {
	return nullptr;
}

} // namespace Kimera
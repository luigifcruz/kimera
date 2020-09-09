#ifndef VK_TOOLS_BACKEND_H
#define VK_TOOLS_BACKEND_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
}

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "kimera/render/opengl/device.hpp"
#include "kimera/render/shaders.hpp"
#include "kimera/render/backend.hpp"
#include "kimera/state.hpp"

#include <fstream>
#include <iostream>
#include <vector>

namespace Kimera::Vulkan {

#define VK_CHECK_RESULT(f) {                                                  \
	VkResult res = (f);                                                       \
	if (res != VK_SUCCESS) {                                                  \
		std::cout << "Fatal : VkResult is \"" << magic_enum::enum_name(res) \
                  << "\" in " << __FILE__ << " at line " << __LINE__ << "\n"; \
		assert(res == VK_SUCCESS);                                            \
	}                                                                         \
}

class Tools {
public:
    static VkShaderModule LoadShaderFromFile(const char*, VkDevice);
    static VkShaderModule LoadShader(const char*, size_t, VkDevice);
};

} // namespace Kimera

#endif


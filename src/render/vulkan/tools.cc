#include "kimera/render/vulkan/tools.hpp"

namespace Kimera::Vulkan {

VkShaderModule Tools::LoadShaderFromFile(const char* filename, VkDevice device) {
    std::ifstream is(filename, std::ios::binary | std::ios::in | std::ios::ate);

    if (is.is_open()) {
        size_t size = is.tellg();
        is.seekg(0, std::ios::beg);
        char* shaderCode = new char[size];
        is.read(shaderCode, size);
        is.close();

        assert(size > 0);

        return Vulkan::Tools::LoadShader(shaderCode, size, device);

        delete [] shaderCode;
    } else {
        std::cerr << "[VULKAN] Can't open shader file \"" << filename << "\"." << std::endl;
    }

    return VK_NULL_HANDLE;
}

VkShaderModule Tools::LoadShader(const char* code, size_t size, VkDevice device) {
    VkShaderModule shaderModule;
    VkShaderModuleCreateInfo moduleCreateInfo{};
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.codeSize = size;
    moduleCreateInfo.pCode = (uint32_t*)code;

    VK_CHECK_RESULT(vkCreateShaderModule(device, &moduleCreateInfo, nullptr, &shaderModule));

    return shaderModule;
}

} // namespace Kimera::Vulkan

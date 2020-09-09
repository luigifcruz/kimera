#ifndef VK_BACKEND_H
#define VK_BACKEND_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
}

#include "kimera/render/vulkan/tools.hpp"
#include "kimera/render/shaders.hpp"
#include "kimera/render/backend.hpp"
#include "kimera/state.hpp"

#include <iostream>
#include <vector>

namespace Kimera::Vulkan {

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicFamily;
    std::optional<uint32_t> computeFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete(bool headlessEnabled) {
        if (headlessEnabled)
            return computeFamily.has_value() &&
                   graphicFamily.has_value();
        else
            return graphicFamily.has_value() &&
                   computeFamily.has_value() &&
                   presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Render : public Backend {
public:
    Render(State&);
    ~Render();

    void PrintMeta();

    std::vector<PixelFormat> GetInputFormats();
    std::vector<PixelFormat> GetOutputFormats();

    bool LoadInput(PixelFormat);
    bool LoadDisplay();
    bool LoadFilter();
    bool LoadOutput(PixelFormat);
    bool CommitPipeline();

    bool Push(AVFrame*);
    bool Draw();
    bool Filter();
    bool Pull(AVFrame*);

private:
    VkPhysicalDeviceProperties properties;
    VkPhysicalDevice physicalDevice;
    VkInstance instance;
    VkDevice device;
    VkSurfaceKHR surface;

    VkDebugReportCallbackEXT debugReportCallback{};

    bool validationEnabled = true;
    bool headlessEnabled = true;

    std::vector<PixelFormat> InputFormats = {
        AV_PIX_FMT_BGRA
    };

    std::vector<PixelFormat> OutputFormats = {
        AV_PIX_FMT_BGRA
    };

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation",
    };

    const std::vector<const char*> deviceExtensions = {};

    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();
    bool isDeviceSuitable(VkPhysicalDevice);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice);
    bool checkDeviceExtensionSupport(VkPhysicalDevice);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice);
};

} // namespace Kimera

#endif


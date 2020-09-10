#ifndef VK_BACKEND_H
#define VK_BACKEND_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>
}

#include "kimera/render/vulkan/tools.hpp"
#include "kimera/render/shaders.hpp"
#include "kimera/render/backend.hpp"
#include "kimera/state.hpp"

#include <iostream>
#include <vector>

namespace Kimera::Vulkan {

const int MAX_FRAMES_IN_FLIGHT = 2;

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
    GLFWwindow* window;
    VkSwapchainKHR swapChain;

    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageView;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    std::vector<VkCommandBuffer> commandBuffers;

    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;
    VkPipeline graphicsPipeline;
    VkCommandPool commandPool;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;

    VkQueue graphicQueue;
    VkQueue presentQueue;
    VkQueue computeQueue;

    VkDebugReportCallbackEXT debugReportCallback{};

    bool validationEnabled = true;
    bool headlessEnabled = false;

    std::vector<PixelFormat> InputFormats = {
        AV_PIX_FMT_BGRA
    };

    std::vector<PixelFormat> OutputFormats = {
        AV_PIX_FMT_BGRA
    };

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation",
    };

    std::vector<const char*> deviceExtensions = {};

    void CreateWindow();
    void CreateInstance();
    void CreateSurface();
    void CreateDevice();
    void CreateQueues();
    void CreateSwapChain();
    void CreateImageView();
    void CreateRenderPass();
    void CreateGraphicsPipeline();
    void CreateFrameBuffers();
    void CreateCommandPool();
    void CreateCommandBuffers();
    void CreateSyncObjects();
    void DrawFrame();

    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();
    bool isDeviceSuitable(VkPhysicalDevice);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice);
    bool checkDeviceExtensionSupport(VkPhysicalDevice);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice);

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>&);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR&);
};

} // namespace Kimera

#endif


#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <stdint.h>
#include "camera/camera.h"
#include "engines/renderEngine.h"
#include <thread>

namespace Vermilion
{
    struct RendererQueueFamilies
    {
        VkQueue graphicsQueue;
        VkQueue computeQueue;
        VkQueue presentQueue;
        
        uint32_t graphicsIndex;
        uint32_t computeIndex;
        uint32_t presentIndex;
    };
    
    struct SwapChainSupportDetails {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
    };

    class VkRenderer
    {
        VkInstance m_inst;
        VkPhysicalDevice m_phy;
        VkPhysicalDeviceProperties m_deviceProps;
        VkPhysicalDeviceFeatures m_deviceFeatures;
        VkPhysicalDeviceMemoryProperties m_deviceMemProps;

        VkDevice m_dev;

        RendererQueueFamilies m_queues{};

        SwapChainSupportDetails m_chainSupport;

        VkFormat m_depthFormat;
        VkCommandPool m_cmdPool;
        VkPipelineStageFlags m_submitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo m_sinfo;

        std::vector<VkCommandBuffer> m_drawCommands;

        VkRenderPass m_rpass;

        std::vector<VkShaderModule> m_shaderModules;
        bool m_bHasCompute = false;

        VkPipelineCache m_pcache;
        VkSurfaceKHR m_surface;
        VkSwapchainKHR m_chain;
        
        // Images
        std::vector<VkImage> m_fbImages;
        uint32_t m_bestFormatIndex;
        uint32_t m_bestPresIndex;

        struct 
        {
            VkSemaphore presentComplete;
            VkSemaphore renderComplete;
            VkSemaphore overlayComplete;
        } m_semaphores;

        std::vector<VkFence> m_waitFences;

        GLFWwindow* m_window;
        class RenderEngine *m_engine;
        std::thread m_uiThread;

        private:
            void Shutdown(); // Signalled from GUI
            void UIThread_RT();

        public:
            VkRenderer(Camera *defaultCamera, class RenderEngine *parent);
            void Render();
            ~VkRenderer();
    };
};
#include "renderer.h"

#include <iostream>
#include <cstdint>
#include <set>
#include "tools.h"

const std::vector<const char*> validationLayers = {
    "VK_LAYER_LUNARG_standard_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

void InitInstance(VkInstance &i)
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vermilion Render Window";
    appInfo.applicationVersion = VK_MAKE_VERSION(0,0,1);
    appInfo.pEngineName = "Vermilion Renderer";
    appInfo.engineVersion = VK_MAKE_VERSION(2, 0, 0);
    //appInfo.apiVersion = VK_API_VERSION_1_1;
    appInfo.apiVersion = VK_API_VERSION_1_0;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
    
    if (vkCreateInstance(&createInfo, nullptr, &i) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create instance!");
    }
}

void InitPhysicalDevice(
    VkInstance i,
    VkPhysicalDevice &d,
    VkPhysicalDeviceProperties &devProps,
    VkPhysicalDeviceFeatures &devFeats,
    VkPhysicalDeviceMemoryProperties &devMemProps,
    Vermilion::RendererQueueFamilies &qs,
    VkSurfaceKHR s
    )
{
    uint32_t nGpus = 0;
    auto res = vkEnumeratePhysicalDevices(i, &nGpus, nullptr);
    if(res != VK_SUCCESS) throw std::runtime_error("Unable to Enumerate Devices: Stage 1");

    std::vector<VkPhysicalDevice> phyDevs(nGpus);
    res = vkEnumeratePhysicalDevices(i, &nGpus, phyDevs.data());
    if(res != VK_SUCCESS) throw std::runtime_error("Unable to Enumerate Devices: Stage 2");

    if(nGpus == 0) throw std::runtime_error("No Rendering Device Found");

    // Select Best GPU
    uint32_t selectedGPU = 0;
    uint32_t cause = 0;

    for (uint32_t i = 0; i < phyDevs.size(); i++) 
    {
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(phyDevs[i], &deviceProperties);
		std::cout << "Device [" << i << "] : " << deviceProperties.deviceName << std::endl;
        std::cout << " TypeN: " << deviceProperties.deviceType << std::endl;
		std::cout << " Type: " << physicalDeviceTypeString(deviceProperties.deviceType) << std::endl;
		std::cout << " API: " << (deviceProperties.apiVersion >> 22) << "." << ((deviceProperties.apiVersion >> 12) & 0x3ff) << "." << (deviceProperties.apiVersion & 0xfff) << std::endl;

        switch(deviceProperties.deviceType)
        {
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            if(cause <= VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            {
                selectedGPU = i;
                cause = VK_PHYSICAL_DEVICE_TYPE_CPU;
            }
            break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            if(cause <= VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU
             || cause == VK_PHYSICAL_DEVICE_TYPE_CPU)
            {
                selectedGPU = i;
                cause = VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;
            }
            break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            selectedGPU = i;
            cause = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
            break;
        default:
            if(cause <= deviceProperties.deviceType)
            {
                selectedGPU = i;
                cause = deviceProperties.deviceType;
            }
            break;
        }
	}

    d = phyDevs[selectedGPU];
    std::cout << "Selected GPU: " << selectedGPU << std::endl;

    vkGetPhysicalDeviceProperties(d, &devProps);
    vkGetPhysicalDeviceFeatures(d, &devFeats);
    vkGetPhysicalDeviceMemoryProperties(d, &devMemProps);

    // Get the Queue
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(d, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(d, &queueFamilyCount, queueFamilies.data());

    qs.presentIndex = UINT32_MAX;
    qs.computeIndex = UINT32_MAX;
    qs.graphicsIndex = UINT32_MAX;

    for(int i = 0; i < queueFamilies.size(); ++i)
    {
        auto family = queueFamilies[i];
        if(family.queueCount > 0)
        {
            if(family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                qs.graphicsIndex = i;
            }
            if(family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                qs.computeIndex = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(d, i, s, &presentSupport);

            if(presentSupport)
            {
                qs.presentIndex = i;
            }

            if
            (
                qs.graphicsIndex != UINT32_MAX
                 &&
                qs.presentIndex != UINT32_MAX
                 &&
                qs.computeIndex != UINT32_MAX
            )
            {
                break;
            }

        }
    }
}

void InitVirtualDevice(
    VkInstance i,
    VkPhysicalDevice d,
    Vermilion::RendererQueueFamilies rqf,
    VkDevice &dev
    )
{
    float prios[1] = {1.f};


    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = 
    {
        rqf.presentIndex,
        rqf.computeIndex,
        rqf.graphicsIndex
    };

    for(uint32_t family : uniqueQueueFamilies)
    {   
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = family;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = prios;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.tessellationShader = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    auto res = vkCreateDevice(d, &createInfo, nullptr, &dev);
    if(res != VK_SUCCESS) throw std::runtime_error("Error Creating Logical Device");
}

VkBool32 GetSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat &depthFormat)
{
	// Since all depth formats may be optional, we need to find a suitable depth format to use
	// Start with the highest precision packed format
	std::vector<VkFormat> depthFormats = 
    {
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};

	for (auto& format : depthFormats)
	{
		VkFormatProperties formatProps;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
		// Format must support depth stencil attachment for optimal tiling
		if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			depthFormat = format;
			return true;
		}
    }
	return false;
}

void GetQueueHandles
(
    VkDevice d,
    Vermilion::RendererQueueFamilies &rqf
)
{
    vkGetDeviceQueue(d, rqf.computeIndex, 0, &rqf.computeQueue);
    vkGetDeviceQueue(d, rqf.graphicsIndex, 0, &rqf.graphicsQueue);
    vkGetDeviceQueue(d, rqf.presentIndex, 0, &rqf.presentQueue);
}

void SemaphoreSetup
(
    VkDevice d,
    VkSemaphore &s1
)
{
    VkSemaphoreCreateInfo semaphoreCreateInfo {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    auto res = vkCreateSemaphore(d, &semaphoreCreateInfo, nullptr, &s1);
    if(res != VK_SUCCESS) throw std::runtime_error("Semaphore creation failed");
}

void SubmissionInfo
(
    VkSubmitInfo &sinfo,
    VkPipelineStageFlags &psf,
    VkSemaphore &s1,
    VkSemaphore &s2
)
{
    sinfo = {};
    sinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    sinfo.pWaitDstStageMask = &psf;
    sinfo.waitSemaphoreCount = 1;
    sinfo.pWaitSemaphores = &s1;
    sinfo.signalSemaphoreCount = 1;
    sinfo.pSignalSemaphores = &s2;
}

void Surface
(
    VkInstance i,
    GLFWwindow *w,
    VkSurfaceKHR &s
)
{
    auto res = glfwCreateWindowSurface(i, w, nullptr, &s);
    if(res != VK_SUCCESS) throw std::runtime_error("Error creating surface");
}

void GetChainSupports
(
    Vermilion::SwapChainSupportDetails &cs,
    VkPhysicalDevice& d,
    VkSurfaceKHR& s
)
{
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(d, s, &cs.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(d, s, &formatCount, nullptr);
    if(formatCount == 0) throw std::runtime_error("No Swapchain Formats");
    cs.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(d, s, &formatCount, cs.formats.data());

    vkGetPhysicalDeviceSurfacePresentModesKHR(d, s, &formatCount, nullptr);
    if(formatCount == 0) throw std::runtime_error("No Swapchain Formats");
    cs.presentModes.resize(formatCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(d, s, &formatCount, cs.presentModes.data());
}

VkSurfaceFormatKHR GetSwapSurfaceFormat
(
	const std::vector<VkSurfaceFormatKHR>& formatsVector
)
{
	if(formatsVector.size() == 1 && formatsVector[0].format == VK_FORMAT_UNDEFINED)
	{
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for(auto& format : formatsVector)
	{
		if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return format;
		}
	}

	return formatsVector[0];
}

VkPresentModeKHR GetSwapSurfacePresentMode
(
	const std::vector<VkPresentModeKHR>& presentsVector
)
{
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (const auto& availablePresentMode : presentsVector) {

		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
		{
			return availablePresentMode;
		}

		if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			bestMode = availablePresentMode;
		}
	}

	return bestMode;
}

VkExtent2D GetSwapSurfaceExtent
(
	const VkSurfaceCapabilitiesKHR& caps
)
{
	if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return caps.currentExtent;
	}
	
	VkExtent2D actualExtent = { 1920, 1080 };

	actualExtent.width = std::max(caps.minImageExtent.width, std::min(caps.maxImageExtent.width, actualExtent.width));
	actualExtent.height = std::max(caps.minImageExtent.height, std::min(caps.maxImageExtent.height, actualExtent.height));

	return actualExtent;
}

void CreateSwapChain
(
	Vermilion::SwapChainSupportDetails& cs,
	VkSurfaceKHR &surface,
	Vermilion::RendererQueueFamilies &rqf,
	VkDevice &d,
	VkSwapchainKHR& swapchain,
	std::vector<VkImage> &images,
	VkFormat &chainFormat,
	VkExtent2D &chainExtent
)
{
	VkSurfaceFormatKHR surfaceFormat = GetSwapSurfaceFormat(cs.formats);
	VkPresentModeKHR presentMode = GetSwapSurfacePresentMode(cs.presentModes);
	VkExtent2D extent = GetSwapSurfaceExtent(cs.capabilities);

	uint32_t imageCount = cs.capabilities.minImageCount + 1;

	if (cs.capabilities.maxImageCount > 0 && imageCount > cs.capabilities.maxImageCount) {
		imageCount = cs.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queueFamilyIndices[] = { rqf.graphicsIndex,  rqf.presentIndex };

	if (rqf.graphicsIndex != rqf.presentIndex) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = cs.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(d, &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	// While we are here, lets set up the chain image references
	vkGetSwapchainImagesKHR(d, swapchain, &imageCount, nullptr);
	images.resize(imageCount);
	vkGetSwapchainImagesKHR(d, swapchain, &imageCount, images.data());

	chainFormat = surfaceFormat.format;
	chainExtent = extent;
}

void CreateImageViews
(
	std::vector<VkImage>& images,
	std::vector<VkImageView>& imageViews,
	VkFormat &chainFormat,
	VkDevice &d
)
{
	imageViews.resize(images.size());

	for(uint64_t i = 0; i < images.size(); ++i)
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = chainFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(d, &createInfo, nullptr, &imageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image views!");
		}
	}
}

void CreateGraphicsPipeline
(

)
{
	
}

Vermilion::VkRenderer::VkRenderer(Camera *cam, RenderEngine *parent)
{
    m_engine = parent;
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // This should come from the renderEngine camera
    m_window = glfwCreateWindow(
        cam->uImageU, cam->uImageV, "Vermilion", nullptr,nullptr);

    // Maintain the UI
    //m_uiThread = std::thread(&Vermilion::VkRenderer::UIThread_RT, this);

    // Full Init of Vulkan here
    InitInstance(m_inst);

    Surface(m_inst, m_window, m_surface);

    InitPhysicalDevice(m_inst, m_phy, m_deviceProps, m_deviceFeatures, m_deviceMemProps, m_queues, m_surface);

    InitVirtualDevice(m_inst, m_phy, m_queues, m_dev);

    GetQueueHandles(m_dev, m_queues);

    if(!GetSupportedDepthFormat(m_phy, m_depthFormat)) throw std::runtime_error("No Suitable Depth Format");

    SemaphoreSetup(m_dev, m_semaphores.renderComplete);
    SemaphoreSetup(m_dev, m_semaphores.presentComplete);

    SubmissionInfo(m_sinfo, m_submitStages, m_semaphores.presentComplete, m_semaphores.renderComplete);

    GetChainSupports(m_chainSupport, m_phy, m_surface);

	CreateSwapChain(m_chainSupport, m_surface, m_queues, m_dev, m_chain, m_swapPlanes, m_chainFormat, m_chainExtent);

	CreateImageViews(m_swapPlanes, m_swapPlaneViews, m_chainFormat, m_dev);

	CreateGraphicsPipeline();
}



Vermilion::VkRenderer::~VkRenderer()
{
    vkDestroyDevice(m_dev, nullptr);
    vkDestroySurfaceKHR(m_inst, m_surface, nullptr);
    vkDestroyInstance(m_inst, nullptr);
	vkDestroySwapchainKHR(m_dev, m_chain, nullptr);
    glfwDestroyWindow(m_window);
    glfwTerminate();
	for (auto imageView : m_swapPlaneViews) {
		vkDestroyImageView(m_dev, imageView, nullptr);
	}
    m_uiThread.join();
}

void Vermilion::VkRenderer::Shutdown()
{
    // Clean up our internals, they don't need to exist
    // Go for shutdown
    m_engine->shutdownVulkanPreview();
}

void Vermilion::VkRenderer::Render()
{
    // Do nothing! haha
    UIThread_RT();
}

void Vermilion::VkRenderer::UIThread_RT()
{
    while(!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
    }
    
    // Signal that we need to be destroyed then zombie this runtime
    Shutdown();
}
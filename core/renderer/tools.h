#pragma once
#include <string>
#include <vulkan/vulkan.h>

std::string physicalDeviceTypeString(VkPhysicalDeviceType type)
{
    switch (type)
	{
#define STR(r) case VK_PHYSICAL_DEVICE_TYPE_ ##r: return #r
				STR(OTHER);
				STR(INTEGRATED_GPU);
				STR(DISCRETE_GPU);
				STR(VIRTUAL_GPU);
#undef STR
			default: return "UNKNOWN_DEVICE_TYPE";
	}
}
#pragma once

#include <array>
#include <vulkan/vulkan.h>

//#define RECORD_CPU_TIMES
//#define RECORD_GPU_TIMES
//#define ALERT_FINAL_AVERAGE

class GfxSettings
{
public:
	static const std::array<VkValidationFeatureEnableEXT, 1> validationFeatureEnables;

	static const uint32_t FRAMES_IN_FLIGHT = 3;
	static const uint32_t SWAPCHAIN_IMAGES = 3;
};
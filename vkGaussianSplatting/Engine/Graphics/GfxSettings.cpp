#include "pch.h"
#include "GfxSettings.h"

const std::array<VkValidationFeatureEnableEXT, 1> GfxSettings::validationFeatureEnables =
{
	//VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
	VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT,
};
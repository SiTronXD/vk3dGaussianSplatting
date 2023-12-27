#pragma once

#include <vulkan/vulkan.h>

class PipelineBarrier
{
public:
	static VkImageMemoryBarrier2 imageMemoryBarrier2(
		VkAccessFlags2 srcAccessMask,
		VkAccessFlags2 dstAccessMask,
		VkPipelineStageFlags2 srcStageMask,
		VkPipelineStageFlags2 dstStageMask,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		VkImage image,
		VkImageAspectFlags imageAspectFlags)
	{
		// Subresource range
		VkImageSubresourceRange subresourceRange{};
		subresourceRange.aspectMask = imageAspectFlags;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 1;

		// Image memory barrier 2
		return PipelineBarrier::imageMemoryBarrier2(
			srcAccessMask,
			dstAccessMask,
			srcStageMask,
			dstStageMask,
			oldLayout,
			newLayout,
			image,
			subresourceRange
		);
	}

	static VkImageMemoryBarrier2 imageMemoryBarrier2(
		VkAccessFlags2 srcAccessMask,
		VkAccessFlags2 dstAccessMask,
		VkPipelineStageFlags2 srcStageMask,
		VkPipelineStageFlags2 dstStageMask,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		VkImage image,
		const VkImageSubresourceRange& subresourceRange)
	{
		// Image memory barrier 2
		VkImageMemoryBarrier2 imageMemoryBarrierInfo{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
		imageMemoryBarrierInfo.srcStageMask = srcStageMask;
		imageMemoryBarrierInfo.dstStageMask = dstStageMask;
		imageMemoryBarrierInfo.srcAccessMask = srcAccessMask;
		imageMemoryBarrierInfo.dstAccessMask = dstAccessMask;
		imageMemoryBarrierInfo.oldLayout = oldLayout;
		imageMemoryBarrierInfo.newLayout = newLayout;
		imageMemoryBarrierInfo.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrierInfo.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrierInfo.image = image;
		imageMemoryBarrierInfo.subresourceRange = subresourceRange;

		return imageMemoryBarrierInfo;
	}
};
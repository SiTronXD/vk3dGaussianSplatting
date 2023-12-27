#pragma once

#include <vulkan/vulkan.h>

class DescriptorSet
{
public:
	static VkWriteDescriptorSet writeBuffer(
		uint32_t dstBinding,
		VkDescriptorType descriptorType,
		VkDescriptorBufferInfo* bufferInfo)
	{
		VkWriteDescriptorSet writeDescriptorSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		writeDescriptorSet.dstBinding = dstBinding;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = descriptorType;
		writeDescriptorSet.pBufferInfo = bufferInfo;

		return writeDescriptorSet;
	}

	static VkWriteDescriptorSet writeImage(
		uint32_t dstBinding,
		VkDescriptorType descriptorType,
		VkDescriptorImageInfo* imageInfo)
	{
		VkWriteDescriptorSet writeDescriptorSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		writeDescriptorSet.dstBinding = dstBinding;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = descriptorType;
		writeDescriptorSet.pImageInfo = imageInfo;

		return writeDescriptorSet;
	}
};
#include "pch.h"
#include "DescriptorPool.h"

DescriptorPool::DescriptorPool()
	: descriptorPool(VK_NULL_HANDLE),
	device(nullptr)
{
}

DescriptorPool::~DescriptorPool() {}

void DescriptorPool::createDescriptorPool(
	const Device& device, 
	uint32_t descriptorCount)
{
	this->device = &device;

	// Pool size
	std::array<VkDescriptorPoolSize, 2> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = descriptorCount;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = descriptorCount;

	// Create descriptor pool
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = descriptorCount;
	poolInfo.flags = 0;
	if (vkCreateDescriptorPool(
		this->device->getVkDevice(),
		&poolInfo,
		nullptr,
		&this->descriptorPool) != VK_SUCCESS)
	{
		Log::error("Failed to create descriptor pool.");
	}
}

void DescriptorPool::createImguiDescriptorPool(const Device& device, uint32_t descriptorCount)
{
	this->device = &device;

	// Pool size
	std::array<VkDescriptorPoolSize, 11> poolSizes = 
	{
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 * descriptorCount },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 * descriptorCount },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 * descriptorCount },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 * descriptorCount },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 * descriptorCount },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 * descriptorCount },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 * descriptorCount },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 * descriptorCount },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 * descriptorCount },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 * descriptorCount },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 * descriptorCount }
	};

	// Create descriptor pool
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = 1000 * descriptorCount;
	poolInfo.flags = 0;
	if (vkCreateDescriptorPool(
		this->device->getVkDevice(),
		&poolInfo,
		nullptr,
		&this->descriptorPool) != VK_SUCCESS)
	{
		Log::error("Failed to create descriptor pool.");
	}
}

void DescriptorPool::cleanup()
{
	// Destroys descriptor sets allocated from it
	vkDestroyDescriptorPool(this->device->getVkDevice(), this->descriptorPool, nullptr);
}

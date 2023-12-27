#include "pch.h"
#include "DescriptorSetLayout.h"

DescriptorSetLayout::DescriptorSetLayout()
	: descriptorSetLayout(VK_NULL_HANDLE),
	device(nullptr)
{
}

DescriptorSetLayout::~DescriptorSetLayout()
{
}

void DescriptorSetLayout::createDescriptorSetLayout(
	const Device& device,
	const std::vector<DescriptorSetLayoutElement>& elements)
{
	this->device = &device;

	// Descriptor set layout info, which contains all layout bindings
	std::vector<VkDescriptorSetLayoutBinding> bindings(elements.size());
	for (size_t i = 0; i < bindings.size(); ++i)
	{
		bindings[i].binding = uint32_t(i);
		bindings[i].descriptorType = elements[i].descriptorType;
		bindings[i].descriptorCount = 1;
		bindings[i].stageFlags = elements[i].stage;
		bindings[i].pImmutableSamplers = nullptr;
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	// Create descriptor set layout
	if (vkCreateDescriptorSetLayout(
		this->device->getVkDevice(),
		&layoutInfo,
		nullptr,
		&this->descriptorSetLayout) != VK_SUCCESS)
	{
		Log::error("Failed to create descriptor set layout.");
	}
}

void DescriptorSetLayout::createImguiDescriptorSetLayout(const Device& device)
{
	this->device = &device;

	// Binding
	VkDescriptorSetLayoutBinding binding{};
	binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	binding.descriptorCount = 1;
	binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	// Create info
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &binding;
	if (vkCreateDescriptorSetLayout(
		this->device->getVkDevice(), 
		&layoutInfo,
		nullptr, 
		&this->descriptorSetLayout) != VK_SUCCESS)
	{
		Log::error("Failed to create imgui descriptor set layout.");
	}
}

void DescriptorSetLayout::cleanup()
{
	vkDestroyDescriptorSetLayout(this->device->getVkDevice(), this->descriptorSetLayout, nullptr);
}

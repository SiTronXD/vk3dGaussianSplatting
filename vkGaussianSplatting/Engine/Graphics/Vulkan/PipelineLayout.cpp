#include "pch.h"
#include "PipelineLayout.h"

PipelineLayout::PipelineLayout()
	: pipelineLayout(VK_NULL_HANDLE),
	device(nullptr),
	pushConstantStage(VK_SHADER_STAGE_ALL),
	pushConstantSize(0)
{
}

PipelineLayout::~PipelineLayout()
{
}

void PipelineLayout::createPipelineLayout(
	const Device& device, 
	const std::vector<DescriptorSetLayoutElement>& descriptorSetLayoutElements, 
	const VkShaderStageFlags& pushConstantShaderStageFlags, 
	const uint32_t& pushConstantSize)
{
	this->device = &device;
	this->pushConstantStage = pushConstantShaderStageFlags;
	this->pushConstantSize = pushConstantSize;

	// Create descriptor set layout
	this->descriptorSetLayout.createDescriptorSetLayout(device, descriptorSetLayoutElements);

	// Push constant info
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = pushConstantShaderStageFlags;
	pushConstantRange.offset = 0;
	pushConstantRange.size = pushConstantSize;

	// Create info
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &this->descriptorSetLayout.getVkDescriptorSetLayout();
	pipelineLayoutInfo.pushConstantRangeCount = pushConstantSize > 0;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	if (vkCreatePipelineLayout(
		this->device->getVkDevice(),
		&pipelineLayoutInfo,
		nullptr,
		&this->pipelineLayout) != VK_SUCCESS)
	{
		Log::error("Failed to create pipeline layout!");
	}
}

void PipelineLayout::createImguiPipelineLayout(const Device& device)
{
	this->device = &device;
	this->pushConstantStage = VK_SHADER_STAGE_VERTEX_BIT;
	this->pushConstantSize = sizeof(float) * 4;

	// Create descriptor set layout
	this->descriptorSetLayout.createImguiDescriptorSetLayout(device);

	// Push constant info
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = this->pushConstantStage;
	pushConstantRange.offset = 0;
	pushConstantRange.size = this->pushConstantSize;

	// Create info
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &this->descriptorSetLayout.getVkDescriptorSetLayout();
	pipelineLayoutInfo.pushConstantRangeCount = this->pushConstantSize > 0;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	if (vkCreatePipelineLayout(
		this->device->getVkDevice(),
		&pipelineLayoutInfo,
		nullptr,
		&this->pipelineLayout) != VK_SUCCESS)
	{
		Log::error("Failed to create pipeline layout!");
	}
}

void PipelineLayout::cleanup()
{
	vkDestroyPipelineLayout(this->device->getVkDevice(), this->pipelineLayout, nullptr);

	// Cleanup descriptor set layout as well
	this->descriptorSetLayout.cleanup();
}
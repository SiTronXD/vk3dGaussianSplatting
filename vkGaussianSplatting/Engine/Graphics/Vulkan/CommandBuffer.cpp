#include "pch.h"
#include "CommandBuffer.h"
#include "../Buffer/VertexBuffer.h"
#include "../Buffer/IndexBuffer.h"
#include "PipelineBarrier.h"

CommandBuffer::CommandBuffer()
	: commandBuffer(VK_NULL_HANDLE),
	currentBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS),
	vkCmdPushDescriptorSetKHR(nullptr)
{
}

CommandBuffer::~CommandBuffer()
{
}

void CommandBuffer::init(const Device& device)
{
	// Load push descriptor extension function
	this->vkCmdPushDescriptorSetKHR = (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(device.getVkDevice(), "vkCmdPushDescriptorSetKHR");
	if (!this->vkCmdPushDescriptorSetKHR)
	{
		Log::error("Could not get function pointer for vkCmdPushDescriptorSetKHR.");
	}
}

void CommandBuffer::resetAndBegin()
{
	// Reset command buffer
	vkResetCommandBuffer(this->commandBuffer, 0);

	// Reset and begin recording into command buffer
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;
	if (vkBeginCommandBuffer(this->commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		Log::error("Failed to begin recording command buffer.");
	}
}

void CommandBuffer::beginRenderPass(const VkRenderPassBeginInfo& renderPassInfo)
{
	// Record beginning render pass
	vkCmdBeginRenderPass(
		this->commandBuffer,
		&renderPassInfo,
		VK_SUBPASS_CONTENTS_INLINE
	);
}

void CommandBuffer::beginRendering(const VkRenderingInfo& renderingInfo)
{
	// For dynamic rendering
	vkCmdBeginRendering(this->commandBuffer, &renderingInfo);
}

void CommandBuffer::bindPipeline(const Pipeline& pipeline)
{
	this->currentBindPoint = pipeline.getVkBindPoint();

	vkCmdBindPipeline(
		this->commandBuffer,
		this->currentBindPoint,
		pipeline.getVkPipeline()
	);
}

void CommandBuffer::setViewport(const VkViewport& viewport)
{
	vkCmdSetViewport(this->commandBuffer, 0, 1, &viewport);
}

void CommandBuffer::setScissor(const VkRect2D& scissor)
{
	vkCmdSetScissor(this->commandBuffer, 0, 1, &scissor);
}

void CommandBuffer::bindVertexBuffer(const VertexBuffer& vertexBuffer)
{
	VkBuffer vertexBuffers[] = { vertexBuffer.getVkBuffer() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(this->commandBuffer, 0, 1, vertexBuffers, offsets);
}

void CommandBuffer::bindIndexBuffer(const IndexBuffer& indexBuffer)
{
#ifdef _DEBUG
	if (indexBuffer.getVkBuffer() == VK_NULL_HANDLE)
		Log::error("Index buffer has not been created.");
#endif

	vkCmdBindIndexBuffer(this->commandBuffer, indexBuffer.getVkBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void CommandBuffer::bindDescriptorSet(
	const PipelineLayout& pipelineLayout, 
	const VkDescriptorSet& descriptorSet)
{
	vkCmdBindDescriptorSets(
		this->commandBuffer,
		this->currentBindPoint,
		pipelineLayout.getVkPipelineLayout(),
		0,
		1,
		&descriptorSet,
		0,
		nullptr
	);
}

void CommandBuffer::pushDescriptorSet(
	const PipelineLayout& pipelineLayout,
	uint32_t set,
	uint32_t writeDescriptorCount,
	const VkWriteDescriptorSet* writeDescriptorSets)
{
	this->vkCmdPushDescriptorSetKHR(
		this->commandBuffer,
		this->currentBindPoint,
		pipelineLayout.getVkPipelineLayout(),
		set,
		writeDescriptorCount,
		writeDescriptorSets
	);
}

void CommandBuffer::pushConstant(
	const PipelineLayout& pipelineLayout,
	const void* data)
{
	vkCmdPushConstants(
		this->commandBuffer, 
		pipelineLayout.getVkPipelineLayout(), 
		pipelineLayout.getPushConstantStage(),
		0,
		pipelineLayout.getPushConstantSize(),
		data
	);
}

void CommandBuffer::drawIndexed(uint32_t numIndices, uint32_t firstIndex)
{
	vkCmdDrawIndexed(this->commandBuffer, numIndices, 1, firstIndex, 0, 0);
}

void CommandBuffer::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	vkCmdDispatch(this->commandBuffer, groupCountX, groupCountY, groupCountZ);
}

void CommandBuffer::blit(
	const VkImage& srcImage, VkImageLayout srcImageLayout,
	const VkImage& dstImage, VkImageLayout dstImageLayout,
	const VkImageBlit& blit)
{
	vkCmdBlitImage(
		this->commandBuffer,
		srcImage, srcImageLayout,
		dstImage, dstImageLayout,
		1, &blit,
		VK_FILTER_LINEAR
	);
}

void CommandBuffer::resetEntireQueryPool(const VkQueryPool& queryPool, uint32_t queryCount)
{
	vkCmdResetQueryPool(this->commandBuffer, queryPool, 0, queryCount);
}

void CommandBuffer::writeTimestamp(
	const VkQueryPool& queryPool,
	VkPipelineStageFlagBits pipelineStage, 
	uint32_t queryIndex)
{
	vkCmdWriteTimestamp(this->commandBuffer, pipelineStage, queryPool, queryIndex);
}

void CommandBuffer::memoryBarrier(
	VkAccessFlags2 srcAccessMask,
	VkAccessFlags2 dstAccessMask,
	VkPipelineStageFlags2 srcStageMask,
	VkPipelineStageFlags2 dstStageMask,
	VkImageLayout oldLayout,
	VkImageLayout newLayout,
	VkImage image,
	VkImageAspectFlags imageAspectFlags)
{
	// 1 single image memory barrier
	VkImageMemoryBarrier2 imageMemoryBarrier = PipelineBarrier::imageMemoryBarrier2(
		srcAccessMask,
		dstAccessMask,
		srcStageMask,
		dstStageMask,
		oldLayout,
		newLayout,
		image,
		imageAspectFlags
	);
	this->memoryBarrier(&imageMemoryBarrier, 1);
}

void CommandBuffer::memoryBarrier(
	VkAccessFlags2 srcAccessMask,
	VkAccessFlags2 dstAccessMask,
	VkPipelineStageFlags2 srcStageMask,
	VkPipelineStageFlags2 dstStageMask,
	VkImageLayout oldLayout,
	VkImageLayout newLayout,
	VkImage image,
	const VkImageSubresourceRange& subresourceRange)
{
	// 1 single image memory barrier
	VkImageMemoryBarrier2 imageMemoryBarrier = PipelineBarrier::imageMemoryBarrier2(
		srcAccessMask,
		dstAccessMask,
		srcStageMask,
		dstStageMask,
		oldLayout,
		newLayout,
		image,
		subresourceRange
	);
	this->memoryBarrier(&imageMemoryBarrier, 1);
}

void CommandBuffer::memoryBarrier(
	const VkImageMemoryBarrier2* memoryBarriers,
	uint32_t numMemoryBarriers)
{
	// Pipeline barrier 2
	VkDependencyInfo depInfo{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
	//depInfo.dependencyFlags = ;
	depInfo.imageMemoryBarrierCount = numMemoryBarriers;
	depInfo.pImageMemoryBarriers = memoryBarriers;
	vkCmdPipelineBarrier2(this->commandBuffer, &depInfo);
}

void CommandBuffer::endRenderPass()
{
	// Record ending render pass
	vkCmdEndRenderPass(this->commandBuffer);
}

void CommandBuffer::endRendering()
{
	// For dynamic rendering
	vkCmdEndRendering(this->commandBuffer);
}

void CommandBuffer::end()
{
	// Finish recording
	if (vkEndCommandBuffer(this->commandBuffer) != VK_SUCCESS)
		Log::error("Failed to record command buffer.");
}

void CommandBuffer::setVkCommandBuffer(const VkCommandBuffer& commandBuffer)
{
	this->commandBuffer = commandBuffer;
}

void CommandBuffer::beginSingleTimeUse(const GfxAllocContext& gfxAllocContext)
{
	// Allocate command buffer
	VkCommandBufferAllocateInfo allocInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = gfxAllocContext.singleTimeCommandPool->getVkCommandPool();
	allocInfo.commandBufferCount = 1;
	vkAllocateCommandBuffers(gfxAllocContext.device->getVkDevice(), &allocInfo, &this->commandBuffer);

	// Begin recording command buffer
	VkCommandBufferBeginInfo beginInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(this->commandBuffer, &beginInfo);
}

void CommandBuffer::endSingleTimeUse(const GfxAllocContext& gfxAllocContext)
{
	const VkQueue& queue = gfxAllocContext.queueFamilies->getVkGraphicsQueue();

	// End recording command buffer
	vkEndCommandBuffer(this->commandBuffer);

	// Submit command buffer
	VkSubmitInfo submitInfo { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &this->commandBuffer;
	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	// Deallocate temporary command buffer
	vkFreeCommandBuffers(
		gfxAllocContext.device->getVkDevice(),
		gfxAllocContext.singleTimeCommandPool->getVkCommandPool(),
		1,
		&this->commandBuffer
	);
}
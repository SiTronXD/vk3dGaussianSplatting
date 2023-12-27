#pragma once

#include <vector>

#include "CommandPool.h"
#include "Pipeline.h"

class Device;
class VertexBuffer;
class IndexBuffer;
struct GfxAllocContext;

class CommandBuffer
{
private:
	VkCommandBuffer commandBuffer;
	VkPipelineBindPoint currentBindPoint;

	PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR;

public:
	CommandBuffer();
	~CommandBuffer();

	void init(const Device& device);

	void resetAndBegin();

	void beginRenderPass(const VkRenderPassBeginInfo& renderPassInfo);
	void beginRendering(const VkRenderingInfo& renderingInfo);
	void bindPipeline(const Pipeline& pipeline);
	void setViewport(const VkViewport& viewport);
	void setScissor(const VkRect2D& scissor);
	void bindVertexBuffer(const VertexBuffer& vertexBuffer);
	void bindIndexBuffer(const IndexBuffer& indexBuffer);
	void bindDescriptorSet(
		const PipelineLayout& pipelineLayout,
		const VkDescriptorSet& descriptorSet);
	void pushDescriptorSet(
		const PipelineLayout& pipelineLayout,
		uint32_t set,
		uint32_t writeDescriptorCount,
		const VkWriteDescriptorSet* writeDescriptorSets);
	void pushConstant(
		const PipelineLayout& pipelineLayout,
		const void* data);
	void drawIndexed(uint32_t numIndices, uint32_t firstIndex);
	void dispatch(uint32_t groupCountX, uint32_t groupCountY = 1, uint32_t groupCountZ = 1);
	void blit(
		const VkImage& srcImage, VkImageLayout srcImageLayout,
		const VkImage& dstImage, VkImageLayout dstImageLayout,
		const VkImageBlit& blit);
	void resetEntireQueryPool(
		const VkQueryPool& queryPool,
		uint32_t queryCount);
	void writeTimestamp(
		const VkQueryPool& queryPool, 
		VkPipelineStageFlagBits pipelineStage, 
		uint32_t queryIndex);

	void memoryBarrier(
		VkAccessFlags2 srcAccessMask,
		VkAccessFlags2 dstAccessMask,
		VkPipelineStageFlags2 srcStageMask,
		VkPipelineStageFlags2 dstStageMask,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		VkImage image,
		VkImageAspectFlags imageAspectFlags);
	void memoryBarrier(
		VkAccessFlags2 srcAccessMask,
		VkAccessFlags2 dstAccessMask,
		VkPipelineStageFlags2 srcStageMask,
		VkPipelineStageFlags2 dstStageMask,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		VkImage image,
		const VkImageSubresourceRange& subresourceRange);
	void memoryBarrier(
		const VkImageMemoryBarrier2* memoryBarriers,
		uint32_t numMemoryBarriers);

	void endRenderPass();
	void endRendering();
	void end();

	void setVkCommandBuffer(const VkCommandBuffer& commandBuffer);
	void beginSingleTimeUse(const GfxAllocContext& gfxAllocContext);
	void endSingleTimeUse(const GfxAllocContext& gfxAllocContext);

	inline VkCommandBuffer& getVkCommandBuffer() { return this->commandBuffer; }
};
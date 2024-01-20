#include "pch.h"
#include "Renderer.h"
#include "../ResourceManager.h"
#include "Texture/TextureCube.h"
#include "../Components.h"

void Renderer::renderImgui(CommandBuffer& commandBuffer, ImDrawData* imguiDrawData, uint32_t imageIndex)
{
	// Imgui using dynamic rendering
	VkRenderingAttachmentInfo imguiColorAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
	imguiColorAttachment.imageView = this->swapchain.getVkImageView(imageIndex);
	imguiColorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	imguiColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	imguiColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	VkRenderingInfo imguiRenderingInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
	imguiRenderingInfo.renderArea = { { 0, 0 }, this->swapchain.getVkExtent() };
	imguiRenderingInfo.layerCount = 1;
	imguiRenderingInfo.colorAttachmentCount = 1;
	imguiRenderingInfo.pColorAttachments = &imguiColorAttachment;

	// Bind pipeline before begin rendering because of formats
	commandBuffer.bindPipeline(this->imguiPipeline);

	// Begin rendering for imgui
	commandBuffer.beginRendering(imguiRenderingInfo);

	// Record imgui primitives into it's command buffer
	ImGui_ImplVulkan_RenderDrawData(
		imguiDrawData,
		commandBuffer.getVkCommandBuffer(),
		this->imguiPipeline.getVkPipeline()
	);

	// End rendering
	commandBuffer.endRendering();
}

void Renderer::computeInitSortList(
	CommandBuffer& commandBuffer, 
	const Camera& camera)
{
	// Reset gaussian sort keys (make sure close sorted gaussians have lower valued keys)
	commandBuffer.fillBuffer(
		this->gaussiansSortListSBO,
		sizeof(GaussianSortData) * this->numSortElements,
		std::numeric_limits<uint32_t>::max()
	);

	// Reset gaussian count before culling
	commandBuffer.fillBuffer(
		this->gaussiansCullDataSBO,
		sizeof(GaussianCullData),
		0
	);

	// Reset ranges
	commandBuffer.fillBuffer(
		this->gaussiansTileRangesSBO,
		sizeof(GaussianTileRangeData) * this->getNumTiles(),
		0
	);

	std::array<VkBufferMemoryBarrier2, 3> initBufferBarriers =
	{
		// Gaussians sort list
		PipelineBarrier::bufferMemoryBarrier2(
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			this->gaussiansSortListSBO.getVkBuffer(),
			this->gaussiansSortListSBO.getBufferSize()
		),

		// Gaussians cull data
		PipelineBarrier::bufferMemoryBarrier2(
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			this->gaussiansCullDataSBO.getVkBuffer(),
			this->gaussiansCullDataSBO.getBufferSize()
		),

		// Gaussians tile range data
		PipelineBarrier::bufferMemoryBarrier2(
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			this->gaussiansTileRangesSBO.getVkBuffer(),
			this->gaussiansTileRangesSBO.getBufferSize()
		),
	};
	commandBuffer.bufferMemoryBarrier(
		initBufferBarriers.data(),
		(uint32_t) initBufferBarriers.size()
	);

	// Compute pipeline
	commandBuffer.bindPipeline(this->initSortListPipeline);

	// Binding 0
	VkDescriptorBufferInfo inputCamUboInfo{};
	inputCamUboInfo.buffer = this->camUBO.getVkBuffer();
	inputCamUboInfo.range = this->camUBO.getBufferSize();

	// Binding 1
	VkDescriptorBufferInfo inputGaussiansBufferInfo{};
	inputGaussiansBufferInfo.buffer = this->gaussiansSBO.getVkBuffer();
	inputGaussiansBufferInfo.range = this->gaussiansSBO.getBufferSize();

	// Binding 2
	VkDescriptorBufferInfo outputGaussiansSortInfo{};
	outputGaussiansSortInfo.buffer = this->gaussiansSortListSBO.getVkBuffer();
	outputGaussiansSortInfo.range = this->gaussiansSortListSBO.getBufferSize();

	// Binding 3
	VkDescriptorBufferInfo outputGaussiansCullInfo{};
	outputGaussiansCullInfo.buffer = this->gaussiansCullDataSBO.getVkBuffer();
	outputGaussiansCullInfo.range = this->gaussiansCullDataSBO.getBufferSize();

	// Descriptor sets
	std::array<VkWriteDescriptorSet, 4> computeWriteDescriptorSets
	{
		DescriptorSet::writeBuffer(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &inputCamUboInfo),

		DescriptorSet::writeBuffer(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputGaussiansBufferInfo),
		DescriptorSet::writeBuffer(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &outputGaussiansSortInfo),
		DescriptorSet::writeBuffer(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &outputGaussiansCullInfo)
	};
	commandBuffer.pushDescriptorSet(
		this->initSortListPipelineLayout,
		0,
		uint32_t(computeWriteDescriptorSets.size()),
		computeWriteDescriptorSets.data()
	);

	// Push constant
	InitSortListPCD initSortListPcData{};
	initSortListPcData.clipPlanes = glm::vec4(camera.NEAR_PLANE, camera.FAR_PLANE, (float) this->numGaussians, 0.0f);
	initSortListPcData.resolution = glm::uvec4(
		this->swapchain.getVkExtent().width,
		this->swapchain.getVkExtent().height, 
		0, 
		0
	);
	commandBuffer.pushConstant(
		this->initSortListPipelineLayout,
		(void*)&initSortListPcData
	);

	// Run compute shader
	commandBuffer.dispatch(
		(this->numGaussians + INIT_LIST_WORK_GROUP_SIZE - 1) / INIT_LIST_WORK_GROUP_SIZE
	);
}

void Renderer::computeSortGaussians(CommandBuffer& commandBuffer)
{
	// Make sure number of elements is power of two
	assert((uint32_t)(this->numGaussians & (this->numGaussians - 1)) == 0u);

	// Make sure number of elements is large enough
	assert(this->numGaussians >= BMS_WORK_GROUP_SIZE * 2);

	commandBuffer.bufferMemoryBarrier(
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		this->gaussiansSortListSBO.getVkBuffer(),
		this->gaussiansSortListSBO.getBufferSize()
	);

	// Compute pipeline
	commandBuffer.bindPipeline(this->sortGaussiansPipeline);

	// Binding 0
	VkDescriptorBufferInfo inputOutputGaussiansSortInfo{};
	inputOutputGaussiansSortInfo.buffer = this->gaussiansSortListSBO.getVkBuffer();
	inputOutputGaussiansSortInfo.range = this->gaussiansSortListSBO.getBufferSize();

	// Descriptor set
	std::array<VkWriteDescriptorSet, 1> computeWriteDescriptorSets
	{
		DescriptorSet::writeBuffer(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputOutputGaussiansSortInfo)
	};
	commandBuffer.pushDescriptorSet(
		this->sortGaussiansPipelineLayout,
		0,
		uint32_t(computeWriteDescriptorSets.size()),
		computeWriteDescriptorSets.data()
	);

	uint32_t h = BMS_WORK_GROUP_SIZE * 2;

	this->dispatchBms(
		commandBuffer, 
		BmsSubAlgorithm::LOCAL_BMS, 
		h
	);

	h *= 2;

	for ( ; h <= this->numGaussians; h *= 2)
	{
		this->dispatchBms(
			commandBuffer,
			BmsSubAlgorithm::BIG_FLIP,
			h
		);

		for (uint32_t hh = h / 2; hh > 1; hh /= 2)
		{
			if (hh <= BMS_WORK_GROUP_SIZE * 2)
			{
				this->dispatchBms(
					commandBuffer,
					BmsSubAlgorithm::LOCAL_DISPERSE,
					hh
				);
				break;
			}
			else
			{
				this->dispatchBms(
					commandBuffer,
					BmsSubAlgorithm::BIG_DISPERSE,
					hh
				);
			}
		}
	}
}

void Renderer::computeRanges(CommandBuffer& commandBuffer)
{
	// Gaussians cull data
	commandBuffer.bufferMemoryBarrier(
		VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		this->gaussiansCullDataSBO.getVkBuffer(),
		this->gaussiansCullDataSBO.getBufferSize()
	);

	// Compute pipeline
	commandBuffer.bindPipeline(this->findRangesPipeline);

	// Binding 0
	VkDescriptorBufferInfo inputGaussiansSortInfo{};
	inputGaussiansSortInfo.buffer = this->gaussiansSortListSBO.getVkBuffer();
	inputGaussiansSortInfo.range = this->gaussiansSortListSBO.getBufferSize();

	// Binding 1
	VkDescriptorBufferInfo outputGaussiansRangeInfo{};
	outputGaussiansRangeInfo.buffer = this->gaussiansTileRangesSBO.getVkBuffer();
	outputGaussiansRangeInfo.range = this->gaussiansTileRangesSBO.getBufferSize();

	// Descriptor sets
	std::array<VkWriteDescriptorSet, 2> computeWriteDescriptorSets
	{
		DescriptorSet::writeBuffer(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputGaussiansSortInfo),

		DescriptorSet::writeBuffer(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &outputGaussiansRangeInfo),
	};
	commandBuffer.pushDescriptorSet(
		this->findRangesPipelineLayout,
		0,
		uint32_t(computeWriteDescriptorSets.size()),
		computeWriteDescriptorSets.data()
	);

	// Push constant
	FindRangesPCD findRangesPcData{};
	findRangesPcData.data.x = this->numSortElements;
	commandBuffer.pushConstant(
		this->findRangesPipelineLayout,
		(void*)&findRangesPcData
	);

	// Run compute shader
	commandBuffer.dispatch(
		(this->numSortElements + FIND_RANGES_GROUP_SIZE - 1) / FIND_RANGES_GROUP_SIZE
	);
}

void Renderer::computeRenderGaussians(
	CommandBuffer& commandBuffer,
	uint32_t imageIndex)
{
	// Transition swapchain image layout
	std::array<VkImageMemoryBarrier2, 1> renderGaussiansMemoryBarriers =
	{
		PipelineBarrier::imageMemoryBarrier2(
			VK_ACCESS_NONE,
			VK_ACCESS_SHADER_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_GENERAL,
			this->swapchain.getVkImage(imageIndex),
			VK_IMAGE_ASPECT_COLOR_BIT
		)
	};
	commandBuffer.imageMemoryBarrier(
		renderGaussiansMemoryBarriers.data(), 
		(uint32_t) renderGaussiansMemoryBarriers.size()
	);

	// Gaussians cull data
	commandBuffer.bufferMemoryBarrier(
		VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		this->gaussiansCullDataSBO.getVkBuffer(),
		this->gaussiansCullDataSBO.getBufferSize()
	);

	// Compute pipeline
	commandBuffer.bindPipeline(this->renderGaussiansPipeline);

	// Binding 0
	VkDescriptorBufferInfo inputGaussiansInfo{};
	inputGaussiansInfo.buffer = this->gaussiansSBO.getVkBuffer();
	inputGaussiansInfo.range = this->gaussiansSBO.getBufferSize();

	// Binding 1
	VkDescriptorBufferInfo inputGaussiansSortListInfo{};
	inputGaussiansSortListInfo.buffer = this->gaussiansSortListSBO.getVkBuffer();
	inputGaussiansSortListInfo.range = this->gaussiansSortListSBO.getBufferSize();

	// Binding 2
	VkDescriptorBufferInfo inputGaussiansCullDataInfo{};
	inputGaussiansCullDataInfo.buffer = this->gaussiansCullDataSBO.getVkBuffer();
	inputGaussiansCullDataInfo.range = this->gaussiansCullDataSBO.getBufferSize();

	// Binding 3
	VkDescriptorBufferInfo inputCamUboInfo{};
	inputCamUboInfo.buffer = this->camUBO.getVkBuffer();
	inputCamUboInfo.range = this->camUBO.getBufferSize();

	// Binding 4
	VkDescriptorImageInfo outputImageInfo{};
	outputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	outputImageInfo.imageView = this->swapchain.getVkImageView(imageIndex);

	// Descriptor set
	std::array<VkWriteDescriptorSet, 5> computeWriteDescriptorSets
	{
		DescriptorSet::writeBuffer(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputGaussiansInfo),
		DescriptorSet::writeBuffer(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputGaussiansSortListInfo),
		DescriptorSet::writeBuffer(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputGaussiansCullDataInfo),

		DescriptorSet::writeBuffer(3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &inputCamUboInfo),

		DescriptorSet::writeImage(4, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, &outputImageInfo)
	};
	commandBuffer.pushDescriptorSet(
		this->renderGaussiansPipelineLayout,
		0,
		uint32_t(computeWriteDescriptorSets.size()),
		computeWriteDescriptorSets.data()
	);

	// Push constant
	RenderGaussiansPCD renderGaussiansPcData{};
	renderGaussiansPcData.resolution =
		glm::uvec4(
			this->swapchain.getVkExtent().width,
			this->swapchain.getVkExtent().height,
			this->numGaussians,
			0u
		);
	commandBuffer.pushConstant(
		this->renderGaussiansPipelineLayout,
		(void*)&renderGaussiansPcData
	);

	// Run compute shader
	commandBuffer.dispatch(
		(this->swapchain.getVkExtent().width + 16 - 1) / 16,
		(this->swapchain.getVkExtent().height + 16 - 1) / 16
	);

	// Transition swapchain image layout for presentation
	commandBuffer.imageMemoryBarrier(
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_NONE,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		this->swapchain.getVkImage(imageIndex),
		VK_IMAGE_ASPECT_COLOR_BIT
	);
}

void Renderer::dispatchBms(CommandBuffer& commandBuffer, BmsSubAlgorithm subAlgorithm, uint32_t h)
{
	// Push constant
	SortGaussiansPCD sortGaussiansPcData{};
	sortGaussiansPcData.data.x = static_cast<uint32_t>(subAlgorithm);
	sortGaussiansPcData.data.y = h;
	commandBuffer.pushConstant(
		this->sortGaussiansPipelineLayout,
		(void*)&sortGaussiansPcData
	);

	// Run compute shader
	// Divide workload by 2, since 1 thread works on pairs of elements
	commandBuffer.dispatch(
		this->numGaussians / BMS_WORK_GROUP_SIZE / 2
	);

	commandBuffer.bufferMemoryBarrier(
		VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT, // TODO: should only be read after last dispatch
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		this->gaussiansSortListSBO.getVkBuffer(),
		this->gaussiansSortListSBO.getBufferSize()
	);
}

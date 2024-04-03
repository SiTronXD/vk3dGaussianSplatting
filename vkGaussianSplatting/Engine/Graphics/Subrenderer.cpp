#include "pch.h"
#include "Renderer.h"

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
		this->gaussiansSortListSBO->getVkBuffer(),
		sizeof(GaussianSortData) * this->numSortElements,
		std::numeric_limits<uint32_t>::max()
	);

	// Reset radix sort ping pong buffer, similar to gaussian sort keys
	commandBuffer.fillBuffer(
		this->radixSortPingPongBuffer->getVkBuffer(),
		sizeof(GaussianSortData) * this->numSortElements,
		std::numeric_limits<uint32_t>::max()
	);

	// Reset gaussian count before culling
	commandBuffer.fillBuffer(
		this->gaussiansCullDataSBO.getVkBuffer(),
		sizeof(uint32_t),
		0
	);

	// Reset ranges
	commandBuffer.fillBuffer(
		this->gaussiansTileRangesSBO.getVkBuffer(),
		sizeof(GaussianTileRangeData) * this->getNumTiles(),
		0
	);

	std::array<VkBufferMemoryBarrier2, 4> initBufferBarriers =
	{
		// Gaussians sort list
		PipelineBarrier::bufferMemoryBarrier2(
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			this->gaussiansSortListSBO->getVkBuffer(),
			this->gaussiansSortListSBO->getBufferSize()
		),

		// Radix sort ping pong buffer
		PipelineBarrier::bufferMemoryBarrier2(
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			this->radixSortPingPongBuffer->getVkBuffer(),
			this->radixSortPingPongBuffer->getBufferSize()
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
	inputCamUboInfo.buffer = this->camUBO.getVkBuffer(GfxState::currentFrameIndex);
	inputCamUboInfo.range = this->camUBO.getBufferSize();

	// Binding 1
	VkDescriptorBufferInfo inputGaussiansBufferInfo{};
	inputGaussiansBufferInfo.buffer = this->gaussiansSBO.getVkBuffer();
	inputGaussiansBufferInfo.range = this->gaussiansSBO.getBufferSize();

	// Binding 2
	VkDescriptorBufferInfo outputGaussiansSortInfo{};
	outputGaussiansSortInfo.buffer = this->gaussiansSortListSBO->getVkBuffer();
	outputGaussiansSortInfo.range = this->gaussiansSortListSBO->getBufferSize();

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

void Renderer::computeSortGaussiansBMS(CommandBuffer& commandBuffer, uint32_t numElemToSort)
{
	// Make sure number of elements is power of two
	assert((uint32_t)(numElemToSort & (numElemToSort - 1)) == 0u);

	// Make sure number of elements is large enough
	assert(numElemToSort >= BMS_WORK_GROUP_SIZE * 2);

	// Wait for work on initialization of the sorting list to finish
	std::array<VkBufferMemoryBarrier2, 2> initBufferBarriers =
	{
		// Gaussians sort list
		PipelineBarrier::bufferMemoryBarrier2(
			VK_ACCESS_SHADER_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			this->gaussiansSortListSBO->getVkBuffer(),
			this->gaussiansSortListSBO->getBufferSize()
		),

		// Gaussians
		PipelineBarrier::bufferMemoryBarrier2(
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			this->gaussiansSBO.getVkBuffer(),
			this->gaussiansSBO.getBufferSize()
		)
	};
	commandBuffer.bufferMemoryBarrier(
		initBufferBarriers.data(),
		(uint32_t) initBufferBarriers.size()
	);

	// Compute pipeline
	commandBuffer.bindPipeline(this->sortGaussiansBmsPipeline);

	// Binding 0
	VkDescriptorBufferInfo inputOutputGaussiansSortInfo{};
	inputOutputGaussiansSortInfo.buffer = this->gaussiansSortListSBO->getVkBuffer();
	inputOutputGaussiansSortInfo.range = this->gaussiansSortListSBO->getBufferSize();

	// Descriptor set
	std::array<VkWriteDescriptorSet, 1> computeWriteDescriptorSets
	{
		DescriptorSet::writeBuffer(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputOutputGaussiansSortInfo)
	};
	commandBuffer.pushDescriptorSet(
		this->sortGaussiansBmsPipelineLayout,
		0,
		uint32_t(computeWriteDescriptorSets.size()),
		computeWriteDescriptorSets.data()
	);

	uint32_t h = BMS_WORK_GROUP_SIZE * 2;

	this->dispatchBms(
		commandBuffer,
		BmsSubAlgorithm::LOCAL_BMS,
		h,
		numElemToSort
	);

	h *= 2;

	for (; h <= numElemToSort; h *= 2)
	{
		this->dispatchBms(
			commandBuffer,
			BmsSubAlgorithm::BIG_FLIP,
			h,
			numElemToSort
		);

		for (uint32_t hh = h / 2; hh > 1; hh /= 2)
		{
			if (hh <= BMS_WORK_GROUP_SIZE * 2)
			{
				this->dispatchBms(
					commandBuffer,
					BmsSubAlgorithm::LOCAL_DISPERSE,
					hh,
					numElemToSort
				);
				break;
			}
			else
			{
				this->dispatchBms(
					commandBuffer,
					BmsSubAlgorithm::BIG_DISPERSE,
					hh,
					numElemToSort
				);
			}
		}
	}
}

void Renderer::computeSortGaussiansRS(CommandBuffer& commandBuffer)
{
	// TODO: add static asserts concerning shared memory limitations based on 
	// WORK_GROUP_SIZE and BIN_COUNT.

	// Sanity check for now, to ensure that no bits outside of 64 are evaluated.
	// This check might be removed if RS_BITS_PER_PASS is increased to 5 or 6. But should that 
	// be the case, then the shaders have to take that change into account.
	assert(this->radixSortNumSortBits % RS_BITS_PER_PASS == 0);

	// Limitation of the scatter shader
	assert(RS_BITS_PER_PASS % 2 == 0);
	assert(RS_WORK_GROUP_SIZE >= RS_BIN_COUNT);

	// Wait for work on initialization of the sorting list to finish
	std::array<VkBufferMemoryBarrier2, 4> initBufferBarriers =
	{
		// Gaussians sort list
		PipelineBarrier::bufferMemoryBarrier2(
			VK_ACCESS_SHADER_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			this->gaussiansSortListSBO->getVkBuffer(),
			this->gaussiansSortListSBO->getBufferSize()
		),

		// Gaussians
		PipelineBarrier::bufferMemoryBarrier2(
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			this->gaussiansSBO.getVkBuffer(),
			this->gaussiansSBO.getBufferSize()
		),

		// Gaussians cull data
		PipelineBarrier::bufferMemoryBarrier2(
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			this->gaussiansCullDataSBO.getVkBuffer(),
			this->gaussiansCullDataSBO.getBufferSize()
		),

		// Indirect dispatch
		PipelineBarrier::bufferMemoryBarrier2(
			VK_ACCESS_NONE,
			VK_ACCESS_SHADER_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			this->radixSortIndirectDispatchBuffer.getVkBuffer(),
			this->radixSortIndirectDispatchBuffer.getBufferSize()
		)
	};
	commandBuffer.bufferMemoryBarrier(
		initBufferBarriers.data(),
		(uint32_t) initBufferBarriers.size()
	);

	// ------------------ 0. Indirect setup ------------------
	{
		// Compute pipeline
		commandBuffer.bindPipeline(this->radixSortIndirectSetupPipeline);

		// Binding 0
		VkDescriptorBufferInfo inputCullInfo{};
		inputCullInfo.buffer = this->gaussiansCullDataSBO.getVkBuffer();
		inputCullInfo.range = this->gaussiansCullDataSBO.getBufferSize();

		// Binding 1
		VkDescriptorBufferInfo outputIndirectDispatchInfo{};
		outputIndirectDispatchInfo.buffer = this->radixSortIndirectDispatchBuffer.getVkBuffer();
		outputIndirectDispatchInfo.range = this->radixSortIndirectDispatchBuffer.getBufferSize();

		// Descriptor sets
		std::array<VkWriteDescriptorSet, 2> indirectSetupDescriptorSets
		{
			DescriptorSet::writeBuffer(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputCullInfo),
			DescriptorSet::writeBuffer(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &outputIndirectDispatchInfo),
		};
		commandBuffer.pushDescriptorSet(
			this->radixSortIndirectSetupPipelineLayout,
			0,
			uint32_t(indirectSetupDescriptorSets.size()),
			indirectSetupDescriptorSets.data()
		);

		// Dispatch
		commandBuffer.dispatch(1);
	}

	// Wait on dispatch parameters
	commandBuffer.bufferMemoryBarrier(
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
		this->radixSortIndirectDispatchBuffer.getVkBuffer(),
		this->radixSortIndirectDispatchBuffer.getBufferSize()
	);

	SortGaussiansRsPCD sortGaussiansPcData{};

	StorageBuffer* srcSortBuffer = this->gaussiansSortListSBO.get();
	StorageBuffer* dstSortBuffer = this->radixSortPingPongBuffer.get();

	for (uint32_t shiftBits = 0; shiftBits < this->radixSortNumSortBits; shiftBits += RS_BITS_PER_PASS)
	{
		sortGaussiansPcData.data.x = shiftBits;

		// ------------------ 1. Count ------------------
		{
			// Compute pipeline
			commandBuffer.bindPipeline(this->radixSortCountPipeline);

			// Binding 0
			VkDescriptorBufferInfo inputIndirectDispatchInfo{};
			inputIndirectDispatchInfo.buffer = this->radixSortIndirectDispatchBuffer.getVkBuffer();
			inputIndirectDispatchInfo.range = this->radixSortIndirectDispatchBuffer.getBufferSize();

			// Binding 1
			VkDescriptorBufferInfo inputGaussiansSortInfo{};
			inputGaussiansSortInfo.buffer = srcSortBuffer->getVkBuffer();
			inputGaussiansSortInfo.range = srcSortBuffer->getBufferSize();

			// Binding 2
			VkDescriptorBufferInfo outputSumTableInfo{};
			outputSumTableInfo.buffer = this->radixSortSumTableBuffer.getVkBuffer();
			outputSumTableInfo.range = this->radixSortSumTableBuffer.getBufferSize();

			// Descriptor sets
			std::array<VkWriteDescriptorSet, 3> countDescriptorSets
			{
				DescriptorSet::writeBuffer(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputIndirectDispatchInfo),
				DescriptorSet::writeBuffer(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputGaussiansSortInfo),
				DescriptorSet::writeBuffer(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &outputSumTableInfo)
			};
			commandBuffer.pushDescriptorSet(
				this->radixSortCountPipelineLayout,
				0,
				uint32_t(countDescriptorSets.size()),
				countDescriptorSets.data()
			);

			// Push constant
			commandBuffer.pushConstant(
				this->radixSortCountPipelineLayout,
				(void*)&sortGaussiansPcData
			);

			// Dispatch
			commandBuffer.dispatchIndirect(
				this->radixSortIndirectDispatchBuffer.getVkBuffer(), 
				offsetof(RadixIndirectDispatch, countSizeX)
			);
		}

		// ------------------ 2. Reduce ------------------
		{
			commandBuffer.bufferMemoryBarrier(
				VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
				VK_ACCESS_SHADER_READ_BIT,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				this->radixSortSumTableBuffer.getVkBuffer(),
				this->radixSortSumTableBuffer.getBufferSize()
			);

			// Compute pipeline
			commandBuffer.bindPipeline(this->radixSortReducePipeline);

			// Binding 0
			VkDescriptorBufferInfo inputIndirectDispatchInfo{};
			inputIndirectDispatchInfo.buffer = this->radixSortIndirectDispatchBuffer.getVkBuffer();
			inputIndirectDispatchInfo.range = this->radixSortIndirectDispatchBuffer.getBufferSize();

			// Binding 1
			VkDescriptorBufferInfo inputSumTableInfo{};
			inputSumTableInfo.buffer = this->radixSortSumTableBuffer.getVkBuffer();
			inputSumTableInfo.range = this->radixSortSumTableBuffer.getBufferSize();

			// Binding 2
			VkDescriptorBufferInfo outputReduceInfo{};
			outputReduceInfo.buffer = this->radixSortReduceBuffer.getVkBuffer();
			outputReduceInfo.range = this->radixSortReduceBuffer.getBufferSize();

			// Descriptor sets
			std::array<VkWriteDescriptorSet, 3> reduceDescriptorSets
			{
				DescriptorSet::writeBuffer(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputIndirectDispatchInfo),
				DescriptorSet::writeBuffer(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputSumTableInfo),
				DescriptorSet::writeBuffer(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &outputReduceInfo),
			};
			commandBuffer.pushDescriptorSet(
				this->radixSortReducePipelineLayout,
				0,
				uint32_t(reduceDescriptorSets.size()),
				reduceDescriptorSets.data()
			);

			// Dispatch
			commandBuffer.dispatchIndirect(
				this->radixSortIndirectDispatchBuffer.getVkBuffer(), 
				offsetof(RadixIndirectDispatch, reduceSizeX)
			);
		}

		// ------------------ 3. Scan ------------------
		{
			commandBuffer.bufferMemoryBarrier(
				VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
				VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				this->radixSortReduceBuffer.getVkBuffer(),
				this->radixSortReduceBuffer.getBufferSize()
			);


			// Compute pipeline
			commandBuffer.bindPipeline(this->radixSortScanPipeline);

			// Binding 0
			VkDescriptorBufferInfo inputIndirectDispatchInfo{};
			inputIndirectDispatchInfo.buffer = this->radixSortIndirectDispatchBuffer.getVkBuffer();
			inputIndirectDispatchInfo.range = this->radixSortIndirectDispatchBuffer.getBufferSize();

			// Binding 1
			VkDescriptorBufferInfo inputOutputReduceInfo{};
			inputOutputReduceInfo.buffer = this->radixSortReduceBuffer.getVkBuffer();
			inputOutputReduceInfo.range = this->radixSortReduceBuffer.getBufferSize();

			// Descriptor sets
			std::array<VkWriteDescriptorSet, 2> scanDescriptorSets
			{
				DescriptorSet::writeBuffer(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputIndirectDispatchInfo),
				DescriptorSet::writeBuffer(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputOutputReduceInfo)
			};
			commandBuffer.pushDescriptorSet(
				this->radixSortScanPipelineLayout,
				0,
				uint32_t(scanDescriptorSets.size()),
				scanDescriptorSets.data()
			);

			// Dispatch
			commandBuffer.dispatch(1);
		}

		// ------------------ 4. Scan add ------------------
		{
			commandBuffer.bufferMemoryBarrier(
				VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
				VK_ACCESS_SHADER_READ_BIT,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				this->radixSortReduceBuffer.getVkBuffer(),
				this->radixSortReduceBuffer.getBufferSize()
			);


			// Compute pipeline
			commandBuffer.bindPipeline(this->radixSortScanAddPipeline);

			// Binding 0
			VkDescriptorBufferInfo inputIndirectDispatchInfo{};
			inputIndirectDispatchInfo.buffer = this->radixSortIndirectDispatchBuffer.getVkBuffer();
			inputIndirectDispatchInfo.range = this->radixSortIndirectDispatchBuffer.getBufferSize();

			// Binding 1
			VkDescriptorBufferInfo inputReduceInfo{};
			inputReduceInfo.buffer = this->radixSortReduceBuffer.getVkBuffer();
			inputReduceInfo.range = this->radixSortReduceBuffer.getBufferSize();

			// Binding 2
			VkDescriptorBufferInfo inputOutputSumTableInfo{};
			inputOutputSumTableInfo.buffer = this->radixSortSumTableBuffer.getVkBuffer();
			inputOutputSumTableInfo.range = this->radixSortSumTableBuffer.getBufferSize();

			// Descriptor sets
			std::array<VkWriteDescriptorSet, 3> scanAddDescriptorSets
			{
				DescriptorSet::writeBuffer(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputIndirectDispatchInfo),
				DescriptorSet::writeBuffer(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputReduceInfo),
				DescriptorSet::writeBuffer(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputOutputSumTableInfo)
			};
			commandBuffer.pushDescriptorSet(
				this->radixSortScanAddPipelineLayout,
				0,
				uint32_t(scanAddDescriptorSets.size()),
				scanAddDescriptorSets.data()
			);

			// Dispatch
			commandBuffer.dispatchIndirect(
				this->radixSortIndirectDispatchBuffer.getVkBuffer(), 
				offsetof(RadixIndirectDispatch, reduceSizeX)
			);
		}

		// ------------------ 5. Scatter ------------------
		{
			commandBuffer.bufferMemoryBarrier(
				VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
				VK_ACCESS_SHADER_READ_BIT,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				this->radixSortSumTableBuffer.getVkBuffer(),
				this->radixSortSumTableBuffer.getBufferSize()
			);

			// Compute pipeline
			commandBuffer.bindPipeline(this->radixSortScatterPipeline);

			// Binding 0
			VkDescriptorBufferInfo inputIndirectDispatchInfo{};
			inputIndirectDispatchInfo.buffer = this->radixSortIndirectDispatchBuffer.getVkBuffer();
			inputIndirectDispatchInfo.range = this->radixSortIndirectDispatchBuffer.getBufferSize();

			// Binding 1
			VkDescriptorBufferInfo inputSumTableInfo{};
			inputSumTableInfo.buffer = this->radixSortSumTableBuffer.getVkBuffer();
			inputSumTableInfo.range = this->radixSortSumTableBuffer.getBufferSize();

			// Binding 2
			VkDescriptorBufferInfo inputSortSourceInfo{};
			inputSortSourceInfo.buffer = srcSortBuffer->getVkBuffer();
			inputSortSourceInfo.range = srcSortBuffer->getBufferSize();

			// Binding 3
			VkDescriptorBufferInfo outputSortDestinationInfo{};
			outputSortDestinationInfo.buffer = dstSortBuffer->getVkBuffer();
			outputSortDestinationInfo.range = dstSortBuffer->getBufferSize();

			// Descriptor sets
			std::array<VkWriteDescriptorSet, 4> scatterDescriptorSets
			{
				DescriptorSet::writeBuffer(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputIndirectDispatchInfo),
				DescriptorSet::writeBuffer(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputSumTableInfo),
				DescriptorSet::writeBuffer(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputSortSourceInfo),
				DescriptorSet::writeBuffer(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &outputSortDestinationInfo)
			};
			commandBuffer.pushDescriptorSet(
				this->radixSortScatterPipelineLayout,
				0,
				uint32_t(scatterDescriptorSets.size()),
				scatterDescriptorSets.data()
			);

			// Push constant
			commandBuffer.pushConstant(
				this->radixSortScatterPipelineLayout,
				(void*)&sortGaussiansPcData
			);

			// Dispatch
			commandBuffer.dispatchIndirect(
				this->radixSortIndirectDispatchBuffer.getVkBuffer(), 
				offsetof(RadixIndirectDispatch, countSizeX)
			);

			// No matter which shader reads dst next, the shader should wait for dst
			commandBuffer.bufferMemoryBarrier(
				VK_ACCESS_SHADER_WRITE_BIT,
				VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				dstSortBuffer->getVkBuffer(),
				dstSortBuffer->getBufferSize()
			);

			// Swap
			StorageBuffer* temp = srcSortBuffer;
			srcSortBuffer = dstSortBuffer;
			dstSortBuffer = temp;
		}

		// Swap if original sort list is not pointing to correct buffer
		if ((this->radixSortNumSortBits / RS_BITS_PER_PASS) % 2 != 0)
		{
			// Swap
			this->tempSwapPingPongBuffer = this->gaussiansSortListSBO;
			this->gaussiansSortListSBO = this->radixSortPingPongBuffer;
			this->radixSortPingPongBuffer = this->tempSwapPingPongBuffer;
		}
	}
}

void Renderer::computeSortGaussians(CommandBuffer& commandBuffer, uint32_t numElemToSort)
{
	//this->computeSortGaussiansBMS(commandBuffer, numElemToSort);
	this->computeSortGaussiansRS(commandBuffer);
}

void Renderer::computeRanges(CommandBuffer& commandBuffer)
{
	// *Memory barrier has already been inserted at the end of the last sorting pass*

	// Compute pipeline
	commandBuffer.bindPipeline(this->findRangesPipeline);

	// Binding 0
	VkDescriptorBufferInfo inputGaussiansSortInfo{};
	inputGaussiansSortInfo.buffer = this->gaussiansSortListSBO->getVkBuffer();
	inputGaussiansSortInfo.range = this->gaussiansSortListSBO->getBufferSize();

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
	// TODO: make this an indirect dispatch if radix sort is being used
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

	std::array<VkBufferMemoryBarrier2, 1> renderGaussiansBufferBarriers =
	{
		// Range data
		PipelineBarrier::bufferMemoryBarrier2(
			VK_ACCESS_SHADER_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			this->gaussiansTileRangesSBO.getVkBuffer(),
			this->gaussiansTileRangesSBO.getBufferSize()
		)
	};

	// Gaussians cull data
	commandBuffer.bufferMemoryBarrier(
		renderGaussiansBufferBarriers.data(),
		(uint32_t) renderGaussiansBufferBarriers.size()
	);

	// Compute pipeline
	commandBuffer.bindPipeline(this->renderGaussiansPipeline);

	// Binding 0
	VkDescriptorBufferInfo inputGaussiansInfo{};
	inputGaussiansInfo.buffer = this->gaussiansSBO.getVkBuffer();
	inputGaussiansInfo.range = this->gaussiansSBO.getBufferSize();

	// Binding 1
	VkDescriptorBufferInfo inputGaussiansSortListInfo{};
	inputGaussiansSortListInfo.buffer = this->gaussiansSortListSBO->getVkBuffer();
	inputGaussiansSortListInfo.range = this->gaussiansSortListSBO->getBufferSize();

	// Binding 2
	VkDescriptorBufferInfo inputGaussiansRangeInfo{};
	inputGaussiansRangeInfo.buffer = this->gaussiansTileRangesSBO.getVkBuffer();
	inputGaussiansRangeInfo.range = this->gaussiansTileRangesSBO.getBufferSize();

	// Binding 3
	VkDescriptorBufferInfo inputCamUboInfo{};
	inputCamUboInfo.buffer = this->camUBO.getVkBuffer(GfxState::currentFrameIndex);
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
		DescriptorSet::writeBuffer(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputGaussiansRangeInfo),

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
			0
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

void Renderer::dispatchBms(
	CommandBuffer& commandBuffer, 
	BmsSubAlgorithm subAlgorithm, 
	uint32_t h,
	uint32_t numElemToSort)
{
	// Push constant
	SortGaussiansBmsPCD sortGaussiansPcData{};
	sortGaussiansPcData.data.x = static_cast<uint32_t>(subAlgorithm);
	sortGaussiansPcData.data.y = h;
	commandBuffer.pushConstant(
		this->sortGaussiansBmsPipelineLayout,
		(void*)&sortGaussiansPcData
	);

	// Run compute shader
	// Divide workload by 2, since 1 thread works on pairs of elements
	commandBuffer.dispatch(
		numElemToSort / BMS_WORK_GROUP_SIZE / 2
	);

	commandBuffer.bufferMemoryBarrier(
		VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT, // TODO: should only be read after last dispatch
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		this->gaussiansSortListSBO->getVkBuffer(),
		this->gaussiansSortListSBO->getBufferSize()
	);
}

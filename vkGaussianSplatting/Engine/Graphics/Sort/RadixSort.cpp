#include "pch.h"
#include "RadixSort.h"

uint32_t RadixSort::getMinNumBits(uint32_t x) const
{
	for (int32_t i = 32 - 1; i >= 0; --i)
	{
		if (uint32_t(x >> i) & 1)
			return static_cast<uint32_t>(i + 1);
	}

	return 0;
}

RadixSort::RadixSort()
	: gfxAllocContext(nullptr),
	maxNumSortElements(0),
	radixSortNumSortBits(0)
{

}

void RadixSort::singleInitResources(const GfxAllocContext& allocContext)
{
	this->gfxAllocContext = &allocContext;

	// Radix sort compute pipeline (indirect setup)
	this->indirectSetupPipelineLayout.createPipelineLayout(
		*this->gfxAllocContext->device,
		{
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT }
		},
		VK_SHADER_STAGE_COMPUTE_BIT
	);
	this->indirectSetupPipeline.createComputePipeline(
		*this->gfxAllocContext->device,
		this->indirectSetupPipelineLayout,
		"Resources/Shaders/RadixSortIndirectSetup.comp.spv",
		{
			SpecializationConstant{ (void*) RS_WORK_GROUP_SIZE, sizeof(uint32_t)}
		}
	);

	// Radix sort compute pipeline (count)
	this->countPipelineLayout.createPipelineLayout(
		*this->gfxAllocContext->device,
		{
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT }
		},
		VK_SHADER_STAGE_COMPUTE_BIT,
		sizeof(SortGaussiansRsPCD)
	);
	this->countPipeline.createComputePipeline(
		*this->gfxAllocContext->device,
		this->countPipelineLayout,
		"Resources/Shaders/RadixSortCount.comp.spv",
		{
			SpecializationConstant{ (void*) RS_WORK_GROUP_SIZE, sizeof(uint32_t)}
		}
	);

	// Radix sort compute pipeline (reduce)
	this->reducePipelineLayout.createPipelineLayout(
		*this->gfxAllocContext->device,
		{
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT }
		},
		VK_SHADER_STAGE_COMPUTE_BIT
	);
	this->reducePipeline.createComputePipeline(
		*this->gfxAllocContext->device,
		this->reducePipelineLayout,
		"Resources/Shaders/RadixSortReduce.comp.spv",
		{
			SpecializationConstant{ (void*) RS_WORK_GROUP_SIZE, sizeof(uint32_t)}
		}
	);

	// Radix sort compute pipeline (scan)
	this->scanPipelineLayout.createPipelineLayout(
		*this->gfxAllocContext->device,
		{
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT }
		},
		VK_SHADER_STAGE_COMPUTE_BIT
	);
	this->scanPipeline.createComputePipeline(
		*this->gfxAllocContext->device,
		this->scanPipelineLayout,
		"Resources/Shaders/RadixSortScan.comp.spv",
		{
			SpecializationConstant{ (void*) RS_SCAN_WORK_GROUP_SIZE, sizeof(uint32_t)}
		}
	);

	// Radix sort compute pipeline (scan add)
	this->scanAddPipelineLayout.createPipelineLayout(
		*this->gfxAllocContext->device,
		{
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT }
		},
		VK_SHADER_STAGE_COMPUTE_BIT
	);
	this->scanAddPipeline.createComputePipeline(
		*this->gfxAllocContext->device,
		this->scanAddPipelineLayout,
		"Resources/Shaders/RadixSortScanAdd.comp.spv",
		{
			SpecializationConstant{ (void*) RS_WORK_GROUP_SIZE, sizeof(uint32_t)}
		}
	);

	// Radix sort compute pipeline (scatter)
	this->scatterPipelineLayout.createPipelineLayout(
		*this->gfxAllocContext->device,
		{
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT }
		},
		VK_SHADER_STAGE_COMPUTE_BIT,
		sizeof(SortGaussiansRsPCD)
	);
	this->scatterPipeline.createComputePipeline(
		*this->gfxAllocContext->device,
		this->scatterPipelineLayout,
		"Resources/Shaders/RadixSortScatter.comp.spv",
		{
			SpecializationConstant{ (void*) RS_WORK_GROUP_SIZE, sizeof(uint32_t)}
		}
	);
}

void RadixSort::initForScene(uint32_t maxNumSortElements, uint32_t numTiles)
{
	this->maxNumSortElements = maxNumSortElements;

	uint32_t numCountThreadGroups = (this->maxNumSortElements + RS_WORK_GROUP_SIZE - 1) / RS_WORK_GROUP_SIZE;
	uint32_t numSumElements = numCountThreadGroups * RS_BIN_COUNT;
	uint32_t numReduceBlocks = (numCountThreadGroups + RS_WORK_GROUP_SIZE - 1) / RS_WORK_GROUP_SIZE;
	uint32_t numReduceElements = numReduceBlocks * RS_BIN_COUNT;

	// Indirect dispatch buffer
	RadixIndirectDispatch initIndirectDispatch{};
	initIndirectDispatch.numSortElements = this->maxNumSortElements;
	initIndirectDispatch.countSizeX = numCountThreadGroups;
	initIndirectDispatch.reduceSizeX = numReduceElements;
	this->indirectDispatchBuffer.createGpuBuffer(
		*this->gfxAllocContext,
		sizeof(RadixIndirectDispatch),
		&initIndirectDispatch,
		VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT
	);

	// Sum table
	const std::vector<glm::uvec4> dummySumTableData(numSumElements);
	this->sumTableBuffer.createGpuBuffer(
		*this->gfxAllocContext,
		sizeof(dummySumTableData[0]) * dummySumTableData.size(),
		dummySumTableData.data()
	);

	// Reduce buffer
	const std::vector<glm::uvec4> dummyReduceData(numReduceElements);
	this->reduceBuffer.createGpuBuffer(
		*this->gfxAllocContext,
		sizeof(dummyReduceData[0]) * dummyReduceData.size(),
		dummyReduceData.data()
	);

	// Ping pong buffer
	GaussianSortData clearedSortData
	{ 
		glm::uvec4(
			std::numeric_limits<uint32_t>::max(), 
			std::numeric_limits<uint32_t>::max(), 
			std::numeric_limits<uint32_t>::max(), 
			std::numeric_limits<uint32_t>::max()
		)
	};
	std::vector<GaussianSortData> sortData(this->maxNumSortElements, clearedSortData);
	this->pingPongBuffer = std::make_shared<StorageBuffer>();
	this->pingPongBuffer->createGpuBuffer(
		*this->gfxAllocContext,
		sizeof(sortData[0]) * sortData.size(),
		sortData.data()
	);
	sortData.clear();
	sortData.shrink_to_fit();

	// Not all of the highest bits in the sorting keys are utilized, 
	// meaning that sorting only needs to be done for the lowest bits actually being used.
	uint32_t sortBits = 32 + this->getMinNumBits(numTiles - 1);
	this->radixSortNumSortBits = uint32_t((sortBits + RS_BITS_PER_PASS - 1) / RS_BITS_PER_PASS) * RS_BITS_PER_PASS;
}

void RadixSort::computeSort(
	CommandBuffer& commandBuffer,
	StorageBuffer& gaussiansCullDataSBO,
	std::shared_ptr<StorageBuffer>& gaussiansSortListSBO)
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
	std::array<VkBufferMemoryBarrier2, 3> initBufferBarriers =
	{
		// Gaussians sort list
		PipelineBarrier::bufferMemoryBarrier2(
			VK_ACCESS_SHADER_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			gaussiansSortListSBO->getVkBuffer(),
			gaussiansSortListSBO->getBufferSize()
		),

		// Gaussians cull data
		PipelineBarrier::bufferMemoryBarrier2(
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			gaussiansCullDataSBO.getVkBuffer(),
			gaussiansCullDataSBO.getBufferSize()
		),

		// Indirect dispatch
		PipelineBarrier::bufferMemoryBarrier2(
			VK_ACCESS_NONE,
			VK_ACCESS_SHADER_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			this->indirectDispatchBuffer.getVkBuffer(),
			this->indirectDispatchBuffer.getBufferSize()
		)
	};
	commandBuffer.bufferMemoryBarrier(
		initBufferBarriers.data(),
		(uint32_t) initBufferBarriers.size()
	);

	// ------------------ 0. Indirect setup ------------------
	{
		// Compute pipeline
		commandBuffer.bindPipeline(this->indirectSetupPipeline);

		// Binding 0
		VkDescriptorBufferInfo inputCullInfo{};
		inputCullInfo.buffer = gaussiansCullDataSBO.getVkBuffer();
		inputCullInfo.range = gaussiansCullDataSBO.getBufferSize();

		// Binding 1
		VkDescriptorBufferInfo outputIndirectDispatchInfo{};
		outputIndirectDispatchInfo.buffer = this->indirectDispatchBuffer.getVkBuffer();
		outputIndirectDispatchInfo.range = this->indirectDispatchBuffer.getBufferSize();

		// Descriptor sets
		std::array<VkWriteDescriptorSet, 2> indirectSetupDescriptorSets
		{
			DescriptorSet::writeBuffer(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputCullInfo),
			DescriptorSet::writeBuffer(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &outputIndirectDispatchInfo),
		};
		commandBuffer.pushDescriptorSet(
			this->indirectSetupPipelineLayout,
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
		this->indirectDispatchBuffer.getVkBuffer(),
		this->indirectDispatchBuffer.getBufferSize()
	);

	SortGaussiansRsPCD sortGaussiansPcData{};

	StorageBuffer* srcSortBuffer = gaussiansSortListSBO.get();
	StorageBuffer* dstSortBuffer = this->pingPongBuffer.get();

	for (uint32_t shiftBits = 0; shiftBits < this->radixSortNumSortBits; shiftBits += RS_BITS_PER_PASS)
	{
		sortGaussiansPcData.data.x = shiftBits;

		// ------------------ 1. Count ------------------
		{
			// Compute pipeline
			commandBuffer.bindPipeline(this->countPipeline);

			// Binding 0
			VkDescriptorBufferInfo inputIndirectDispatchInfo{};
			inputIndirectDispatchInfo.buffer = this->indirectDispatchBuffer.getVkBuffer();
			inputIndirectDispatchInfo.range = this->indirectDispatchBuffer.getBufferSize();

			// Binding 1
			VkDescriptorBufferInfo inputGaussiansSortInfo{};
			inputGaussiansSortInfo.buffer = srcSortBuffer->getVkBuffer();
			inputGaussiansSortInfo.range = srcSortBuffer->getBufferSize();

			// Binding 2
			VkDescriptorBufferInfo outputSumTableInfo{};
			outputSumTableInfo.buffer = this->sumTableBuffer.getVkBuffer();
			outputSumTableInfo.range = this->sumTableBuffer.getBufferSize();

			// Descriptor sets
			std::array<VkWriteDescriptorSet, 3> countDescriptorSets
			{
				DescriptorSet::writeBuffer(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputIndirectDispatchInfo),
				DescriptorSet::writeBuffer(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputGaussiansSortInfo),
				DescriptorSet::writeBuffer(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &outputSumTableInfo)
			};
			commandBuffer.pushDescriptorSet(
				this->countPipelineLayout,
				0,
				uint32_t(countDescriptorSets.size()),
				countDescriptorSets.data()
			);

			// Push constant
			commandBuffer.pushConstant(
				this->countPipelineLayout,
				(void*)&sortGaussiansPcData
			);

			// Dispatch
			commandBuffer.dispatchIndirect(
				this->indirectDispatchBuffer.getVkBuffer(), 
				offsetof(RadixIndirectDispatch, countSizeX)
			);
		}

		// ------------------ 2. Reduce ------------------
		{
			std::array<VkBufferMemoryBarrier2, 2> reduceWaitMemoryBarriers
			{
				PipelineBarrier::bufferMemoryBarrier2(
					VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
					VK_ACCESS_SHADER_READ_BIT,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					this->sumTableBuffer.getVkBuffer(),
					this->sumTableBuffer.getBufferSize()
				),

				PipelineBarrier::bufferMemoryBarrier2(
					VK_ACCESS_NONE,
					VK_ACCESS_SHADER_WRITE_BIT,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					this->reduceBuffer.getVkBuffer(),
					this->reduceBuffer.getBufferSize()
				)
			};

			commandBuffer.bufferMemoryBarrier(
				reduceWaitMemoryBarriers.data(),
				reduceWaitMemoryBarriers.size()
			);

			// Compute pipeline
			commandBuffer.bindPipeline(this->reducePipeline);

			// Binding 0
			VkDescriptorBufferInfo inputIndirectDispatchInfo{};
			inputIndirectDispatchInfo.buffer = this->indirectDispatchBuffer.getVkBuffer();
			inputIndirectDispatchInfo.range = this->indirectDispatchBuffer.getBufferSize();

			// Binding 1
			VkDescriptorBufferInfo inputSumTableInfo{};
			inputSumTableInfo.buffer = this->sumTableBuffer.getVkBuffer();
			inputSumTableInfo.range = this->sumTableBuffer.getBufferSize();

			// Binding 2
			VkDescriptorBufferInfo outputReduceInfo{};
			outputReduceInfo.buffer = this->reduceBuffer.getVkBuffer();
			outputReduceInfo.range = this->reduceBuffer.getBufferSize();

			// Descriptor sets
			std::array<VkWriteDescriptorSet, 3> reduceDescriptorSets
			{
				DescriptorSet::writeBuffer(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputIndirectDispatchInfo),
				DescriptorSet::writeBuffer(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputSumTableInfo),
				DescriptorSet::writeBuffer(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &outputReduceInfo),
			};
			commandBuffer.pushDescriptorSet(
				this->reducePipelineLayout,
				0,
				uint32_t(reduceDescriptorSets.size()),
				reduceDescriptorSets.data()
			);

			// Dispatch
			commandBuffer.dispatchIndirect(
				this->indirectDispatchBuffer.getVkBuffer(), 
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
				this->reduceBuffer.getVkBuffer(),
				this->reduceBuffer.getBufferSize()
			);


			// Compute pipeline
			commandBuffer.bindPipeline(this->scanPipeline);

			// Binding 0
			VkDescriptorBufferInfo inputIndirectDispatchInfo{};
			inputIndirectDispatchInfo.buffer = this->indirectDispatchBuffer.getVkBuffer();
			inputIndirectDispatchInfo.range = this->indirectDispatchBuffer.getBufferSize();

			// Binding 1
			VkDescriptorBufferInfo inputOutputReduceInfo{};
			inputOutputReduceInfo.buffer = this->reduceBuffer.getVkBuffer();
			inputOutputReduceInfo.range = this->reduceBuffer.getBufferSize();

			// Descriptor sets
			std::array<VkWriteDescriptorSet, 2> scanDescriptorSets
			{
				DescriptorSet::writeBuffer(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputIndirectDispatchInfo),
				DescriptorSet::writeBuffer(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputOutputReduceInfo)
			};
			commandBuffer.pushDescriptorSet(
				this->scanPipelineLayout,
				0,
				uint32_t(scanDescriptorSets.size()),
				scanDescriptorSets.data()
			);

			// Dispatch
			commandBuffer.dispatch(1);
		}

		// ------------------ 4. Scan add ------------------
		{
			std::array<VkBufferMemoryBarrier2, 2> scanAddWaitMemoryBarriers
			{
				PipelineBarrier::bufferMemoryBarrier2(
					VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
					VK_ACCESS_SHADER_READ_BIT,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					this->reduceBuffer.getVkBuffer(),
					this->reduceBuffer.getBufferSize()
				),

				PipelineBarrier::bufferMemoryBarrier2(
					VK_ACCESS_SHADER_READ_BIT,
					VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					this->sumTableBuffer.getVkBuffer(),
					this->sumTableBuffer.getBufferSize()
				)
			};

			commandBuffer.bufferMemoryBarrier(
				scanAddWaitMemoryBarriers.data(),
				scanAddWaitMemoryBarriers.size()
			);


			// Compute pipeline
			commandBuffer.bindPipeline(this->scanAddPipeline);

			// Binding 0
			VkDescriptorBufferInfo inputIndirectDispatchInfo{};
			inputIndirectDispatchInfo.buffer = this->indirectDispatchBuffer.getVkBuffer();
			inputIndirectDispatchInfo.range = this->indirectDispatchBuffer.getBufferSize();

			// Binding 1
			VkDescriptorBufferInfo inputReduceInfo{};
			inputReduceInfo.buffer = this->reduceBuffer.getVkBuffer();
			inputReduceInfo.range = this->reduceBuffer.getBufferSize();

			// Binding 2
			VkDescriptorBufferInfo inputOutputSumTableInfo{};
			inputOutputSumTableInfo.buffer = this->sumTableBuffer.getVkBuffer();
			inputOutputSumTableInfo.range = this->sumTableBuffer.getBufferSize();

			// Descriptor sets
			std::array<VkWriteDescriptorSet, 3> scanAddDescriptorSets
			{
				DescriptorSet::writeBuffer(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputIndirectDispatchInfo),
				DescriptorSet::writeBuffer(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputReduceInfo),
				DescriptorSet::writeBuffer(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputOutputSumTableInfo)
			};
			commandBuffer.pushDescriptorSet(
				this->scanAddPipelineLayout,
				0,
				uint32_t(scanAddDescriptorSets.size()),
				scanAddDescriptorSets.data()
			);

			// Dispatch
			commandBuffer.dispatchIndirect(
				this->indirectDispatchBuffer.getVkBuffer(), 
				offsetof(RadixIndirectDispatch, reduceSizeX)
			);
		}

		// ------------------ 5. Scatter ------------------
		{
			std::array<VkBufferMemoryBarrier2, 2> scatterWaitMemoryBarriers
			{
				PipelineBarrier::bufferMemoryBarrier2(
					VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
					VK_ACCESS_SHADER_READ_BIT,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					this->sumTableBuffer.getVkBuffer(),
					this->sumTableBuffer.getBufferSize()
				),

				PipelineBarrier::bufferMemoryBarrier2(
					VK_ACCESS_NONE,
					VK_ACCESS_SHADER_WRITE_BIT,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					dstSortBuffer->getVkBuffer(),
					dstSortBuffer->getBufferSize()
				)
			};

			commandBuffer.bufferMemoryBarrier(
				scatterWaitMemoryBarriers.data(),
				scatterWaitMemoryBarriers.size()
			);

			// Compute pipeline
			commandBuffer.bindPipeline(this->scatterPipeline);

			// Binding 0
			VkDescriptorBufferInfo inputIndirectDispatchInfo{};
			inputIndirectDispatchInfo.buffer = this->indirectDispatchBuffer.getVkBuffer();
			inputIndirectDispatchInfo.range = this->indirectDispatchBuffer.getBufferSize();

			// Binding 1
			VkDescriptorBufferInfo inputSumTableInfo{};
			inputSumTableInfo.buffer = this->sumTableBuffer.getVkBuffer();
			inputSumTableInfo.range = this->sumTableBuffer.getBufferSize();

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
				this->scatterPipelineLayout,
				0,
				uint32_t(scatterDescriptorSets.size()),
				scatterDescriptorSets.data()
			);

			// Push constant
			commandBuffer.pushConstant(
				this->scatterPipelineLayout,
				(void*)&sortGaussiansPcData
			);

			// Dispatch
			commandBuffer.dispatchIndirect(
				this->indirectDispatchBuffer.getVkBuffer(), 
				offsetof(RadixIndirectDispatch, countSizeX)
			);

			// No matter which shader reads dst next, the shader should wait for dst
			commandBuffer.bufferMemoryBarrier(
				VK_ACCESS_SHADER_WRITE_BIT,
				VK_ACCESS_SHADER_READ_BIT,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				dstSortBuffer->getVkBuffer(),
				dstSortBuffer->getBufferSize()
			);

			// Check if there will be a next iteration of radix sort passes
			if (shiftBits + RS_BITS_PER_PASS < this->radixSortNumSortBits)
			{
				commandBuffer.bufferMemoryBarrier(
					VK_ACCESS_SHADER_READ_BIT,
					VK_ACCESS_SHADER_WRITE_BIT,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					this->sumTableBuffer.getVkBuffer(),
					this->sumTableBuffer.getBufferSize()
				);
			}

			// Swap
			StorageBuffer* temp = srcSortBuffer;
			srcSortBuffer = dstSortBuffer;
			dstSortBuffer = temp;
		}

		// Swap if original sort list is not pointing to correct buffer
		if ((this->radixSortNumSortBits / RS_BITS_PER_PASS) % 2 != 0)
		{
			// Swap
			this->tempSwapPingPongBuffer = gaussiansSortListSBO;
			gaussiansSortListSBO = this->pingPongBuffer;
			this->pingPongBuffer = this->tempSwapPingPongBuffer;
		}
	}
}

void RadixSort::cleanup()
{
	this->pingPongBuffer->cleanup();
	this->reduceBuffer.cleanup();
	this->sumTableBuffer.cleanup();
	this->indirectDispatchBuffer.cleanup();

	this->scatterPipelineLayout.cleanup();
	this->scatterPipeline.cleanup();
	this->scanAddPipelineLayout.cleanup();
	this->scanAddPipeline.cleanup();
	this->scanPipelineLayout.cleanup();
	this->scanPipeline.cleanup();
	this->reducePipelineLayout.cleanup();
	this->reducePipeline.cleanup();
	this->countPipelineLayout.cleanup();
	this->countPipeline.cleanup();
	this->indirectSetupPipelineLayout.cleanup();
	this->indirectSetupPipeline.cleanup();
}

void RadixSort::gpuClearBuffers(CommandBuffer& commandBuffer)
{
	// Reset radix sort ping pong buffer, similar to gaussian sort keys
	commandBuffer.fillBuffer(
		this->pingPongBuffer->getVkBuffer(),
		sizeof(GaussianSortData) * this->maxNumSortElements,
		std::numeric_limits<uint32_t>::max()
	);

	commandBuffer.bufferMemoryBarrier(
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		this->pingPongBuffer->getVkBuffer(),
		this->pingPongBuffer->getBufferSize()
	);
}
#include "pch.h"
#include "BitonicMergeSort.h"
#include "../Buffer/StorageBuffer.h"

void BitonicMergeSort::dispatchBms(
	CommandBuffer& commandBuffer,
	BmsSubAlgorithm subAlgorithm,
	uint32_t h,
	uint32_t numElemToSort,
	std::shared_ptr<StorageBuffer>& gaussiansSortListSBO)
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
		gaussiansSortListSBO->getVkBuffer(),
		gaussiansSortListSBO->getBufferSize()
	);
}

void BitonicMergeSort::singleInitResources(const GfxAllocContext& allocContext)
{
	this->sortGaussiansBmsPipelineLayout.createPipelineLayout(
		*allocContext.device,
		{
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT }
		},
		VK_SHADER_STAGE_COMPUTE_BIT,
		sizeof(SortGaussiansBmsPCD)
	);
	this->sortGaussiansBmsPipeline.createComputePipeline(
		*allocContext.device,
		this->sortGaussiansBmsPipelineLayout,
		"Resources/Shaders/BitonicMergeSort.comp.spv",
		{
			SpecializationConstant{ (void*) BMS_WORK_GROUP_SIZE, sizeof(uint32_t)}
		}
	);
}

void BitonicMergeSort::initForScene(uint32_t maxNumSortElements, uint32_t numTiles)
{
	this->maxNumSortElements = maxNumSortElements;
}

void BitonicMergeSort::computeSort(
	CommandBuffer& commandBuffer,
	StorageBuffer& gaussiansCullDataSBO,
	std::shared_ptr<StorageBuffer>& gaussiansSortListSBO)
{
	// Make sure number of elements is power of two
	assert((uint32_t)(this->maxNumSortElements & (this->maxNumSortElements - 1)) == 0u);

	// Make sure number of elements is large enough
	assert(this->maxNumSortElements >= BMS_WORK_GROUP_SIZE * 2);

	// Wait for work on initialization of the sorting list to finish
	commandBuffer.bufferMemoryBarrier(
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		gaussiansSortListSBO->getVkBuffer(),
		gaussiansSortListSBO->getBufferSize()
	);

	// Compute pipeline
	commandBuffer.bindPipeline(this->sortGaussiansBmsPipeline);

	// Binding 0
	VkDescriptorBufferInfo inputOutputGaussiansSortInfo{};
	inputOutputGaussiansSortInfo.buffer = gaussiansSortListSBO->getVkBuffer();
	inputOutputGaussiansSortInfo.range = gaussiansSortListSBO->getBufferSize();

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
		this->maxNumSortElements,
		gaussiansSortListSBO
	);

	h *= 2;

	for (; h <= this->maxNumSortElements; h *= 2)
	{
		this->dispatchBms(
			commandBuffer,
			BmsSubAlgorithm::BIG_FLIP,
			h,
			this->maxNumSortElements,
			gaussiansSortListSBO
		);

		for (uint32_t hh = h / 2; hh > 1; hh /= 2)
		{
			if (hh <= BMS_WORK_GROUP_SIZE * 2)
			{
				this->dispatchBms(
					commandBuffer,
					BmsSubAlgorithm::LOCAL_DISPERSE,
					hh,
					this->maxNumSortElements,
					gaussiansSortListSBO
				);
				break;
			}
			else
			{
				this->dispatchBms(
					commandBuffer,
					BmsSubAlgorithm::BIG_DISPERSE,
					hh,
					this->maxNumSortElements,
					gaussiansSortListSBO
				);
			}
		}
	}
}

void BitonicMergeSort::cleanup()
{
	this->sortGaussiansBmsPipeline.cleanup();
	this->sortGaussiansBmsPipelineLayout.cleanup();
}

void BitonicMergeSort::gpuClearBuffers(CommandBuffer& commandBuffer)
{
	
}
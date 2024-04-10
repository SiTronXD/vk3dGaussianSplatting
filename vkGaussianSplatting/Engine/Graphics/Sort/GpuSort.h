#pragma once

#define BITONIC_MERGE_SORT 0
#define RADIX_SORT 1

class StorageBuffer;

class GpuSort
{
private:

public:
	virtual void singleInitResources(const GfxAllocContext& allocContext) = 0;
	virtual void initForScene(uint32_t maxNumSortElements, uint32_t numTiles) = 0;
	virtual void computeSort(
		CommandBuffer& commandBuffer,
		StorageBuffer& gaussiansCullDataSBO,
		std::shared_ptr<StorageBuffer>& gaussiansSortListSBO) = 0;
	virtual void cleanup() = 0;

	virtual void gpuClearBuffers(CommandBuffer& commandBuffer) = 0;
};
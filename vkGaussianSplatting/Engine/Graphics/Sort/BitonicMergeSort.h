#pragma once

#include "GpuSort.h"

class StorageBuffer;

class BitonicMergeSort : public GpuSort
{
private:
	const static uint32_t BMS_WORK_GROUP_SIZE = 512;

	enum class BmsSubAlgorithm
	{
		LOCAL_BMS = 0,
		LOCAL_DISPERSE = 1,
		BIG_FLIP = 2,
		BIG_DISPERSE = 3
	};

	PipelineLayout sortGaussiansBmsPipelineLayout;
	Pipeline sortGaussiansBmsPipeline;

	uint32_t maxNumSortElements;

	void dispatchBms(
		CommandBuffer& commandBuffer, 
		BmsSubAlgorithm subAlgorithm, 
		uint32_t h, 
		uint32_t numElemToSort,
		std::shared_ptr<StorageBuffer>& gaussiansSortListSBO);

public:
	virtual void singleInitResources(const GfxAllocContext& allocContext) override;
	virtual void initForScene(uint32_t maxNumSortElements, uint32_t numTiles) override;
	virtual void computeSort(
		CommandBuffer& commandBuffer,
		StorageBuffer& gaussiansCullDataSBO,
		std::shared_ptr<StorageBuffer>& gaussiansSortListSBO) override;
	virtual void cleanup() override;

	virtual void gpuClearBuffers(CommandBuffer& commandBuffer) override;
};
#pragma once

#include "GpuSort.h"
#include "../Buffer/StorageBuffer.h"

class RadixSort : public GpuSort
{
private:
	PipelineLayout indirectSetupPipelineLayout;
	Pipeline indirectSetupPipeline;
	PipelineLayout countPipelineLayout;
	Pipeline countPipeline;
	PipelineLayout reducePipelineLayout;
	Pipeline reducePipeline;
	PipelineLayout scanPipelineLayout;
	Pipeline scanPipeline;
	PipelineLayout scanAddPipelineLayout;
	Pipeline scanAddPipeline;
	PipelineLayout scatterPipelineLayout;
	Pipeline scatterPipeline;

	StorageBuffer indirectDispatchBuffer;
	StorageBuffer sumTableBuffer;
	StorageBuffer reduceBuffer;
	std::shared_ptr<StorageBuffer> pingPongBuffer;
	std::shared_ptr<StorageBuffer> tempSwapPingPongBuffer;

	const GfxAllocContext* gfxAllocContext;

	uint32_t maxNumSortElements;
	uint32_t radixSortNumSortBits;

	uint32_t getMinNumBits(uint32_t x) const;

public:
	const static uint32_t RS_BITS_PER_PASS = 4;
	const static uint32_t RS_BIN_COUNT = 1u << RS_BITS_PER_PASS;
	const static uint32_t RS_WORK_GROUP_SIZE = 64; // 128 saves 0.5 ms on sorting, but seems to worsen rendering timings by 1 ms (before rendering optimization).
	const static uint32_t RS_SCAN_WORK_GROUP_SIZE = 1024;

	RadixSort();

	virtual void singleInitResources(const GfxAllocContext& allocContext) override;
	virtual void initForScene(uint32_t maxNumSortElements, uint32_t numTiles) override;
	virtual void computeSort(
		CommandBuffer& commandBuffer,
		StorageBuffer& gaussiansCullDataSBO,
		std::shared_ptr<StorageBuffer>& gaussiansSortListSBO) override;
	virtual void cleanup() override;

	virtual void gpuClearBuffers(CommandBuffer& commandBuffer) override;
};
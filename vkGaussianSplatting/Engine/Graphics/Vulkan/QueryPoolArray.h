#pragma once

class QueryPoolArray
{
private:
	std::vector<VkQueryPool> queryPools;

	// queryResults[poolIndex][resultIndex]
	std::vector<std::vector<uint64_t>> queryResults;

	Device* device;

	uint32_t queryCount;

public:
	QueryPoolArray();

	void create(Device& device, uint32_t numQueryPools, uint32_t queryCount);
	void getQueryPoolResults(uint32_t queryPoolIndex);
	void cleanup();

	inline uint32_t getQueryCount() const { return this->queryCount; }

	inline uint64_t getQueryResult(uint32_t queryPoolIndex, uint32_t index) const { return this->queryResults[queryPoolIndex][index]; }

	inline const VkQueryPool& operator[](uint32_t index) const { return this->queryPools[index]; }
};
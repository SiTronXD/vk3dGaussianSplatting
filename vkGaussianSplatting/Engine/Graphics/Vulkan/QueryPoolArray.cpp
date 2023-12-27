#include "pch.h"
#include "QueryPoolArray.h"

QueryPoolArray::QueryPoolArray()
	: device(nullptr),
	queryCount(0)
{
}

void QueryPoolArray::create(Device& device, uint32_t numQueryPools, uint32_t queryCount)
{
	this->device = &device;
	this->queryCount = queryCount;

	// Create info
	VkQueryPoolCreateInfo queryPoolCreateInfo{ VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO };
	queryPoolCreateInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
	queryPoolCreateInfo.queryCount = this->queryCount;

	// Create query pools
	this->queryPools.resize(numQueryPools);
	this->queryResults.resize(numQueryPools);
	for (uint32_t i = 0; i < numQueryPools; ++i)
	{
		if (vkCreateQueryPool(
			this->device->getVkDevice(),
			&queryPoolCreateInfo,
			nullptr,
			&this->queryPools[i]) != VK_SUCCESS)
		{
			Log::error("Failed to create query pool with index " + std::to_string(i) + ".");
		}

		// Preallocate query results
		this->queryResults[i].resize(this->queryCount);
	}
}


void QueryPoolArray::getQueryPoolResults(uint32_t queryPoolIndex)
{
	uint32_t resultRlementSize = sizeof(this->queryResults[0][0]);
	uint32_t dataSize = this->queryCount * resultRlementSize;
	vkGetQueryPoolResults(
		this->device->getVkDevice(),
		this->queryPools[queryPoolIndex],
		0,
		this->queryCount,
		dataSize,
		this->queryResults[queryPoolIndex].data(),
		resultRlementSize,
		VK_QUERY_RESULT_64_BIT
	);
}

void QueryPoolArray::cleanup()
{
	for (size_t i = 0; i < this->queryPools.size(); ++i)
		vkDestroyQueryPool(this->device->getVkDevice(), this->queryPools[i], nullptr);
	this->queryPools.clear();
	this->queryResults.clear();
}

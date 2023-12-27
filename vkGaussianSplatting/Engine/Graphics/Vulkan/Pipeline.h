#pragma once

#include "PipelineLayout.h"

class Device;

class Pipeline
{
private:
	VkPipeline pipeline;
	VkPipelineCache pipelineCache;
	VkPipelineBindPoint bindPoint;

	const Device* device;

public:
	Pipeline();
	~Pipeline();

	void createPipelineCache(const Device& device);
	void createGraphicsPipeline(
		const Device& device,
		PipelineLayout& pipelineLayout,
		const std::vector<VkFormat>& colorFormats,
		VkFormat depthFormat,
		const std::string& vertexShader,
		const std::string& fragmentShader);
	void createComputePipeline(
		const Device& device,
		PipelineLayout& pipelineLayout,
		const std::string& computeShader);
	void createImguiPipeline(
		const Device& device,
		PipelineLayout& imguiPipelineLayout,
		VkFormat colorFormat,
		VkFormat depthFormat);

	void cleanup();
	void cleanupPipelineCache();

	inline const VkPipeline& getVkPipeline() const { return this->pipeline; }
	inline VkPipelineBindPoint getVkBindPoint() const { return this->bindPoint; }
};
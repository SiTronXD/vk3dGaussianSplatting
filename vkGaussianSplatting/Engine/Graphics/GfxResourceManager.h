#pragma once

#include <unordered_map>

struct Material;

class GfxResourceManager
{
private:
	std::unordered_map<std::string, uint32_t> materialToRsmPipeline;
	std::unordered_map<std::string, uint32_t> materialToShadowMapPipeline;
	std::unordered_map<std::string, uint32_t> materialToPipeline;
	std::vector<Pipeline> pipelines;

	PipelineLayout graphicsPipelineLayout;

	const GfxAllocContext* gfxAllocContext;

public:
	GfxResourceManager();

	void init(const GfxAllocContext& gfxAllocContext);
	void cleanup();

	uint32_t getMaterialRsmPipelineIndex(const Material& material);
	uint32_t getMaterialShadowMapPipelineIndex(const Material& material);
	uint32_t getMaterialPipelineIndex(const Material& material);

	inline const PipelineLayout& getGraphicsPipelineLayout() const { return this->graphicsPipelineLayout; }
	inline const Pipeline& getPipeline(uint32_t index) const { return this->pipelines[index]; }

	inline const Pipeline& getOneShadowMapPipeline() const { return this->pipelines[this->materialToShadowMapPipeline.begin()->second]; }
	inline const Pipeline& getOneHdrPipeline() const { return this->pipelines[this->materialToPipeline.begin()->second]; }
};
#pragma once

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>
#include <vk_mem_alloc.h>

#include "../Application/Window.h"
#include "../Application/Scene.h"
#include "Vulkan/VulkanInstance.h"
#include "Vulkan/DebugMessenger.h"
#include "Vulkan/Surface.h"
#include "Vulkan/PhysicalDevice.h"
#include "Vulkan/DescriptorSetLayout.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/CommandBufferArray.h"
#include "Vulkan/QueryPoolArray.h"
#include "Vulkan/SemaphoreArray.h"
#include "Vulkan/FenceArray.h"
#include "Buffer/UniformBuffer.h"
#include "Buffer/StorageBuffer.h"
#include "Texture/Texture.h"
#include "Swapchain.h"
#include "Camera.h"
#include "Mesh.h"
#include "GfxAllocContext.h"

// For imgui
#include "Vulkan/Legacy/DescriptorPool.h"
#include "Vulkan/Legacy/RenderPass.h"

class ResourceManager;
class DescriptorPool;

enum class BmsSubAlgorithm
{
	LOCAL_BMS = 0,
	LOCAL_DISPERSE = 1,
	BIG_FLIP = 2,
	BIG_DISPERSE = 3
};

#define RECORD_GPU_TIMES
//#define RECORD_CPU_TIMES
#define ALERT_FINAL_AVERAGE

class Renderer
{
private:
	VulkanInstance instance;
	DebugMessenger debugMessenger;
	Surface surface;
	PhysicalDevice physicalDevice;
	Device device;
	QueueFamilies queueFamilies;
	Swapchain swapchain;
	VmaAllocator vmaAllocator;

	GfxAllocContext gfxAllocContext{};

	// Timestamp queries
#ifdef RECORD_GPU_TIMES
	const uint32_t MAX_QUERY_COUNT = 9;
	QueryPoolArray queryPools;
	float elapsedFrames;
	float avgInitSortListMs;
	float avgSortMs;
	float avgFindRangesMs;
	float avgRenderGaussiansMs;
	float avgTotalGpuTimeMs;
#endif

#ifdef RECORD_CPU_TIMES
	float elapsedFrames;
	float avgWaitForFenceMs;
	float avgRecordCommandBufferMs;
	float avgPresentMs;
	float avgCpuFrameTimeMs;
#endif

#if defined(RECORD_GPU_TIMES) && defined(RECORD_CPU_TIMES)
	THIS_IS_NOT_ALLOWED___MAKE_A_COMPILE_ERROR
#endif

	// Pipelines/layouts
	PipelineLayout initSortListPipelineLayout;
	Pipeline initSortListPipeline;
	PipelineLayout findRangesPipelineLayout;
	Pipeline findRangesPipeline;
	PipelineLayout renderGaussiansPipelineLayout;
	Pipeline renderGaussiansPipeline;

	// Pipeline/layouts for bitonic merge sort
	PipelineLayout sortGaussiansBmsPipelineLayout;
	Pipeline sortGaussiansBmsPipeline;

	// Pipeline/layouts for radix sort
	PipelineLayout radixSortCountPipelineLayout;
	Pipeline radixSortCountPipeline;
	PipelineLayout radixSortReducePipelineLayout;
	Pipeline radixSortReducePipeline;
	PipelineLayout radixSortScanPipelineLayout;
	Pipeline radixSortScanPipeline;
	PipelineLayout radixSortScanAddPipelineLayout;
	Pipeline radixSortScanAddPipeline;
	PipelineLayout radixSortScatterPipelineLayout;
	Pipeline radixSortScatterPipeline;

	CommandPool commandPool;
	CommandPool singleTimeCommandPool;
	CommandBufferArray commandBuffers;

	// Imgui
	PipelineLayout imguiPipelineLayout;
	Pipeline imguiPipeline;
	DescriptorPool imguiDescriptorPool;
	ImGuiIO* imguiIO;

	SemaphoreArray imageAvailableSemaphores;
	SemaphoreArray renderGaussiansFinishedSemaphores;
	FenceArray inFlightFences;

	UniformBuffer camUBO;
	StorageBuffer gaussiansSBO;
	StorageBuffer gaussiansCullDataSBO;
	StorageBuffer gaussiansTileRangesSBO;
	std::shared_ptr<StorageBuffer> gaussiansSortListSBO;

	// Buffers for radix sort
	StorageBuffer radixSortSumTableBuffer;
	StorageBuffer radixSortReduceBuffer;
	std::shared_ptr<StorageBuffer> radixSortPingPongBuffer;
	std::shared_ptr<StorageBuffer> tempSwapPingPongBuffer;

	uint32_t numGaussians;
	uint32_t numSortElements;
	uint32_t radixSortNumSortBits;

	Window* window;
	ResourceManager* resourceManager;

	void initVulkan();
	void initVma();
	void initImgui();

	void createCamUbo();
	void createSyncObjects();

	void updateUniformBuffer(const Camera& camera);

	void recordCommandBuffer(
		uint32_t imageIndex, 
		Scene& scene);

	void resizeWindow();
	void cleanupImgui();

	void computeSortGaussiansBMS(CommandBuffer& commandBuffer, uint32_t numElemToSort);
	void computeSortGaussiansRS(CommandBuffer& commandBuffer, uint32_t numElemToSort);

	void renderImgui(CommandBuffer& commandBuffer, ImDrawData* imguiDrawData, uint32_t imageIndex);
	void computeInitSortList(CommandBuffer& commandBuffer, const Camera& camera);
	void computeSortGaussians(CommandBuffer& commandBuffer, uint32_t numElemToSort);
	void computeRanges(CommandBuffer& commandBuffer);
	void computeRenderGaussians(CommandBuffer& commandBuffer, uint32_t imageIndex);

	void dispatchBms(CommandBuffer& commandBuffer, BmsSubAlgorithm subAlgorithm, uint32_t h, uint32_t numElemToSort);

	inline float getNewAvgTime(float avgValue, float newValue, float t) const { return (1.0f - t)* avgValue + t * newValue; }

	uint32_t getNumTiles() const;
	uint32_t getCeilPowTwo(uint32_t x) const;
	uint32_t getMinNumBits(uint32_t x) const;

	inline const VkDevice& getVkDevice() const { return this->device.getVkDevice(); }

public:
	const static uint32_t WAIT_ELAPSED_WARMUP_FRAMES_FOR_AVG = 1000;
	const static uint32_t WAIT_ELAPSED_FRAMES_FOR_AVG = 500;

	const static uint32_t INIT_LIST_WORK_GROUP_SIZE = 32;
	const static uint32_t BMS_WORK_GROUP_SIZE = 512;

	const static uint32_t RS_BITS_PER_PASS = 4;
	const static uint32_t RS_BIN_COUNT = 1u << RS_BITS_PER_PASS;
	const static uint32_t RS_WORK_GROUP_SIZE = 64;

	const static uint32_t TILE_SIZE = 16;
	const static uint32_t FIND_RANGES_GROUP_SIZE = 16;

	bool framebufferResized = false;

	Renderer();
	~Renderer();

	void init(ResourceManager& resourceManager);
	void initForScene(Scene& scene);
	void setWindow(Window& window);

	void startCleanup();
	void cleanup();

	void draw(Scene& scene);

	void generateMemoryDump();

	inline float getSwapchainAspectRatio() 
		{ return (float) this->swapchain.getWidth() / this->swapchain.getHeight(); }
	inline const GfxAllocContext& getGfxAllocContext() const { return this->gfxAllocContext; }
};

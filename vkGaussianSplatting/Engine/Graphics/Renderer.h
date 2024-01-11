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
#include "BRDFData.h"

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

class Renderer
{
private:
	const float WAIT_ELAPSED_FRAMES_FOR_AVG = 500.0f;

	const uint32_t INIT_LIST_WORK_GROUP_SIZE = 32;
	const uint32_t BMS_WORK_GROUP_SIZE = 8;

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
	QueryPoolArray queryPools;
	float elapsedFrames;
	float avgRsmMs;
	float avgSmMs;
	float avgDeferredGeomMs;
	float avgDeferredLightMs;
	float avgGpuFrameTimeMs;
#endif

#ifdef RECORD_CPU_TIMES
	float elapsedFrames;
	float avgWaitForFenceMs;
	float avgRecordCommandBufferMs;
	float avgPresentMs;
	float avgCpuFrameTimeMs;
#endif

	// Pipelines and layouts
	PipelineLayout initSortListPipelineLayout;
	Pipeline initSortListPipeline;
	PipelineLayout sortGaussiansPipelineLayout;
	Pipeline sortGaussiansPipeline;
	PipelineLayout renderGaussiansPipelineLayout;
	Pipeline renderGaussiansPipeline;

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

	UniformBuffer gaussiansCamUBO;
	StorageBuffer gaussiansSBO;
	StorageBuffer gaussiansSortListSBO;

	uint32_t numGaussians;

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

	void renderImgui(CommandBuffer& commandBuffer, ImDrawData* imguiDrawData, uint32_t imageIndex);
	void computeInitSortList(CommandBuffer& commandBuffer, const Camera& camera);
	void computeSortGaussians(CommandBuffer& commandBuffer);
	void computeRenderGaussians(CommandBuffer& commandBuffer, uint32_t imageIndex);

	void dispatchBms(CommandBuffer& commandBuffer, BmsSubAlgorithm subAlgorithm, uint32_t h);

	inline const VkDevice& getVkDevice() const { return this->device.getVkDevice(); }

public:
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
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
#include "GfxResourceManager.h"
#include "RSM.h"
#include "BRDFData.h"

// For imgui
#include "Vulkan/Legacy/DescriptorPool.h"
#include "Vulkan/Legacy/RenderPass.h"

class ResourceManager;
class DescriptorPool;

class Renderer
{
private:
	const float WAIT_ELAPSED_FRAMES_FOR_AVG = 500.0f;

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

	// Compute
	PipelineLayout deferredLightPipelineLayout;
	Pipeline deferredLightPipeline;
	PipelineLayout postProcessPipelineLayout;
	Pipeline postProcessPipeline;

	CommandPool commandPool;
	CommandPool singleTimeCommandPool;
	CommandBufferArray commandBuffers;

	// Imgui
	PipelineLayout imguiPipelineLayout;
	Pipeline imguiPipeline;
	DescriptorPool imguiDescriptorPool;
	ImGuiIO* imguiIO;

	SemaphoreArray imageAvailableSemaphores;
	SemaphoreArray postProcessFinishedSemaphores;
	FenceArray inFlightFences;

	UniformBuffer uniformBuffer;
	StorageBuffer shCoefficientBuffer;

	Window* window;
	ResourceManager* resourceManager;

	GfxResourceManager gfxResManager;

	// Reflective shadow map
	RSM rsm;

	uint32_t skyboxTextureIndex;

	void initVulkan();
	void initVma();
	void initImgui();

	void createCamUbo();
	void addShCoefficients(
		const std::vector<std::vector<RGB>>& shCoeffs, 
		std::vector<SHData>& outputShSets);
	void createShCoefficientBuffer(Scene& scene);
	void createSyncObjects();

	void updateUniformBuffer(const Camera& camera);

	void recordCommandBuffer(
		uint32_t imageIndex, 
		Scene& scene);

	void resizeWindow();
	void cleanupImgui();

	void renderMesh(CommandBuffer& commandBuffer, const Mesh& mesh);
	void renderMeshWithBrdf(
		CommandBuffer& commandBuffer, 
		const Mesh& mesh, 
		const Material& material,
		PCD& pushConstantData);
	void renderRSM(CommandBuffer& commandBuffer, Scene& scene);
	void renderShadowMap(CommandBuffer& commandBuffer, Scene& scene);
	void renderDeferredScene(CommandBuffer& commandBuffer, Scene& scene);
	void computeDeferredLight(CommandBuffer& commandBuffer, const glm::vec3& camPos);
	void renderImgui(CommandBuffer& commandBuffer, ImDrawData* imguiDrawData);
	void computePostProcess(CommandBuffer& commandBuffer, uint32_t imageIndex);

	inline const VkDevice& getVkDevice() const { return this->device.getVkDevice(); }

public:
	bool framebufferResized = false;

	Renderer();
	~Renderer();

	void init(ResourceManager& resourceManager);
	void initForScene(Scene& scene);
	void setWindow(Window& window);

	void setSpotlightOrientation(const glm::vec3& position, const glm::vec3& lookAtPosition);

	void startCleanup();
	void cleanup();

	void draw(Scene& scene);

	void generateMemoryDump();

	void setSkyboxTexture(uint32_t skyboxTextureIndex);

	inline float getSwapchainAspectRatio() 
		{ return (float) this->swapchain.getWidth() / this->swapchain.getHeight(); }
	inline const GfxAllocContext& getGfxAllocContext() const { return this->gfxAllocContext; }
};
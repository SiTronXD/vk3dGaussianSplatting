#include "pch.h"
#include "Renderer.h"
#include "../ResourceManager.h"
#include "Vulkan/PipelineBarrier.h"
#include "Vulkan/DescriptorSet.h"
#include "Texture/TextureCube.h"
#include "../Dev/StrHelper.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

const std::vector<const char*> validationLayers =
{
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> instanceExtensions =
{
#ifdef _DEBUG
	"VK_EXT_validation_features"
#endif
};

const std::vector<const char*> deviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

void Renderer::initVulkan()
{
	// Gfx alloc context
	this->gfxAllocContext.device = &this->device;
	this->gfxAllocContext.queueFamilies = &this->queueFamilies;
	this->gfxAllocContext.singleTimeCommandPool = &this->singleTimeCommandPool;
	this->gfxAllocContext.vmaAllocator = &this->vmaAllocator;

	// Print if validation layers are enabled
	if (enableValidationLayers)
	{
		Log::write("Validation layers are enabled!");
	}

	this->instance.createInstance(
		enableValidationLayers,
		instanceExtensions,
		validationLayers, 
		this->window
	);
	this->debugMessenger.createDebugMessenger(this->instance, enableValidationLayers);
	this->surface.createSurface(this->instance, *this->window);
	this->physicalDevice.pickPhysicalDevice(
		this->instance,
		this->surface,
		deviceExtensions, 
		this->queueFamilies
	);
	this->device.createDevice(
		this->physicalDevice,
		deviceExtensions, 
		validationLayers, 
		enableValidationLayers, 
		this->queueFamilies.getIndices()
	);
	this->initVma();
	this->queueFamilies.extractQueueHandles(this->getVkDevice());
	this->swapchain.createSwapchain(this->surface, this->gfxAllocContext, *this->window, this->queueFamilies);
	
	this->commandPool.create(this->device, this->queueFamilies, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	this->singleTimeCommandPool.create(this->device, this->queueFamilies, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
	this->commandBuffers.createCommandBuffers(this->device, this->commandPool, GfxSettings::FRAMES_IN_FLIGHT);
	this->swapchain.createFramebuffers();

#ifdef RECORD_GPU_TIMES
	this->queryPools.create(this->device, GfxSettings::FRAMES_IN_FLIGHT, 9);
#endif

	// Deferred light compute pipelines
	this->deferredLightPipelineLayout.createPipelineLayout(
		this->device,
		{
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT },

			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },

			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT }
		},
		VK_SHADER_STAGE_COMPUTE_BIT,
		sizeof(DeferredLightPCD)
	);
	this->deferredLightPipeline.createComputePipeline(
		this->device,
		this->deferredLightPipelineLayout,
		"Resources/Shaders/DeferredLight.comp.spv"
	);

	// Post process compute pipelines
	this->postProcessPipelineLayout.createPipelineLayout(
		this->device,
		{
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT },
		},
		VK_SHADER_STAGE_COMPUTE_BIT,
		sizeof(PostProcessPCD)
	);
	this->postProcessPipeline.createComputePipeline(
		this->device, 
		this->postProcessPipelineLayout, 
		"Resources/Shaders/PostProcess.comp.spv"
	);

	this->createSyncObjects();
	this->createCamUbo();

	this->gfxResManager.init(this->gfxAllocContext);

	this->rsm.init(this->gfxAllocContext);
}

void Renderer::initVma()
{
	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.vulkanApiVersion = this->instance.getApiVersion();
	allocatorInfo.instance = this->instance.getVkInstance();
	allocatorInfo.physicalDevice = this->physicalDevice.getVkPhysicalDevice();
	allocatorInfo.device = this->device.getVkDevice();
	allocatorInfo.preferredLargeHeapBlockSize = 16 * 1024 * 1024;
	if (vmaCreateAllocator(&allocatorInfo, &this->vmaAllocator) != VK_SUCCESS)
	{
		Log::error("Failed to create VMA allocator.");
	}
}

static void checkVkResult(VkResult err)
{
	if (err == 0)
		return;

	Log::error("Vulkan error: " + std::to_string(err));
}

void Renderer::initImgui()
{
	// Vulkan objects interacting with imgui
	this->imguiPipelineLayout.createImguiPipelineLayout(this->device);
	this->imguiPipeline.createImguiPipeline(
		this->device,
		this->imguiPipelineLayout,
		this->swapchain.getHdrTexture().getVkFormat(),
		Texture::getDepthBufferFormat()
	);

	//this->imguiRenderPass.createImguiRenderPass(this->device, this->swapchain.getVkFormat());
	this->imguiDescriptorPool.createImguiDescriptorPool(this->device, GfxSettings::FRAMES_IN_FLIGHT);

	// Imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	this->imguiIO = &ImGui::GetIO(); (void)this->imguiIO;
	this->imguiIO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	this->imguiIO->ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	this->imguiIO->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

	ImGui::StyleColorsDark();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (this->imguiIO->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	ImGui_ImplGlfw_InitForVulkan(this->window->getWindowHandle(), true);
	ImGui_ImplVulkan_InitInfo initInfo = {};
	initInfo.Instance = this->instance.getVkInstance();
	initInfo.PhysicalDevice = this->physicalDevice.getVkPhysicalDevice();
	initInfo.Device = this->getVkDevice();
	initInfo.QueueFamily = this->queueFamilies.getIndices().graphicsFamily.value();
	initInfo.Queue = this->queueFamilies.getVkGraphicsQueue();
	initInfo.PipelineCache = VK_NULL_HANDLE;
	initInfo.DescriptorPool = this->imguiDescriptorPool.getVkDescriptorPool();
	initInfo.Allocator = nullptr;
	initInfo.MinImageCount = GfxSettings::FRAMES_IN_FLIGHT;
	initInfo.ImageCount = GfxSettings::FRAMES_IN_FLIGHT; // Imgui seems to use the name "ImageCount" when the parameter is instead used as frames in flight.
	initInfo.CheckVkResultFn = checkVkResult;

	// Temporary render pass object to init imgui backend
	RenderPass tempImguiRenderPass;
	tempImguiRenderPass.createImguiRenderPass(this->device, this->swapchain.getVkFormat());

	// Init imgui vulkan backend
	ImGui_ImplVulkan_Init(&initInfo, tempImguiRenderPass.getVkRenderPass());
	tempImguiRenderPass.cleanup();

	// Upload font to gpu
	CommandBuffer tempCommandBuffer;
	tempCommandBuffer.beginSingleTimeUse(this->gfxAllocContext);
	ImGui_ImplVulkan_CreateFontsTexture(tempCommandBuffer.getVkCommandBuffer());
	tempCommandBuffer.endSingleTimeUse(this->gfxAllocContext);
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void Renderer::cleanup()
{
	this->cleanupImgui();

	this->rsm.cleanup();

	this->gfxResManager.cleanup();

	this->swapchain.cleanup();

	this->shCoefficientBuffer.cleanup();
	this->uniformBuffer.cleanup();

	this->imageAvailableSemaphores.cleanup();
	this->postProcessFinishedSemaphores.cleanup();
	this->inFlightFences.cleanup();

#ifdef RECORD_GPU_TIMES
	this->queryPools.cleanup();
#endif

	this->singleTimeCommandPool.cleanup();
	this->commandPool.cleanup();
	this->postProcessPipeline.cleanup();
	this->postProcessPipelineLayout.cleanup();
	this->deferredLightPipeline.cleanup();
	this->deferredLightPipelineLayout.cleanup();
	
	vmaDestroyAllocator(this->vmaAllocator);
	
	this->device.cleanup();
	this->debugMessenger.cleanup();
	this->surface.cleanup();

	// Destroys both physical device and instance
	this->instance.cleanup();
}

void Renderer::setSpotlightOrientation(const glm::vec3& position, const glm::vec3& forwardDir)
{
	this->rsm.setOrientation(position, forwardDir);
}

void Renderer::createCamUbo()
{
	this->uniformBuffer.createDynamicCpuBuffer(
		this->gfxAllocContext,
		sizeof(CamUBO)
	);
}

void Renderer::addShCoefficients(
	const std::vector<std::vector<RGB>>& shCoeffs, 
	std::vector<SHData>& outputShSets)
{
	// For each angle
	for (size_t j = 0; j < shCoeffs.size(); ++j)
	{
		// For each coefficient
		SHData newShData{};
		for (size_t k = 0; k < shCoeffs[j].size(); ++k)
		{
			for (uint32_t n = 0; n < 3; ++n)
			{
				newShData.coefficients[k * 3 + n] = shCoeffs[j][k].rgb[n]; // R0 G0 B0 R1 G1 B1 R2 G2 B2
			}
		}

		outputShSets.push_back(newShData);
	}
}

void Renderer::createShCoefficientBuffer(Scene& scene)
{
	std::vector<SHData> shSets;

	if (this->resourceManager->getNumBRDFs() == 0)
	{
		Log::error("This scene does not contain any materials.");
	}

	// For each BRDF data within a material
	for (size_t i = 0; i < this->resourceManager->getNumBRDFs(); ++i)
	{
		// R0, G0, B0, R1, G1, B1, RC0, GC0, BC0, RC1, GC1, BC1, ...
		this->addShCoefficients(
			this->resourceManager->getBRDFData(i).getShCoefficientSets(), 
			shSets
		);
		this->addShCoefficients(
			this->resourceManager->getBRDFData(i).getShCoefficientCosSets(),
			shSets
		);
	}

	// Create sh coefficient buffer
	VkDeviceSize bufferSize = sizeof(shSets[0]) * shSets.size();
	this->shCoefficientBuffer.createStaticGpuBuffer(
		this->gfxAllocContext,
		bufferSize,
		shSets.data()
	);
}

void Renderer::createSyncObjects()
{
	this->imageAvailableSemaphores.create(
		this->device, 
		GfxSettings::FRAMES_IN_FLIGHT
	);
	this->postProcessFinishedSemaphores.create(
		this->device,
		GfxSettings::FRAMES_IN_FLIGHT
	);
	this->inFlightFences.create(
		this->device,
		GfxSettings::FRAMES_IN_FLIGHT,
		VK_FENCE_CREATE_SIGNALED_BIT
	);
}

void Renderer::draw(Scene& scene)
{
#ifdef RECORD_CPU_TIMES
	Time::startTimer();
#endif

	// Wait, then reset fence
	vkWaitForFences(
		this->getVkDevice(), 
		1, 
		&this->inFlightFences[GfxState::getFrameIndex()],
		VK_TRUE, 
		UINT64_MAX
	);

#ifdef RECORD_CPU_TIMES
	float waitForFencesMs = Time::endTimer() * 1000.0f;
#endif

	// Get next image index from the swapchain
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(
		this->getVkDevice(),
		this->swapchain.getVkSwapchain(),
		UINT64_MAX,
		this->imageAvailableSemaphores[GfxState::getFrameIndex()],
		VK_NULL_HANDLE,
		&imageIndex
	);

	// Window resize?
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		this->resizeWindow();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		Log::error("Failed to acquire swapchain image.");
	}

	this->updateUniformBuffer(scene.getCamera());
	this->rsm.update();

	// Only reset the fence if we are submitting work
	this->inFlightFences.reset(GfxState::getFrameIndex());

#ifdef RECORD_CPU_TIMES
	Time::startTimer();
#endif

	// Record command buffer
	this->recordCommandBuffer(imageIndex, scene);

#ifdef RECORD_CPU_TIMES
	float recordCommandBufferMs = Time::endTimer() * 1000.0f;
#endif

	// Update and Render additional Platform Windows
	if (this->imguiIO->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	// Info for submitting command buffer
	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };

	VkSemaphore waitSemaphores[] = { this->imageAvailableSemaphores[GfxState::getFrameIndex()] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = 
		&this->commandBuffers[GfxState::getFrameIndex()].getVkCommandBuffer();

	VkSemaphore signalSemaphores[] = { this->postProcessFinishedSemaphores[GfxState::getFrameIndex()] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	// Submit command buffer
	if (vkQueueSubmit(
		this->queueFamilies.getVkGraphicsQueue(),
		1,
		&submitInfo,
		this->inFlightFences[GfxState::getFrameIndex()])
		!= VK_SUCCESS)
	{
		Log::error("Failed to submit graphics command buffer.");
	}

	// Present info
	VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { this->swapchain.getVkSwapchain() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

#ifdef RECORD_CPU_TIMES
	Time::startTimer();
#endif

	// Present
	result = vkQueuePresentKHR(this->queueFamilies.getVkPresentQueue(), &presentInfo);

#ifdef RECORD_CPU_TIMES
	float presentMs = Time::endTimer() * 1000.0f;
#endif

	// Window resize
	if (result == VK_ERROR_OUT_OF_DATE_KHR ||
		result == VK_SUBOPTIMAL_KHR ||
		this->framebufferResized)
	{
		this->framebufferResized = false;
		this->resizeWindow();
	}
	else if (result != VK_SUCCESS)
	{
		Log::error("Failed to present swapchain image.");
	}


#ifdef RECORD_CPU_TIMES
	// Gather this frame's data
	float cpuFrameTimeMs = (Time::getGodTime() * 1000.0f);

	// Average
	float t = 1.0f / (this->elapsedFrames + 1.0f);
	this->avgWaitForFenceMs = (1.0f - t) * this->avgWaitForFenceMs + t * waitForFencesMs;
	this->avgRecordCommandBufferMs = (1.0f - t) * this->avgRecordCommandBufferMs + t * recordCommandBufferMs;
	this->avgPresentMs = (1.0f - t) * this->avgPresentMs + t * presentMs;
	this->avgCpuFrameTimeMs = (1.0f - t) * this->avgCpuFrameTimeMs + t * cpuFrameTimeMs;
	this->elapsedFrames++;

	// Print
	Log::write(
		"elapsed frames: " + std::to_string(this->elapsedFrames) + 
		"   waitForFence: " + std::to_string(waitForFencesMs) + 
		"   recordCommandBuffer: " + std::to_string(recordCommandBufferMs) + 
		"   present ms: " + std::to_string(presentMs) + 
		"   CPU frame time ms: " + std::to_string(cpuFrameTimeMs));

	// Print average times after 500 frames
#ifdef ALERT_FINAL_AVERAGE
	if (this->elapsedFrames >= this->WAIT_ELAPSED_FRAMES_FOR_AVG - 0.5f)
	{
		Log::writeAlert(
			"waitForFence ms: " + StrHelper::toTimingStr(this->avgWaitForFenceMs) + "\n" +
			"recordCommandBuffer ms: " + StrHelper::toTimingStr(this->avgRecordCommandBufferMs) + "\n" +
			"present ms: " + StrHelper::toTimingStr(this->avgPresentMs) + "\n" +
			"CPU frame time ms: " + StrHelper::toTimingStr(this->avgCpuFrameTimeMs));
	}
#endif
#endif 

	// TODO: fix this by waiting at the start of a frame
#ifdef RECORD_GPU_TIMES
	this->device.waitIdle();
	this->queryPools.getQueryPoolResults(GfxState::getFrameIndex());

	// Gather this frame's data
	float rsmMs = 
		float(this->queryPools.getQueryResult(GfxState::getFrameIndex(), 1) - 
			this->queryPools.getQueryResult(GfxState::getFrameIndex(), 0)) *
		GpuProperties::getTimestampPeriod() * 1e-6;
	float smMs =
		float(this->queryPools.getQueryResult(GfxState::getFrameIndex(), 3) -
			this->queryPools.getQueryResult(GfxState::getFrameIndex(), 2)) *
		GpuProperties::getTimestampPeriod() * 1e-6;
	float deferredGeomMs =
		float(this->queryPools.getQueryResult(GfxState::getFrameIndex(), 5) -
			this->queryPools.getQueryResult(GfxState::getFrameIndex(), 4)) *
		GpuProperties::getTimestampPeriod() * 1e-6;
	float deferredLightMs =
		float(this->queryPools.getQueryResult(GfxState::getFrameIndex(), 7) -
			this->queryPools.getQueryResult(GfxState::getFrameIndex(), 6)) *
		GpuProperties::getTimestampPeriod() * 1e-6;
	float entireGpuFrameTimeMs = 
		float(this->queryPools.getQueryResult(GfxState::getFrameIndex(), 8) -
		this->queryPools.getQueryResult(GfxState::getFrameIndex(), 0)) *
		GpuProperties::getTimestampPeriod() * 1e-6;

	// Average
	float t = 1.0f / (this->elapsedFrames + 1.0f);
	this->avgRsmMs = (1.0f - t) * this->avgRsmMs + t * rsmMs;
	this->avgSmMs = (1.0f - t) * this->avgSmMs + t * smMs;
	this->avgDeferredGeomMs = (1.0f - t) * this->avgDeferredGeomMs + t * deferredGeomMs;
	this->avgDeferredLightMs = (1.0f - t) * this->avgDeferredLightMs + t * deferredLightMs;
	this->avgGpuFrameTimeMs = (1.0f - t) * this->avgGpuFrameTimeMs + t * entireGpuFrameTimeMs;
	this->elapsedFrames++;

	// Print
	Log::write("elapsedFrames: " + std::to_string(this->elapsedFrames) + "   rsm ms: " + std::to_string(rsmMs) + "   sm ms: " + std::to_string(smMs) + "   deferred geom ms: " + std::to_string(deferredGeomMs) + "   deferred light ms: " + std::to_string(deferredLightMs));
	
	// Print average times after 500 frames
#ifdef ALERT_FINAL_AVERAGE
	if (this->elapsedFrames >= this->WAIT_ELAPSED_FRAMES_FOR_AVG - 0.5f)
	{
		Log::writeAlert(
			"rsm ms: " + StrHelper::toTimingStr(this->avgRsmMs) + "\n" +
			"sm ms: " + StrHelper::toTimingStr(this->avgSmMs) + "\n" +
			"deferred geom ms: " + StrHelper::toTimingStr(this->avgDeferredGeomMs) + "\n" +
			"deferred light ms: " + StrHelper::toTimingStr(this->avgDeferredLightMs) + "\n" +
			"GPU frame time ms: " + StrHelper::toTimingStr(this->avgGpuFrameTimeMs));
	}
#endif
#endif

	// Next frame index
	GfxState::currentFrameIndex = (GfxState::currentFrameIndex + 1) % GfxSettings::FRAMES_IN_FLIGHT;
}

void Renderer::generateMemoryDump()
{
	Log::alert("Generated memory dump called \"VmaDump.json\"");

	char* vmaDump;
	vmaBuildStatsString(this->vmaAllocator, &vmaDump, VK_TRUE);

	std::ofstream file("VmaDump.json");
	file << vmaDump << std::endl;
	file.close();

	vmaFreeStatsString(this->vmaAllocator, vmaDump);
}

void Renderer::setSkyboxTexture(uint32_t skyboxTextureIndex)
{
	this->skyboxTextureIndex = skyboxTextureIndex;
}

void Renderer::updateUniformBuffer(const Camera& camera)
{
	// Create ubo struct with matrix data
	CamUBO camUbo{};
	camUbo.vp = camera.getProjectionMatrix() * camera.getViewMatrix();
	camUbo.pos = glm::vec4(camera.getPosition(), 1.0f);

	// Update buffer contents
	this->uniformBuffer.updateBuffer(&camUbo);
}

void Renderer::recordCommandBuffer(
	uint32_t imageIndex, 
	Scene& scene)
{
	ImDrawData* imguiDrawData = ImGui::GetDrawData();

	CommandBuffer& commandBuffer = this->commandBuffers[GfxState::getFrameIndex()];

	// Begin
	commandBuffer.resetAndBegin();

#ifdef RECORD_GPU_TIMES
	commandBuffer.resetEntireQueryPool(
		this->queryPools[GfxState::getFrameIndex()],
		this->queryPools.getQueryCount()
	);

	commandBuffer.writeTimestamp(
		this->queryPools[GfxState::getFrameIndex()],
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0
	);
#endif

	this->renderRSM(commandBuffer, scene);

#ifdef RECORD_GPU_TIMES
	commandBuffer.writeTimestamp(
		this->queryPools[GfxState::getFrameIndex()],
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		1
	);
	commandBuffer.writeTimestamp(
		this->queryPools[GfxState::getFrameIndex()],
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		2
	);
#endif

	this->renderShadowMap(commandBuffer, scene);

#ifdef RECORD_GPU_TIMES
	commandBuffer.writeTimestamp(
		this->queryPools[GfxState::getFrameIndex()],
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		3
	);
	commandBuffer.writeTimestamp(
		this->queryPools[GfxState::getFrameIndex()],
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		4
	);
#endif

	this->renderDeferredScene(commandBuffer, scene);

#ifdef RECORD_GPU_TIMES
	commandBuffer.writeTimestamp(
		this->queryPools[GfxState::getFrameIndex()],
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		5
	);
	commandBuffer.writeTimestamp(
		this->queryPools[GfxState::getFrameIndex()],
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		6
	);
#endif

	this->computeDeferredLight(commandBuffer, scene.getCamera().getPosition());

#ifdef RECORD_GPU_TIMES
	commandBuffer.writeTimestamp(
		this->queryPools[GfxState::getFrameIndex()],
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		7
	);
#endif

	//this->renderImgui(commandBuffer, imguiDrawData);
	this->computePostProcess(commandBuffer, imageIndex);

#ifdef RECORD_GPU_TIMES
	commandBuffer.writeTimestamp(
		this->queryPools[GfxState::getFrameIndex()],
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		8
	);
#endif

	// Stop recording
	commandBuffer.end();
}

void Renderer::resizeWindow()
{
	this->swapchain.recreate();
}

void Renderer::cleanupImgui()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	this->imguiPipelineLayout.cleanup();
	this->imguiPipeline.cleanup();
	this->imguiDescriptorPool.cleanup();
}

Renderer::Renderer()
	: window(nullptr),
	resourceManager(nullptr),
	imguiIO(nullptr),
	vmaAllocator(nullptr),

#ifdef RECORD_GPU_TIMES
	elapsedFrames(0.0f),
	avgRsmMs(0.0f),
	avgSmMs(0.0f),
	avgDeferredGeomMs(0.0f),
	avgDeferredLightMs(0.0f),
	avgGpuFrameTimeMs(0.0f),
#endif

#ifdef RECORD_CPU_TIMES
	elapsedFrames(0.0f),
	avgWaitForFenceMs(0.0f),
	avgRecordCommandBufferMs(0.0f),
	avgPresentMs(0.0f),
	avgCpuFrameTimeMs(0.0f),
#endif

	skyboxTextureIndex(~0u)
{
}

Renderer::~Renderer()
{
}

void Renderer::init(ResourceManager& resourceManager)
{
	this->resourceManager = &resourceManager;

	this->initVulkan();
	this->initImgui();
}

void Renderer::initForScene(Scene& scene)
{
	this->createShCoefficientBuffer(scene);

	// Add skybox as entity to scene
	{
		// Material
		Material skyboxMaterial{};
		std::strcpy(skyboxMaterial.vertexShader, "DeferredGeomSkybox.vert.spv");
		std::strcpy(skyboxMaterial.fragmentShader, "DeferredGeomSkybox.frag.spv");
		skyboxMaterial.albedoTextureId = this->skyboxTextureIndex;
		skyboxMaterial.castShadows = false;

		// Mesh
		MeshComponent skyboxMesh{};
		skyboxMesh.meshId = this->resourceManager->addMesh("Resources/Models/invertedCube.obj", skyboxMaterial);

		// Entity
		uint32_t skybox = scene.createEntity();
		scene.setComponent<MeshComponent>(skybox, skyboxMesh);
		scene.setComponent<Material>(skybox, skyboxMaterial);
	}

	// Materials
	uint32_t whiteTextureId = this->resourceManager->addTexture("Resources/Textures/white.png");
	auto tView = scene.getRegistry().view<Material>();
	tView.each([&](Material& material)
		{
			// Pipelines for materials
			material.rsmPipelineIndex = this->gfxResManager.getMaterialRsmPipelineIndex(material);
			material.shadowMapPipelineIndex = this->gfxResManager.getMaterialShadowMapPipelineIndex(material);
			material.deferredGeomPipelineIndex = this->gfxResManager.getMaterialPipelineIndex(material);
		}
	);

	// Sort materials based on pipeline indices
	scene.getRegistry().sort<Material>(
		[](const auto& lhs, const auto& rhs)
		{
			return lhs.deferredGeomPipelineIndex < rhs.deferredGeomPipelineIndex;
		}
	);
}

void Renderer::setWindow(Window& window)
{
	this->window = &window;
}

void Renderer::startCleanup()
{
	// Wait for device before cleanup
	this->device.waitIdle();
}

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
	this->queryPools.create(this->device, GfxSettings::FRAMES_IN_FLIGHT, MAX_QUERY_COUNT);
#endif

	// Init sort list compute pipeline
	this->initSortListPipelineLayout.createPipelineLayout(
		this->device,
		{
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },

			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT }
		},
		VK_SHADER_STAGE_COMPUTE_BIT,
		sizeof(InitSortListPCD)
	);
	this->initSortListPipeline.createComputePipeline(
		this->device,
		this->initSortListPipelineLayout,
		"Resources/Shaders/InitSortList.comp.spv"
	);

	// Sort gaussians bitonic merge sort compute pipeline
	this->sortGaussiansBmsPipelineLayout.createPipelineLayout(
		this->device,
		{
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT }
		},
		VK_SHADER_STAGE_COMPUTE_BIT,
		sizeof(SortGaussiansBmsPCD)
	);
	this->sortGaussiansBmsPipeline.createComputePipeline(
		this->device,
		this->sortGaussiansBmsPipelineLayout,
		"Resources/Shaders/BitonicMergeSort.comp.spv",
		{
			SpecializationConstant{ (void*) Renderer::BMS_WORK_GROUP_SIZE, sizeof(uint32_t)}
		}
	);

	// Radix sort compute pipeline (count)
	this->radixSortCountPipelineLayout.createPipelineLayout(
		this->device,
		{
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT }
		},
		VK_SHADER_STAGE_COMPUTE_BIT,
		sizeof(SortGaussiansRsPCD)
	);
	this->radixSortCountPipeline.createComputePipeline(
		this->device,
		this->radixSortCountPipelineLayout,
		"Resources/Shaders/RadixSortCount.comp.spv",
		{
			SpecializationConstant{ (void*)Renderer::RS_WORK_GROUP_SIZE, sizeof(uint32_t)}
		}
	);

	// Radix sort compute pipeline (reduce)
	this->radixSortReducePipelineLayout.createPipelineLayout(
		this->device,
		{
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT }
		},
		VK_SHADER_STAGE_COMPUTE_BIT
	);
	this->radixSortReducePipeline.createComputePipeline(
		this->device,
		this->radixSortReducePipelineLayout,
		"Resources/Shaders/RadixSortReduce.comp.spv",
		{
			SpecializationConstant{ (void*)Renderer::RS_WORK_GROUP_SIZE, sizeof(uint32_t)}
		}
	);

	// Radix sort compute pipeline (scan)
	this->radixSortScanPipelineLayout.createPipelineLayout(
		this->device,
		{
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT }
		},
		VK_SHADER_STAGE_COMPUTE_BIT,
		sizeof(SortGaussiansRsPCD)
	);
	this->radixSortScanPipeline.createComputePipeline(
		this->device,
		this->radixSortScanPipelineLayout,
		"Resources/Shaders/RadixSortScan.comp.spv",
		{
			SpecializationConstant{ (void*)Renderer::RS_WORK_GROUP_SIZE, sizeof(uint32_t)}
		}
	);

	// Radix sort compute pipeline (scan add)
	this->radixSortScanAddPipelineLayout.createPipelineLayout(
		this->device,
		{
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT }
		},
		VK_SHADER_STAGE_COMPUTE_BIT,
		sizeof(SortGaussiansRsPCD)
	);
	this->radixSortScanAddPipeline.createComputePipeline(
		this->device,
		this->radixSortScanAddPipelineLayout,
		"Resources/Shaders/RadixSortScanAdd.comp.spv",
		{
			SpecializationConstant{ (void*)Renderer::RS_WORK_GROUP_SIZE, sizeof(uint32_t)}
		}
	);

	// Radix sort compute pipeline (scatter)
	this->radixSortScatterPipelineLayout.createPipelineLayout(
		this->device,
		{
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT }
		},
		VK_SHADER_STAGE_COMPUTE_BIT,
		sizeof(SortGaussiansRsPCD)
	);
	this->radixSortScatterPipeline.createComputePipeline(
		this->device,
		this->radixSortScatterPipelineLayout,
		"Resources/Shaders/RadixSortScatter.comp.spv",
		{
			SpecializationConstant{ (void*)Renderer::RS_WORK_GROUP_SIZE, sizeof(uint32_t)}
		}
	);

	// Find ranges compute pipeline
	this->findRangesPipelineLayout.createPipelineLayout(
		this->device,
		{
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT }
		},
		VK_SHADER_STAGE_COMPUTE_BIT,
		sizeof(FindRangesPCD)
	);
	this->findRangesPipeline.createComputePipeline(
		this->device,
		this->findRangesPipelineLayout,
		"Resources/Shaders/FindRanges.comp.spv"
	);

	// Render gaussians compute pipeline
	this->renderGaussiansPipelineLayout.createPipelineLayout(
		this->device,
		{
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },

			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT },

			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT }
		},
		VK_SHADER_STAGE_COMPUTE_BIT,
		sizeof(RenderGaussiansPCD)
	);
	this->renderGaussiansPipeline.createComputePipeline(
		this->device, 
		this->renderGaussiansPipelineLayout,
		"Resources/Shaders/RenderGaussians.comp.spv"
	);

	this->createSyncObjects();
	this->createCamUbo();
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
		this->swapchain.getVkFormat(),
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

	this->swapchain.cleanup();

	this->radixSortPingPongBuffer.cleanup();
	this->radixSortReduceBuffer.cleanup();
	this->radixSortSumTableBuffer.cleanup();

	this->gaussiansTileRangesSBO.cleanup();
	this->gaussiansCullDataSBO.cleanup();
	this->gaussiansSortListSBO.cleanup();
	this->gaussiansSBO.cleanup();
	this->camUBO.cleanup();

	this->imageAvailableSemaphores.cleanup();
	this->renderGaussiansFinishedSemaphores.cleanup();
	this->inFlightFences.cleanup();

#ifdef RECORD_GPU_TIMES
	this->queryPools.cleanup();
#endif

	this->singleTimeCommandPool.cleanup();
	this->commandPool.cleanup();
	this->renderGaussiansPipeline.cleanup();
	this->renderGaussiansPipelineLayout.cleanup();
	this->findRangesPipeline.cleanup();
	this->findRangesPipelineLayout.cleanup();

	this->radixSortScatterPipelineLayout.cleanup();
	this->radixSortScatterPipeline.cleanup();
	this->radixSortScanAddPipelineLayout.cleanup();
	this->radixSortScanAddPipeline.cleanup();
	this->radixSortScanPipelineLayout.cleanup();
	this->radixSortScanPipeline.cleanup();
	this->radixSortReducePipelineLayout.cleanup();
	this->radixSortReducePipeline.cleanup();
	this->radixSortCountPipelineLayout.cleanup();
	this->radixSortCountPipeline.cleanup();

	this->sortGaussiansBmsPipeline.cleanup();
	this->sortGaussiansBmsPipelineLayout.cleanup();

	this->initSortListPipeline.cleanup();
	this->initSortListPipelineLayout.cleanup();
	
	vmaDestroyAllocator(this->vmaAllocator);
	
	this->device.cleanup();
	this->debugMessenger.cleanup();
	this->surface.cleanup();

	// Destroys both physical device and instance
	this->instance.cleanup();
}

void Renderer::createCamUbo()
{
	this->camUBO.createCpuGpuBuffer(
		this->gfxAllocContext,
		sizeof(CamUBO)
	);
}

void Renderer::createSyncObjects()
{
	this->imageAvailableSemaphores.create(
		this->device, 
		GfxSettings::FRAMES_IN_FLIGHT
	);
	this->renderGaussiansFinishedSemaphores.create(
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

	VkSemaphore signalSemaphores[] = { this->renderGaussiansFinishedSemaphores[GfxState::getFrameIndex()] };
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
	this->avgWaitForFenceMs = this->getNewAvgTime(this->avgWaitForFenceMs, waitForFencesMs, t);
	this->avgRecordCommandBufferMs = this->getNewAvgTime(this->avgRecordCommandBufferMs, recordCommandBufferMs, t);
	this->avgPresentMs = this->getNewAvgTime(this->avgPresentMs, presentMs, t);
	this->avgCpuFrameTimeMs = this->getNewAvgTime(this->avgCpuFrameTimeMs, cpuFrameTimeMs, t);
	this->elapsedFrames++;

	// Print
	Log::write(
		"elapsed frames: " + std::to_string(this->elapsedFrames) + 
		"   waitForFence: " + std::to_string(waitForFencesMs) + 
		"   recordCommandBuffer: " + std::to_string(recordCommandBufferMs) + 
		"   present ms: " + std::to_string(presentMs) + 
		"   CPU frame time ms: " + std::to_string(cpuFrameTimeMs));

	// Print average times after a number of frames
#ifdef ALERT_FINAL_AVERAGE
	if (this->elapsedFrames >= this->WAIT_ELAPSED_FRAMES_FOR_AVG - 0.5f)
	{
		Log::writeAlert("Not properly implemented yet...");
		Log::writeAlert(
			"waitForFence ms: " + StrHelper::toTimingStr(this->avgWaitForFenceMs) + "\n" +
			"recordCommandBuffer ms: " + StrHelper::toTimingStr(this->avgRecordCommandBufferMs) + "\n" +
			"present ms: " + StrHelper::toTimingStr(this->avgPresentMs) + "\n" +
			"CPU frame time ms: " + StrHelper::toTimingStr(this->avgCpuFrameTimeMs));
	}
#endif
#endif 

#ifdef RECORD_GPU_TIMES
	this->device.waitIdle();
	this->queryPools.getQueryPoolResults(GfxState::getFrameIndex());

	// Gather this frame's data
	auto computeDiffs = [&](uint32_t startQueryIndex, uint32_t endQueryIndex)
	{
		return float(this->queryPools.getQueryResult(GfxState::getFrameIndex(), endQueryIndex) -
			this->queryPools.getQueryResult(GfxState::getFrameIndex(), startQueryIndex)) *
			GpuProperties::getTimestampPeriod() * 1e-6;
	};
	float initSortListMs = computeDiffs(1, 2);
	float sortMs = computeDiffs(2, 3);
	float findRangesMs = computeDiffs(3, 4);
	float renderGaussiansMs = computeDiffs(4, 5);
	float totalGpuTimeMs = computeDiffs(0, 6);

	// Average timings
	float t = 1.0f / (this->elapsedFrames + 1.0f);
	this->avgInitSortListMs = this->getNewAvgTime(this->avgInitSortListMs, initSortListMs, t);
	this->avgSortMs = this->getNewAvgTime(this->avgSortMs, sortMs, t);
	this->avgFindRangesMs = this->getNewAvgTime(this->avgFindRangesMs, findRangesMs, t);
	this->avgRenderGaussiansMs = this->getNewAvgTime(this->avgRenderGaussiansMs, renderGaussiansMs, t);
	this->avgTotalGpuTimeMs = this->getNewAvgTime(this->avgTotalGpuTimeMs, totalGpuTimeMs, t);
	this->elapsedFrames++;

	// Print
	Log::write(
		"elapsedFrames: " + std::to_string(this->elapsedFrames) + 
		"   init sort list ms: " + std::to_string(initSortListMs) + 
		"   sort ms: " + std::to_string(sortMs) +
		"   find ranges ms: " + std::to_string(findRangesMs) +
		"   render gaussians ms: " + std::to_string(renderGaussiansMs) + 
		"   total gpu time ms: " + std::to_string(totalGpuTimeMs)
	);
	
	// Print average times after a number of frames
#ifdef ALERT_FINAL_AVERAGE
	if (this->elapsedFrames >= this->WAIT_ELAPSED_FRAMES_FOR_AVG - 0.5f)
	{
		Log::writeAlert("Not properly implemented yet...");
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

void Renderer::updateUniformBuffer(const Camera& camera)
{
	CamUBO camUbo{};
	camUbo.viewMat = camera.getViewMatrix();
	camUbo.projMat = camera.getProjectionMatrix();

	this->camUBO.updateBuffer(&camUbo);
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
	commandBuffer.writeTimestamp(
		this->queryPools[GfxState::getFrameIndex()],
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		1
	);
#endif

	this->computeInitSortList(
		commandBuffer,
		scene.getCamera()
	);

#ifdef RECORD_GPU_TIMES
	commandBuffer.writeTimestamp(
		this->queryPools[GfxState::getFrameIndex()],
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		2
	);
#endif

	this->computeSortGaussians(
		commandBuffer,
		this->numSortElements
	);

#ifdef RECORD_GPU_TIMES
	commandBuffer.writeTimestamp(
		this->queryPools[GfxState::getFrameIndex()],
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		3
	);
#endif

	this->computeRanges(
		commandBuffer
	);

#ifdef RECORD_GPU_TIMES
	commandBuffer.writeTimestamp(
		this->queryPools[GfxState::getFrameIndex()],
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		4
	);
#endif

	this->computeRenderGaussians(
		commandBuffer,
		imageIndex
	);

#ifdef RECORD_GPU_TIMES
	commandBuffer.writeTimestamp(
		this->queryPools[GfxState::getFrameIndex()],
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		5
	);
	commandBuffer.writeTimestamp(
		this->queryPools[GfxState::getFrameIndex()],
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		6
	);
#endif

	//this->renderImgui(commandBuffer, imguiDrawData);

	// Stop recording
	commandBuffer.end();
}

void Renderer::resizeWindow()
{
	assert(false);
	Log::warning("Gaussian tile range buffer needs to be recreated...");

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

#ifdef RECORD_GPU_TIMES
	elapsedFrames(0.0f),
	avgInitSortListMs(0.0f),
	avgSortMs(0.0f),
	avgFindRangesMs(0.0f),
	avgRenderGaussiansMs(0.0f),
	avgTotalGpuTimeMs(0.0f),
#endif

#ifdef RECORD_CPU_TIMES
	elapsedFrames(0.0f),
	avgWaitForFenceMs(0.0f),
	avgRecordCommandBufferMs(0.0f),
	avgPresentMs(0.0f),
	avgCpuFrameTimeMs(0.0f),
#endif

	vmaAllocator(nullptr),
	numGaussians(0),
	numSortElements(0)
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

uint32_t Renderer::getNumTiles() const
{
	return 
		((this->swapchain.getVkExtent().width + TILE_SIZE - 1) / TILE_SIZE) *
		((this->swapchain.getVkExtent().height + TILE_SIZE - 1) / TILE_SIZE);
}

uint32_t Renderer::getCeilPowTwo(uint32_t x) const
{
	uint32_t num = 1;
	while (num < x)
		num *= 2u;

	return num;
}

void Renderer::initForScene(Scene& scene)
{
	// Retrieve gaussians from resource manager
	const std::vector<GaussianData>& gaussiansData = 
		this->resourceManager->getGaussians();

	// Gaussians SBO
	this->gaussiansSBO.createGpuBuffer(
		this->gfxAllocContext,
		sizeof(gaussiansData[0]) * gaussiansData.size(),
		gaussiansData.data()
	);
	this->numGaussians = (uint32_t) gaussiansData.size();
	this->numSortElements = this->getCeilPowTwo(this->numGaussians + 48 * 16 * this->getNumTiles());

	// Gaussians list SBO for sorting
	std::vector<GaussianSortData> sortData(this->numSortElements); // Dummy data
	this->gaussiansSortListSBO.createGpuBuffer(
		this->gfxAllocContext,
		sizeof(sortData[0]) * sortData.size(),
		sortData.data()
	);

	// Cull data
	GaussianCullData cullData{};
	cullData.numGaussiansToRender.y = this->numSortElements;
	this->gaussiansCullDataSBO.createGpuBuffer(
		this->gfxAllocContext,
		sizeof(GaussianCullData),
		&cullData
	);

	// Range data
	uint32_t numTiles = this->getNumTiles();
	const std::vector<GaussianTileRangeData> dummyRangeData(numTiles);
	this->gaussiansTileRangesSBO.createGpuBuffer(
		this->gfxAllocContext,
		sizeof(dummyRangeData[0]) * dummyRangeData.size(),
		dummyRangeData.data()
	);



	// Buffers for radix sort
	uint32_t numCountThreadGroups = (this->numSortElements + RS_WORK_GROUP_SIZE - 1) / RS_WORK_GROUP_SIZE;
	uint32_t numSumElements = numCountThreadGroups * RS_BIN_COUNT;
	const std::vector<glm::uvec4> dummySumTableData(numSumElements);
	this->radixSortSumTableBuffer.createGpuBuffer(
		this->gfxAllocContext,
		sizeof(dummySumTableData[0]) * dummySumTableData.size(),
		dummySumTableData.data()
	);

	uint32_t numReduceBlocks = (numCountThreadGroups + RS_WORK_GROUP_SIZE - 1) / RS_WORK_GROUP_SIZE;
	uint32_t numReduceElements = numReduceBlocks * RS_BIN_COUNT;
	const std::vector<glm::uvec4> dummyReduceData(numReduceElements);
	this->radixSortReduceBuffer.createGpuBuffer(
		this->gfxAllocContext,
		sizeof(dummyReduceData[0]) * dummyReduceData.size(),
		dummyReduceData.data()
	);

	this->radixSortPingPongBuffer.createGpuBuffer(
		this->gfxAllocContext,
		sizeof(sortData[0]) * sortData.size(),
		sortData.data()
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

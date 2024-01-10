#include "pch.h"
#include "Renderer.h"
#include "../ResourceManager.h"
#include "Texture/TextureCube.h"
#include "../Components.h"

void Renderer::renderMeshWithBrdf(
	CommandBuffer& commandBuffer, 
	const Mesh& mesh, 
	const Material& material,
	PCD& pushConstantData)
{
	const std::vector<Submesh>& submeshes = mesh.getSubmeshes();

	// Record binding vertex/index buffer
	commandBuffer.bindVertexBuffer(mesh.getVertexBuffer());
	commandBuffer.bindIndexBuffer(mesh.getIndexBuffer());

	// Record draws
	for (size_t i = 0, numSubmeshes = submeshes.size(); i < numSubmeshes; ++i)
	{
		commandBuffer.pushConstant(
			this->gfxResManager.getGraphicsPipelineLayout(), // Can be fixed with shader reflection
			&pushConstantData
		);

		const Submesh& currentSubmesh = submeshes[i];
		commandBuffer.drawIndexed(currentSubmesh.numIndices, currentSubmesh.startIndex);
	}
}

void Renderer::renderDeferredScene(CommandBuffer& commandBuffer, Scene& scene)
{
	// Dynamic viewport
	float swapchainHeight = (float)this->swapchain.getHeight();
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = swapchainHeight;
	viewport.width = static_cast<float>(this->swapchain.getWidth());
	viewport.height = -swapchainHeight;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	commandBuffer.setViewport(viewport);

	// Dynamic scissor
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = this->swapchain.getVkExtent();
	commandBuffer.setScissor(scissor);

	// Bind dummy pipeline before begin rendering
	commandBuffer.bindPipeline(this->gfxResManager.getOneHdrPipeline());

	// Render to HDR buffer
	
	// Transition layouts for color and depth
	std::array<VkImageMemoryBarrier2, 5> memoryBarriers =
	{
		// Position
		PipelineBarrier::imageMemoryBarrier2(
			VK_ACCESS_NONE,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			this->swapchain.getDeferredPositionTexture().getVkImage(),
			VK_IMAGE_ASPECT_COLOR_BIT
		),

		// Normal
		PipelineBarrier::imageMemoryBarrier2(
			VK_ACCESS_NONE,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			this->swapchain.getDeferredNormalTexture().getVkImage(),
			VK_IMAGE_ASPECT_COLOR_BIT
		),

		// BRDF index
		PipelineBarrier::imageMemoryBarrier2(
			VK_ACCESS_NONE,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			this->swapchain.getDeferredBrdfTexture().getVkImage(),
			VK_IMAGE_ASPECT_COLOR_BIT
		),

		// HDR
		PipelineBarrier::imageMemoryBarrier2(
			VK_ACCESS_NONE,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			this->swapchain.getHdrTexture().getVkImage(),
			VK_IMAGE_ASPECT_COLOR_BIT
		),

		// Depth
		PipelineBarrier::imageMemoryBarrier2(
			VK_ACCESS_NONE,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, // Stage from "previous frame"
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,	// Stage from "current frame"
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			this->swapchain.getDepthTexture().getVkImage(),
			VK_IMAGE_ASPECT_DEPTH_BIT
		)
	};
	commandBuffer.imageMemoryBarrier(memoryBarriers.data(), uint32_t(memoryBarriers.size()));

	// Clear values for color and depth
	std::array<VkClearValue, 4> clearValues{};
	clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	clearValues[1].color = { { 64.0f, 64.0f, 64.0f, 1.0f } };
	clearValues[2].color.uint32[0] = 0u; clearValues[2].color.uint32[1] = 0u; clearValues[2].color.uint32[2] = 0u; clearValues[2].color.uint32[3] = 0u;
	clearValues[3].depthStencil = { 1.0f, 0 };

	// Color attachment
	std::array<VkRenderingAttachmentInfo, 3> colorAttachments{};
	colorAttachments[0].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorAttachments[0].imageView = this->swapchain.getDeferredPositionTexture().getVkImageView();
	colorAttachments[0].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachments[0].clearValue = clearValues[0];

	colorAttachments[1].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorAttachments[1].imageView = this->swapchain.getDeferredNormalTexture().getVkImageView();
	colorAttachments[1].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachments[1].clearValue = clearValues[1];

	colorAttachments[2].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorAttachments[2].imageView = this->swapchain.getDeferredBrdfTexture().getVkImageView();
	colorAttachments[2].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachments[2].clearValue = clearValues[2];

	// Depth attachment
	VkRenderingAttachmentInfo depthAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
	depthAttachment.imageView = this->swapchain.getDepthTexture().getVkImageView();
	depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.clearValue = clearValues[1];

	// Begin rendering
	VkRenderingInfo renderingInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
	renderingInfo.renderArea = { { 0, 0 }, this->swapchain.getVkExtent() };
	renderingInfo.layerCount = 1;
	renderingInfo.colorAttachmentCount = uint32_t(colorAttachments.size());
	renderingInfo.pColorAttachments = colorAttachments.data();
	renderingInfo.pDepthAttachment = &depthAttachment;
	commandBuffer.beginRendering(renderingInfo);

	{
		uint32_t currentPipelineIndex = ~0u;
		uint32_t numPipelineSwitches = 0;

		// Common descriptor set bindings

		// Binding 0
		VkDescriptorBufferInfo camUboInfo{};
		camUboInfo.buffer = this->camUBO.getVkBuffer(GfxState::getFrameIndex());
		camUboInfo.range = this->camUBO.getBufferSize();

		// Binding 1
		VkDescriptorImageInfo albedoImageInfo{};
		albedoImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		std::array<VkWriteDescriptorSet, 2> writeDescriptorSets
		{
			DescriptorSet::writeBuffer(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &camUboInfo),

			DescriptorSet::writeImage(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nullptr)
		};

		// Loop through entities with mesh components
		auto view = scene.getRegistry().view<Material, MeshComponent, Transform>();
		view.each([&](
			const Material& material,
			const MeshComponent& meshComp,
			const Transform& transform)
			{
				// Switch pipeline if necessary
				if (currentPipelineIndex != material.deferredGeomPipelineIndex)
				{
					commandBuffer.bindPipeline(
						this->gfxResManager.getPipeline(material.deferredGeomPipelineIndex)
					);

					currentPipelineIndex = material.deferredGeomPipelineIndex;
					numPipelineSwitches++;
				}

				// Binding 1
				const Texture* albedoTexture =
					this->resourceManager->getTexture(material.albedoTextureId);
				albedoImageInfo.imageView = albedoTexture->getVkImageView();
				albedoImageInfo.sampler = albedoTexture->getVkSampler();

				// Push descriptor set update
				writeDescriptorSets[1].pImageInfo = &albedoImageInfo;
				commandBuffer.pushDescriptorSet(
					this->gfxResManager.getGraphicsPipelineLayout(),
					0,
					uint32_t(writeDescriptorSets.size()),
					writeDescriptorSets.data()
				);

				// Push constant data
				PCD pushConstantData{};
				pushConstantData.modelMat = transform.modelMat;

				// Mesh
				const Mesh& currentMesh = this->resourceManager->getMesh(meshComp.meshId);
				this->renderMeshWithBrdf(commandBuffer, currentMesh, material, pushConstantData);
			}
		);
	}

	// End rendering
	commandBuffer.endRendering();
}

void Renderer::computeDeferredLight(CommandBuffer& commandBuffer, const glm::vec3& camPos)
{
	// Transition HDR and swapchain image
	std::array<VkImageMemoryBarrier2, 4> deferredLightMemoryBarriers =
	{
		// Position
		PipelineBarrier::imageMemoryBarrier2(
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_GENERAL,
			this->swapchain.getDeferredPositionTexture().getVkImage(),
			VK_IMAGE_ASPECT_COLOR_BIT
		),

		// Normal
		PipelineBarrier::imageMemoryBarrier2(
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_GENERAL,
			this->swapchain.getDeferredNormalTexture().getVkImage(),
			VK_IMAGE_ASPECT_COLOR_BIT
		),

		// BRDF index
		PipelineBarrier::imageMemoryBarrier2(
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_GENERAL,
			this->swapchain.getDeferredBrdfTexture().getVkImage(),
			VK_IMAGE_ASPECT_COLOR_BIT
		),

		// Swapchain image
		PipelineBarrier::imageMemoryBarrier2(
			VK_ACCESS_NONE,
			VK_ACCESS_SHADER_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_GENERAL,
			this->swapchain.getHdrTexture().getVkImage(),
			VK_IMAGE_ASPECT_COLOR_BIT
		)
	};
	commandBuffer.imageMemoryBarrier(deferredLightMemoryBarriers.data(), uint32_t(deferredLightMemoryBarriers.size()));

	// Compute pipeline
	commandBuffer.bindPipeline(this->deferredLightPipeline);

	// Binding 0
	VkDescriptorImageInfo inputPositionImageInfo{};
	inputPositionImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	inputPositionImageInfo.imageView = this->swapchain.getDeferredPositionTexture().getVkImageView();

	// Binding 1
	VkDescriptorImageInfo inputNormalImageInfo{};
	inputNormalImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	inputNormalImageInfo.imageView = this->swapchain.getDeferredNormalTexture().getVkImageView();

	// Binding 2
	VkDescriptorImageInfo inputBrdfImageInfo{};
	inputBrdfImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	inputBrdfImageInfo.imageView = this->swapchain.getDeferredBrdfTexture().getVkImageView();

	// Binding 3
	VkDescriptorImageInfo outputImageInfo{};
	outputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	outputImageInfo.imageView = this->swapchain.getHdrTexture().getVkImageView();

	// Descriptor set
	std::array<VkWriteDescriptorSet, 4> computeWriteDescriptorSets
	{
		DescriptorSet::writeImage(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, &inputPositionImageInfo),
		DescriptorSet::writeImage(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, &inputNormalImageInfo),
		DescriptorSet::writeImage(2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, &inputBrdfImageInfo),
		DescriptorSet::writeImage(3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, &outputImageInfo)
	};
	commandBuffer.pushDescriptorSet(
		this->deferredLightPipelineLayout,
		0,
		uint32_t(computeWriteDescriptorSets.size()),
		computeWriteDescriptorSets.data()
	);

	// Push constant
	DeferredLightPCD deferredLightData{};
	deferredLightData.resolution =
		glm::uvec4(
			this->swapchain.getVkExtent().width,
			this->swapchain.getVkExtent().height,
			0u,
			0u
		);
	deferredLightData.camPos = glm::vec4(camPos, 0.0f);
	commandBuffer.pushConstant(
		this->deferredLightPipelineLayout,
		(void*)&deferredLightData
	);

	// Run compute shader
	commandBuffer.dispatch(
		(deferredLightData.resolution.x + 16 - 1) / 16,
		(deferredLightData.resolution.y + 16 - 1) / 16
	);
}

void Renderer::renderImgui(CommandBuffer& commandBuffer, ImDrawData* imguiDrawData)
{
	// Imgui using dynamic rendering
	VkRenderingAttachmentInfo imguiColorAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
	imguiColorAttachment.imageView = this->swapchain.getHdrTexture().getVkImageView();
	imguiColorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	imguiColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	imguiColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	VkRenderingInfo imguiRenderingInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
	imguiRenderingInfo.renderArea = { { 0, 0 }, this->swapchain.getVkExtent() };
	imguiRenderingInfo.layerCount = 1;
	imguiRenderingInfo.colorAttachmentCount = 1;
	imguiRenderingInfo.pColorAttachments = &imguiColorAttachment;

	// Bind pipeline before begin rendering because of formats
	commandBuffer.bindPipeline(this->imguiPipeline);

	// Begin rendering for imgui
	commandBuffer.beginRendering(imguiRenderingInfo);

	// Record imgui primitives into it's command buffer
	ImGui_ImplVulkan_RenderDrawData(
		imguiDrawData,
		commandBuffer.getVkCommandBuffer(),
		this->imguiPipeline.getVkPipeline()
	);

	// End rendering
	commandBuffer.endRendering();
}

void Renderer::computeSortGaussians(CommandBuffer& commandBuffer)
{
	commandBuffer.bufferMemoryBarrier(
		VK_ACCESS_NONE,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		this->gaussiansSortListSBO.getVkBuffer(),
		this->gaussiansSortListSBO.getBufferSize()
	);

	// Compute pipeline
	commandBuffer.bindPipeline(this->sortGaussiansPipeline);

	// Binding 0
	VkDescriptorBufferInfo inputOutputGaussiansSortInfo{};
	inputOutputGaussiansSortInfo.buffer = this->gaussiansSortListSBO.getVkBuffer();
	inputOutputGaussiansSortInfo.range = this->gaussiansSortListSBO.getBufferSize();

	// Descriptor set
	std::array<VkWriteDescriptorSet, 1> computeWriteDescriptorSets
	{
		DescriptorSet::writeBuffer(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputOutputGaussiansSortInfo)
	};
	commandBuffer.pushDescriptorSet(
		this->sortGaussiansPipelineLayout,
		0,
		uint32_t(computeWriteDescriptorSets.size()),
		computeWriteDescriptorSets.data()
	);

	// Make sure number of elements is power of two
	assert((uint32_t) (this->numSortElements & (this->numSortElements - 1)) == 0u);

	uint32_t h = BMS_WORK_GROUP_SIZE /* 2*/;

	this->dispatchBms(
		commandBuffer, 
		BmsSubAlgorithm::LOCAL_BMS, 
		h
	);

	h *= 2;

	for ( ; h <= this->numSortElements; h *= 2)
	{
		this->dispatchBms(
			commandBuffer,
			BmsSubAlgorithm::BIG_FLIP,
			h
		);

		for (uint32_t hh = h / 2; hh > 1; hh /= 2)
		{
			if (hh <= BMS_WORK_GROUP_SIZE /* 2*/)
			{
				this->dispatchBms(
					commandBuffer,
					BmsSubAlgorithm::LOCAL_DISPERSE,
					hh
				);
				break;
			}
			else
			{
				this->dispatchBms(
					commandBuffer,
					BmsSubAlgorithm::BIG_DISPERSE,
					hh
				);
			}
		}
	}
}

void Renderer::computeRenderGaussians(
	CommandBuffer& commandBuffer,
	uint32_t imageIndex)
{
	// Transition HDR and swapchain image
	VkImageMemoryBarrier2 renderGaussiansMemoryBarriers[2] =
	{
		// HDR
		PipelineBarrier::imageMemoryBarrier2(
			VK_ACCESS_SHADER_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_IMAGE_LAYOUT_GENERAL,
			VK_IMAGE_LAYOUT_GENERAL,
			this->swapchain.getHdrTexture().getVkImage(),
			VK_IMAGE_ASPECT_COLOR_BIT
		),

		// Swapchain image
		PipelineBarrier::imageMemoryBarrier2(
			VK_ACCESS_NONE,
			VK_ACCESS_SHADER_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_GENERAL,
			this->swapchain.getVkImage(imageIndex),
			VK_IMAGE_ASPECT_COLOR_BIT
		)
	};
	commandBuffer.imageMemoryBarrier(renderGaussiansMemoryBarriers, 2);

	// Compute pipeline
	commandBuffer.bindPipeline(this->renderGaussiansPipeline);

	// Binding 0
	VkDescriptorBufferInfo inputGaussiansInfo{};
	inputGaussiansInfo.buffer = this->gaussiansSBO.getVkBuffer();
	inputGaussiansInfo.range = this->gaussiansSBO.getBufferSize();

	// Binding 1
	VkDescriptorBufferInfo inputCamUboInfo{};
	inputCamUboInfo.buffer = this->gaussiansCamUBO.getVkBuffer();
	inputCamUboInfo.range = this->gaussiansCamUBO.getBufferSize();

	// Binding 2
	VkDescriptorImageInfo inputImageInfo{};
	inputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	inputImageInfo.imageView = this->swapchain.getHdrTexture().getVkImageView();

	// Binding 3
	VkDescriptorImageInfo outputImageInfo{};
	outputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	outputImageInfo.imageView = this->swapchain.getVkImageView(imageIndex);

	// Descriptor set
	std::array<VkWriteDescriptorSet, 4> computeWriteDescriptorSets
	{
		DescriptorSet::writeBuffer(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inputGaussiansInfo),

		DescriptorSet::writeBuffer(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &inputCamUboInfo),

		DescriptorSet::writeImage(2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, &inputImageInfo),
		DescriptorSet::writeImage(3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, &outputImageInfo)
	};
	commandBuffer.pushDescriptorSet(
		this->renderGaussiansPipelineLayout,
		0,
		uint32_t(computeWriteDescriptorSets.size()),
		computeWriteDescriptorSets.data()
	);

	// Push constant
	RenderGaussiansPCD renderGaussiansPcData{};
	renderGaussiansPcData.resolution =
		glm::uvec4(
			this->swapchain.getVkExtent().width,
			this->swapchain.getVkExtent().height,
			this->numGaussians,
			0u
		);
	commandBuffer.pushConstant(
		this->renderGaussiansPipelineLayout,
		(void*)&renderGaussiansPcData
	);

	// Run compute shader
	commandBuffer.dispatch(
		(this->swapchain.getVkExtent().width + 16 - 1) / 16,
		(this->swapchain.getVkExtent().height + 16 - 1) / 16
	);

	// Transition swapchain image layout for presentation
	commandBuffer.imageMemoryBarrier(
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_NONE,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		this->swapchain.getVkImage(imageIndex),
		VK_IMAGE_ASPECT_COLOR_BIT
	);
}

void Renderer::dispatchBms(CommandBuffer& commandBuffer, BmsSubAlgorithm subAlgorithm, uint32_t h)
{
	// Push constant
	SortGaussiansPCD sortGaussiansPcData{};
	sortGaussiansPcData.data.x = static_cast<uint32_t>(subAlgorithm);
	sortGaussiansPcData.data.y = h;
	commandBuffer.pushConstant(
		this->sortGaussiansPipelineLayout,
		(void*)&sortGaussiansPcData
	);

	// Run compute shader
	commandBuffer.dispatch(
		this->numSortElements / BMS_WORK_GROUP_SIZE
	);

	commandBuffer.bufferMemoryBarrier(
		VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		this->gaussiansSortListSBO.getVkBuffer(),
		this->gaussiansSortListSBO.getBufferSize()
	);
}
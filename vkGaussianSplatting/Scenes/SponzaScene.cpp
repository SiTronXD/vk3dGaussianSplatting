#include "pch.h"
#include "SponzaScene.h"
#include "../Engine/ResourceManager.h"
#include "../Engine/Graphics/Renderer.h"

SponzaScene::SponzaScene() {}

SponzaScene::~SponzaScene() {}

void SponzaScene::init()
{
	this->camera.init(this->getWindow());

	// Set skybox cube map
	this->getRenderer().setSkyboxTexture(
		this->getResourceManager().addCubeMap({ "Resources/Textures/grace_cross.hdr" })
	);

	// Assets
	uint32_t brdfPinkFabric = this->getResourceManager().addBRDF("Resources/BRDFs/pink-fabric.shbrdf");
	uint32_t brdfRed = this->getResourceManager().addBRDF("Resources/BRDFs/red-fabric.shbrdf");
	uint32_t brdfGreen = this->getResourceManager().addBRDF("Resources/BRDFs/green-acrylic.shbrdf");
	uint32_t brdfBlue = this->getResourceManager().addBRDF("Resources/BRDFs/blue-fabric.shbrdf");
	uint32_t brdfPlant = this->getResourceManager().addBRDF("Resources/BRDFs/green-plastic.shbrdf");
	uint32_t brdfMetal = this->getResourceManager().addBRDF("Resources/BRDFs/silver-metallic-paint.shbrdf");

	uint32_t whiteTextureId = this->getResourceManager().addTexture("Resources/Textures/white.png");

	// Sponza entity
	{
		uint32_t sponzaEntity = this->createEntity();
		this->setComponent<MeshComponent>(sponzaEntity, MeshComponent());
		this->setComponent<Material>(sponzaEntity, Material());

		Transform& transform = this->getComponent<Transform>(sponzaEntity);
		transform.modelMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));

		Material& material = this->getComponent<Material>(sponzaEntity);
		material.albedoTextureId = whiteTextureId;

		MeshComponent& modelMesh = this->getComponent<MeshComponent>(sponzaEntity);
		modelMesh.meshId = 
			this->getResourceManager().addMesh("Resources/Models/sponzaSmall.obj", material);

		// Setup materials
		MaterialSet& sponzaMatSet = this->getResourceManager().getMaterialSet(material.materialSetIndex);
		SubmeshMaterial submeshMaterial{};
		submeshMaterial.brdfIndex = brdfRed;
		sponzaMatSet.applySubmeshMaterial(7, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(14, submeshMaterial);

		sponzaMatSet.applySubmeshMaterial(97, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(279, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(280, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(281, submeshMaterial);

		sponzaMatSet.applySubmeshMaterial(100, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(277, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(287, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(288, submeshMaterial);

		submeshMaterial.brdfIndex = brdfGreen;
		sponzaMatSet.applySubmeshMaterial(2, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(12, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(8, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(13, submeshMaterial);

		submeshMaterial.brdfIndex = brdfBlue;
		sponzaMatSet.applySubmeshMaterial(1, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(11, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(9, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(10, submeshMaterial);

		submeshMaterial.brdfIndex = brdfPlant;
		sponzaMatSet.applySubmeshMaterial(141, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(157, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(283, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(357, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(145, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(284, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(352, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(353, submeshMaterial);

		submeshMaterial.brdfIndex = brdfMetal;
		sponzaMatSet.applySubmeshMaterial(47, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(229, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(322, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(329, submeshMaterial);

		sponzaMatSet.applySubmeshMaterial(275, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(297, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(318, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(324, submeshMaterial);

		sponzaMatSet.applySubmeshMaterial(325, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(327, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(328, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(368, submeshMaterial);

		sponzaMatSet.applySubmeshMaterial(50, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(51, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(53, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(380, submeshMaterial);

		sponzaMatSet.applySubmeshMaterial(222, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(223, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(225, submeshMaterial);
		sponzaMatSet.applySubmeshMaterial(228, submeshMaterial);
	}

	// Initial camera setup
	this->camera.setPosition(glm::vec3(6.0f, 1.0f, -0.15f));
	this->camera.setRotation(-SMath::PI * 0.5f, -SMath::PI * 0.0f);

	// Initial light setup
	this->getRenderer().setSpotlightOrientation(
		glm::vec3(2.0f, 1.0f, 0.0f),
		glm::vec3(0.0f)
	);

	// Camera setup for screenshot
	/*this->camera.setPosition(glm::vec3(4.216015, 2.324934, -0.612709));
	this->camera.setRotation(-1.340796, -0.364000);*/
}

void SponzaScene::update()
{
	this->camera.update();
}

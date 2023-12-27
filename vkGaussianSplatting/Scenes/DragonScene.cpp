#include "pch.h"
#include "DragonScene.h"
#include "../Engine/ResourceManager.h"
#include "../Engine/Graphics/Renderer.h"

DragonScene::DragonScene()
{
}

DragonScene::~DragonScene()
{
}

void DragonScene::init()
{
	this->camera.init(this->getWindow());

	// Set skybox cube map
	this->getRenderer().setSkyboxTexture(
		this->getResourceManager().addCubeMap({ "Resources/Textures/grace_cross.hdr" })
	);

	// Assets
	uint32_t brdfWhiteMarble = this->getResourceManager().addBRDF("Resources/BRDFs/white-marble.shbrdf");
	uint32_t brdfRedPhenolic = this->getResourceManager().addBRDF("Resources/BRDFs/red-phenolic.shbrdf");
	uint32_t brdfRedFabric2 = this->getResourceManager().addBRDF("Resources/BRDFs/red-fabric2.shbrdf");
	uint32_t brdfMetal = this->getResourceManager().addBRDF("Resources/BRDFs/silver-metallic-paint.shbrdf");
	uint32_t brdfBlue = this->getResourceManager().addBRDF("Resources/BRDFs/blue-metallic-paint.shbrdf");
	uint32_t brdfBlueFabric = this->getResourceManager().addBRDF("Resources/BRDFs/blue-fabric.shbrdf");
	uint32_t brdfBlueRubber = this->getResourceManager().addBRDF("Resources/BRDFs/blue-rubber.shbrdf");
	uint32_t brdfAlumBronze = this->getResourceManager().addBRDF("Resources/BRDFs/alum-bronze.shbrdf");
	uint32_t brdfAluminaOxide = this->getResourceManager().addBRDF("Resources/BRDFs/alumina-oxide.shbrdf");

	uint32_t whiteTextureId = this->getResourceManager().addTexture("Resources/Textures/white.png");

	// Scene entity
	{
		uint32_t sceneEntity = this->createEntity();
		this->setComponent<MeshComponent>(sceneEntity, MeshComponent());
		this->setComponent<Material>(sceneEntity, Material());

		Transform& transform = this->getComponent<Transform>(sceneEntity);
		transform.modelMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));

		Material& material = this->getComponent<Material>(sceneEntity);
		material.albedoTextureId = whiteTextureId;

		MeshComponent& modelMesh = this->getComponent<MeshComponent>(sceneEntity);
		modelMesh.meshId =
			this->getResourceManager().addMesh("Resources/Models/sponzaSmall.obj", material);

		// Setup materials
		MaterialSet& matSet = this->getResourceManager().getMaterialSet(material.materialSetIndex);
		SubmeshMaterial submeshMaterial{};
		submeshMaterial.brdfIndex = brdfAluminaOxide;
		matSet.applySubmeshMaterial(21, submeshMaterial);

		submeshMaterial.brdfIndex = brdfAlumBronze;
		matSet.applySubmeshMaterial(31, submeshMaterial);
		matSet.applySubmeshMaterial(32, submeshMaterial);
		matSet.applySubmeshMaterial(70, submeshMaterial);
		matSet.applySubmeshMaterial(71, submeshMaterial);
		matSet.applySubmeshMaterial(161, submeshMaterial);
		matSet.applySubmeshMaterial(183, submeshMaterial);
		matSet.applySubmeshMaterial(184, submeshMaterial);
		matSet.applySubmeshMaterial(185, submeshMaterial);
		matSet.applySubmeshMaterial(186, submeshMaterial);
		matSet.applySubmeshMaterial(187, submeshMaterial);
		matSet.applySubmeshMaterial(188, submeshMaterial);
		matSet.applySubmeshMaterial(189, submeshMaterial);

		submeshMaterial.brdfIndex = brdfRedFabric2;
		matSet.applySubmeshMaterial(182, submeshMaterial);

		submeshMaterial.brdfIndex = brdfBlueFabric;
		matSet.applySubmeshMaterial(155, submeshMaterial);
	}

	// Dragon
	{
		uint32_t dragonEntity = this->createEntity();
		this->setComponent<MeshComponent>(dragonEntity, MeshComponent());
		this->setComponent<Material>(dragonEntity, Material());

		Transform& transform = this->getComponent<Transform>(dragonEntity);
		transform.modelMat =
			glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, -0.1f)) * 
			glm::rotate(glm::mat4(1.0f), (-0.5f + 0.17f) * SMath::PI, glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::scale(glm::mat4(1.0f), glm::vec3(0.9f));

		Material& material = this->getComponent<Material>(dragonEntity);
		material.albedoTextureId = whiteTextureId;

		MeshComponent& modelMesh = this->getComponent<MeshComponent>(dragonEntity);
		modelMesh.meshId =
			this->getResourceManager().addMesh("Resources/Models/dragon_vrip.obj", material, false);

		// Setup materials
		MaterialSet& matSet = this->getResourceManager().getMaterialSet(material.materialSetIndex);
		SubmeshMaterial submeshMaterial{};
		submeshMaterial.brdfIndex = brdfBlueRubber;
		matSet.applySubmeshMaterial(0, submeshMaterial);

		submeshMaterial.brdfIndex = brdfRedPhenolic;
		matSet.applySubmeshMaterial(1, submeshMaterial);

		submeshMaterial.brdfIndex = brdfBlue;
		matSet.applySubmeshMaterial(2, submeshMaterial);

		submeshMaterial.brdfIndex = brdfMetal;
		matSet.applySubmeshMaterial(3, submeshMaterial);

		submeshMaterial.brdfIndex = brdfWhiteMarble;
		matSet.applySubmeshMaterial(4, submeshMaterial);
	}

	// Initial camera setup
	this->camera.setPosition(glm::vec3(-0.5f, 0.76f, -0.15f));
	this->camera.setRotation(-SMath::PI * 1.5f, -SMath::PI * 0.05f);

	// Initial light setup
	this->getRenderer().setSpotlightOrientation(
		glm::vec3(0.2f, 2.5f, -0.3f),
		glm::vec3(1.0f, 0.0f, -0.1f)
	);
}

void DragonScene::update()
{
	this->camera.update();
}

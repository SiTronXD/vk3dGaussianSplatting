#include "pch.h"
#include "ManyMeshesScene.h"
#include "../Engine/ResourceManager.h"
#include "../Engine/Graphics/Renderer.h"

ManyMeshesScene::ManyMeshesScene()
{
}

ManyMeshesScene::~ManyMeshesScene()
{
}

void ManyMeshesScene::init()
{
	this->camera.init(this->getWindow());

	// Set skybox cube map
	this->getRenderer().setSkyboxTexture(
		this->getResourceManager().addCubeMap({ "Resources/Textures/grace_cross.hdr" })
	);

	// Assets
	uint32_t brdfPinkFabric = this->getResourceManager().addBRDF("Resources/BRDFs/pink-fabric.shbrdf");
	std::vector<uint32_t> brdfs = 
	{
		brdfPinkFabric,
		this->getResourceManager().addBRDF("Resources/BRDFs/red-phenolic.shbrdf"),
		this->getResourceManager().addBRDF("Resources/BRDFs/red-fabric2.shbrdf"),
		this->getResourceManager().addBRDF("Resources/BRDFs/silver-metallic-paint.shbrdf"),
		this->getResourceManager().addBRDF("Resources/BRDFs/blue-metallic-paint.shbrdf"),
		this->getResourceManager().addBRDF("Resources/BRDFs/blue-fabric.shbrdf"),
		this->getResourceManager().addBRDF("Resources/BRDFs/blue-rubber.shbrdf")
	};

	uint32_t whiteTextureId = this->getResourceManager().addTexture("Resources/Textures/white.png");

	// Ground
	{
		uint32_t groundEntity = this->createEntity();
		this->setComponent<MeshComponent>(groundEntity, MeshComponent());
		this->setComponent<Material>(groundEntity, Material());

		Transform& transform = this->getComponent<Transform>(groundEntity);
		transform.modelMat =
			glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f)) * 
			glm::scale(glm::mat4(1.0f), glm::vec3(this->BOUNDS_SIZE, 0.5f, this->BOUNDS_SIZE));

		Material& material = this->getComponent<Material>(groundEntity);
		material.albedoTextureId = whiteTextureId;

		MeshComponent& modelMesh = this->getComponent<MeshComponent>(groundEntity);
		modelMesh.meshId =
			this->getResourceManager().addMesh("Resources/Models/box.obj", material, false);
	}

	// Random objects
	for(uint32_t i = 0; i < this->NUM_MESHES; ++i)
	{
		uint32_t entity = this->createEntity();
		this->setComponent<MeshComponent>(entity, MeshComponent());
		this->setComponent<Material>(entity, Material());

		Transform& transform = this->getComponent<Transform>(entity);
		transform.modelMat =
			glm::translate(
				glm::mat4(1.0f), 
				glm::vec3(
					(float(rand() % 10000) / 10000.0f * 2.0f - 1.0f) * BOUNDS_SIZE * 0.5f,
					0.0f, 
					(float(rand() % 10000) / 10000.0f * 2.0f - 1.0f) * BOUNDS_SIZE * 0.5f
				)
			) *
			glm::rotate(glm::mat4(1.0f), SMath::PI * 0.5f, glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));

		Material& material = this->getComponent<Material>(entity);
		material.albedoTextureId = whiteTextureId;

		MeshComponent& modelMesh = this->getComponent<MeshComponent>(entity);
		modelMesh.meshId =
			this->getResourceManager().addMesh("Resources/Models/sphereTest.obj", material, false);

		// Setup random material
		material.materialSetIndex = this->getResourceManager().addMaterial(
			this->getResourceManager().getMesh(modelMesh.meshId)
		);
		MaterialSet& matSet = this->getResourceManager().getMaterialSet(material.materialSetIndex);
		SubmeshMaterial submeshMaterial{};
		submeshMaterial.brdfIndex = brdfs[rand() % int(brdfs.size())];
		matSet.applyUniformSubmeshMaterial(submeshMaterial);
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

void ManyMeshesScene::update()
{
	this->camera.update();
}

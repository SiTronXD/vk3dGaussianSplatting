#include "pch.h"
#include "TestScene.h"
#include "../Engine/ResourceManager.h"
#include "../Engine/Graphics/Renderer.h"

#include <imgui/imgui.h>

TestScene::TestScene()
{
}

TestScene::~TestScene()
{
}

void TestScene::init()
{
	this->camera.init(this->getWindow());

	uint32_t whiteTextureId = this->getResourceManager().addTexture("Resources/Textures/white.png");

	// Test entity
	{
		uint32_t testEntity = this->createEntity();
		this->setComponent<MeshComponent>(testEntity, MeshComponent());
		this->setComponent<Material>(testEntity, Material());

		Transform& transform = this->getComponent<Transform>(testEntity);
		transform.modelMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));

		Material& material = this->getComponent<Material>(testEntity);
		material.albedoTextureId = whiteTextureId;

		MeshComponent& modelMesh = this->getComponent<MeshComponent>(testEntity);
		modelMesh.meshId = this->getResourceManager().addMesh("Resources/Models/sphereTest.obj", material);
	}

	// Walls
	glm::mat4 wallTransforms[3] =
	{
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(3.0f, 1.0f, 3.0f)),
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(3.0f, 3.0f, 1.0f)),
		glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 3.0f, 3.0f))
	};
	for(uint32_t i = 0; i < 3; ++i)
	{
		uint32_t testEntity = this->createEntity();
		this->setComponent<MeshComponent>(testEntity, MeshComponent());
		this->setComponent<Material>(testEntity, Material());

		Transform& transform = this->getComponent<Transform>(testEntity);
		transform.modelMat = wallTransforms[i];

		Material& material = this->getComponent<Material>(testEntity);
		material.albedoTextureId = whiteTextureId;

		MeshComponent& modelMesh = this->getComponent<MeshComponent>(testEntity);
		modelMesh.meshId = this->getResourceManager().addMesh("Resources/Models/box.obj", material);
	}

	// Initial camera setup
	this->camera.setPosition(glm::vec3(-1.0f, 0.5f, 1.0f));
	this->camera.setRotation(SMath::PI * 1.0f, -SMath::PI * 0.1f);
}

void TestScene::update()
{
	this->camera.update();
}

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

	// Initial camera setup
	this->camera.setPosition(glm::vec3(-1.0f, 0.5f, 1.0f));
	this->camera.setRotation(SMath::PI * 1.0f, -SMath::PI * 0.1f);
}

void TestScene::update()
{
	this->camera.update();
}

#include "pch.h"
#include "TestScene.h"
#include "../Engine/ResourceManager.h"
#include "../Engine/Graphics/Renderer.h"
#include "../Engine/Dev/StrHelper.h"

TestScene::TestScene()
{ }

TestScene::~TestScene()
{ }

void TestScene::init()
{
	this->camera.init(this->getWindow());

	// Initial camera setup
	this->camera.setPosition(glm::vec3(-1.0f, 0.5f, 1.0f));
	this->camera.setRotation(SMath::PI * 1.0f, -SMath::PI * 0.1f);

	// Used for benchmarks
	this->camera.setPosition(glm::vec3(-0.910660, -0.016507, 0.872644));
	this->camera.setRotation(1.731593, -0.404159);
}

void TestScene::update()
{
	this->camera.update();

	/*Log::write("Cam pos: " + StrHelper::vecToStr(this->camera.getPosition()));
	Log::write("Cam rot: (" + std::to_string(this->camera.getYaw()) + ", " + std::to_string(this->camera.getPitch()) + ")");*/
}

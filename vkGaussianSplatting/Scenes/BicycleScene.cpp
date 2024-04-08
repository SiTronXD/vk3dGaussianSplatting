#include "pch.h"
#include "BicycleScene.h"
#include "../Engine/Dev/StrHelper.h"
#include "../Engine/ResourceManager.h"

void BicycleScene::init()
{
	this->camera.init(this->getWindow());

	// Camera for benchmarks
	this->camera.setPosition(glm::vec3(0.945927, -0.294418, -0.181088));
	this->camera.setRotation(-1.108407, -0.324159);
	

	// Load gaussians from file
	this->getResourceManager().loadGaussians("D:/DownloadedAssets/GaussianFiles/bicycle/point_cloud/iteration_7000/point_cloud.ply");
}

void BicycleScene::update()
{
	this->camera.update();

	/*Log::write("Cam pos: " + StrHelper::vecToStr(this->camera.getPosition()));
	Log::write("Cam rot: (" + std::to_string(this->camera.getYaw()) + ", " + std::to_string(this->camera.getPitch()) + ")");*/
}
#include "pch.h"
#include "GardenScene.h"
#include "../Engine/Dev/StrHelper.h"
#include "../Engine/ResourceManager.h"

void GardenScene::init()
{
	this->camera.init(this->getWindow());

	// Camera for benchmarks
	this->camera.setPosition(glm::vec3(-0.620010, 0.189628, 2.271181));
	this->camera.setRotation(2.971590, -1.074159);

	// Load gaussians from file
	this->getResourceManager().loadGaussians("D:/DownloadedAssets/GaussianFiles/garden/point_cloud/iteration_7000/point_cloud.ply");
}

void GardenScene::update()
{
	this->camera.update();

	/*Log::write("Cam pos: " + StrHelper::vecToStr(this->camera.getPosition()));
	Log::write("Cam rot: (" + std::to_string(this->camera.getYaw()) + ", " + std::to_string(this->camera.getPitch()) + ")");*/
}
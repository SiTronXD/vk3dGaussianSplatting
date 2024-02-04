#include "pch.h"
#include "TrainScene.h"
#include "../Engine/Dev/StrHelper.h"
#include "../Engine/ResourceManager.h"

void TrainScene::init()
{
	this->camera.init(this->getWindow());

	// Camera for benchmarks
	this->camera.setPosition(glm::vec3(-2.857887, 0.188856, 1.048745));
	this->camera.setRotation(1.361593, 0.005841);


	// Load gaussians from file
	this->getResourceManager().loadGaussians("D:/DownloadedAssets/GaussianFiles/train/point_cloud/iteration_7000/point_cloud.ply");
}

void TrainScene::update()
{
	this->camera.update();

	Log::write("Cam pos: " + StrHelper::vecToStr(this->camera.getPosition()));
	Log::write("Cam rot: (" + std::to_string(this->camera.getYaw()) + ", " + std::to_string(this->camera.getPitch()) + ")");
}
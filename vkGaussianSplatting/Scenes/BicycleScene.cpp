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

	// Animation showing specular effect from SH
	/*float t = std::sin(Time::getTimeSinceStart()) * 0.5f + 0.5f;
	glm::vec3 newPos = glm::mix(glm::vec3(0.017753, -0.159444, 1.916727), glm::vec3(-0.959038, -0.159444, 1.328299), t);
	glm::vec2 newRot = glm::mix(glm::vec2(-2.848421, -0.774159), glm::vec2(-4.708428, -0.554159), t);
	this->camera.setPosition(newPos);
	this->camera.setRotation(newRot.x, newRot.y);*/

	// Animation showing SH coefficient heatmap
	/*float t = Time::getTimeSinceStart() * 0.4f;
	glm::vec3 newPos = 
		glm::vec3(3.817537 + -4.389514, 0.978171 + 0.838488, 0.521623 + 0.526775) * 0.5f + 
		glm::vec3(sin(t), 0.0f, cos(t)) * -4.5f;
	glm::vec2 newRot(t, -0.384159);
	this->camera.setPosition(newPos);
	this->camera.setRotation(newRot.x, newRot.y);*/

	/*Log::write("Cam pos: " + StrHelper::vecToStr(this->camera.getPosition()));
	Log::write("Cam rot: (" + std::to_string(this->camera.getYaw()) + ", " + std::to_string(this->camera.getPitch()) + ")");*/
}
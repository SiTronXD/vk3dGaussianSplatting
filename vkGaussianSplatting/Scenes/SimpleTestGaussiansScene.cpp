#include "pch.h"
#include "SimpleTestGaussiansScene.h"
#include "../Engine/ResourceManager.h"

void SimpleTestGaussiansScene::init()
{
	this->camera.init(this->getWindow());

	// Camera for benchmarks
	this->camera.setPosition(glm::vec3(0.0f, 0.0f, 2.0f));
	this->camera.setRotation(SMath::PI, 0.0f);


	// Add gaussians in a row
	for (uint32_t i = 0; i < 16u; ++i)
	{
		GaussianData gaussian{};
		gaussian.position = glm::vec4(-8.0f + (float)i, 0.0f, -1.0f, 0.0f);
		gaussian.scale = glm::vec4(0.1f, 0.2f, 0.5f, 0.0f);
		gaussian.shCoeffs00 = glm::vec4(
			(rand() % 10000) / 10000.0f,
			(rand() % 10000) / 10000.0f,
			(rand() % 10000) / 10000.0f,
			1.0f
		);

		this->getResourceManager().addGaussian(gaussian);
	}
}

void SimpleTestGaussiansScene::update()
{
	this->camera.update();

	/*Log::write("Cam pos: " + StrHelper::vecToStr(this->camera.getPosition()));
	Log::write("Cam rot: (" + std::to_string(this->camera.getYaw()) + ", " + std::to_string(this->camera.getPitch()) + ")");*/
}
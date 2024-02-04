#include "pch.h"
#include "TestSortScene.h"
#include "../Engine/ResourceManager.h"
#include "../Engine/Graphics/Camera.h"

void TestSortScene::init()
{
	this->camera.init(this->getWindow());

	// Camera for benchmarks
	this->camera.setPosition(glm::vec3(0.0f));
	this->camera.setRotation(0.0f, 0.0f);


	// Add gaussians in front of the camera
	for (uint32_t i = 0; i < 16*3; ++i) {

		uint32_t keyDepth = (i + 1) * 1024;
		float zOffset = ((float)keyDepth / (float)4294967295u) *
			(Camera::FAR_PLANE - Camera::NEAR_PLANE) + Camera::NEAR_PLANE;

		GaussianData gaussian{};
		gaussian.position = glm::vec4((-8.0f + (float) i) * 0.01f, 0.0f, zOffset, 0.0f);
		gaussian.scale = glm::vec4(0.02f);
		gaussian.color = glm::vec4(
			(rand() % 10000) / 10000.0f,
			(rand() % 10000) / 10000.0f,
			(rand() % 10000) / 10000.0f,
			1.0f
		);

		this->getResourceManager().addGaussian(gaussian);
	}
}

void TestSortScene::update()
{
	this->camera.update();

	/*Log::write("Cam pos: " + StrHelper::vecToStr(this->camera.getPosition()));
	Log::write("Cam rot: (" + std::to_string(this->camera.getYaw()) + ", " + std::to_string(this->camera.getPitch()) + ")");*/
}
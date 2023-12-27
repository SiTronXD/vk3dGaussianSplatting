#include "pch.h"
#include "Engine.h"
#include "Graphics/Mesh.h"

void Engine::beginImgui()
{
	// Start
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// ------------- Imgui code -------------
	/*ImGui::Begin("imgui window");
	ImGui::End();*/
}

void Engine::endImgui()
{
	// End
	ImGui::Render();
}

Engine::Engine()
{
	
}

Engine::~Engine()
{
}

void Engine::init(Scene* initialScene)
{
	// Init subsystems
	this->window.init(this->renderer, "Harmonics Virtual Lights", 1280, 720);
	this->renderer.init(this->resourceManager);
	this->resourceManager.init(this->renderer.getGfxAllocContext());
	this->sceneManager.init(this->window, this->renderer, this->resourceManager);

	// Init scene
	this->sceneManager.setScene(initialScene);

	// Main loop
	Time::init();
	while (this->window.isRunning())
	{
		Time::startGodTimer();

		// Update before "game logic"
		this->window.update();
		Time::updateDeltaTime();

		this->beginImgui();

		// Scene logic
		this->sceneManager.updateToNextScene();
		this->sceneManager.update();

		this->endImgui();

		// Render
		this->renderer.draw(this->sceneManager.getCurrentScene());

#ifdef _DEBUG
		if (Input::isKeyPressed(Keys::T))
		{
			this->renderer.generateMemoryDump();
		}
#endif

#ifndef RECORD_GPU_TIMES
		// Print fps
		if (Time::hasOneSecondPassed())
			Log::write("FPS: " + std::to_string(1.0f / Time::getDT()));
#endif

		Time::endGodTimer();
	}

	// Cleanup
	this->renderer.startCleanup();
	this->sceneManager.cleanup();
	this->resourceManager.cleanup();
	this->renderer.cleanup();
}

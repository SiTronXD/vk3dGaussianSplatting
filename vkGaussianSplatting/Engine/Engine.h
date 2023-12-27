#pragma once

#include "Application/SceneManager.h"
#include "Graphics/Renderer.h"
#include "ResourceManager.h"

class Engine
{
private:
	Window window;
	Renderer renderer;
	ResourceManager resourceManager;
	SceneManager sceneManager;

	void beginImgui();
	void endImgui();

public:
	Engine();
	~Engine();

	void init(Scene* initialScene);
};
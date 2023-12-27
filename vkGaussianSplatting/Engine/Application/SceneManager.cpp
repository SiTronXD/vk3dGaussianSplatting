#include "pch.h"
#include "SceneManager.h"
#include "Scene.h"
#include "../Graphics/Renderer.h"

SceneManager::SceneManager()
	: currentScene(nullptr),
	nextScene(nullptr),
	window(nullptr),
	renderer(nullptr),
	resourceManager(nullptr)
{
}

SceneManager::~SceneManager()
{
	this->cleanup();
}

void SceneManager::init(Window& window, Renderer& renderer, ResourceManager& resourceManager)
{
	this->window = &window;
	this->renderer = &renderer;
	this->resourceManager = &resourceManager;
}

void SceneManager::setScene(Scene* newScene)
{
	// Next scene was already set
	if (this->nextScene != nullptr)
	{
		delete this->nextScene;
		this->nextScene = nullptr;
	}

	this->nextScene = newScene;
}

void SceneManager::update()
{
	this->currentScene->update();
}

void SceneManager::cleanup()
{
	delete this->nextScene;
	this->nextScene = nullptr;

	delete this->currentScene;
	this->currentScene = nullptr;
}

void SceneManager::updateToNextScene()
{
	// Scene should be switched
	if (this->nextScene != nullptr)
	{
		// Switch
		delete this->currentScene;
		this->currentScene = this->nextScene;
		this->nextScene = nullptr;

		// Init new scene
		this->currentScene->setSceneManager(*this);
		this->currentScene->init();

		// Init subsystems for the new scene
		this->renderer->initForScene(*this->currentScene);
	}
}

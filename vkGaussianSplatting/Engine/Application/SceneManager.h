#pragma once

class Engine;
class Window;
class Renderer;
class ResourceManager;
class Scene;

class SceneManager
{
private:
	friend Engine;

	Window* window;
	Renderer* renderer;
	ResourceManager* resourceManager;

	Scene* currentScene;
	Scene* nextScene;

	void updateToNextScene();

public:
	SceneManager();
	~SceneManager();

	void init(Window& window, Renderer& renderer, ResourceManager& resourceManager);

	void setScene(Scene* newScene);
	void update();
	void cleanup();

	inline Window& getWindow() const { return *this->window; }
	inline Renderer& getRenderer() const { return *this->renderer; }
	inline ResourceManager& getResourceManager() const { return *this->resourceManager; }
	inline Scene& getCurrentScene() const { return *this->currentScene; }
};
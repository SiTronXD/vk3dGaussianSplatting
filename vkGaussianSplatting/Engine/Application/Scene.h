#pragma once

#include "SceneManager.h"
#include "../Graphics/Camera.h"
#include "../Components.h"

class Scene
{
private:
	SceneManager* sceneManager;

	entt::registry registry;

protected:
	Camera camera;

	inline SceneManager& getSceneManager() const { return *this->sceneManager; }
	inline Window& getWindow() const { return this->getSceneManager().getWindow(); }
	inline Renderer& getRenderer() const { return this->getSceneManager().getRenderer(); }
	inline ResourceManager& getResourceManager() const { return this->getSceneManager().getResourceManager(); }

public:
	Scene();
	virtual ~Scene();

	void setSceneManager(SceneManager& sceneManager);

	uint32_t createEntity();

	template<typename T>
	T& getComponent(uint32_t entity);

	template<typename T>
	void setComponent(uint32_t entity, const T& component);

	virtual void init() = 0;
	virtual void update() = 0;

	inline const Camera& getCamera() const { return this->camera; }
	inline entt::registry& getRegistry() { return this->registry; }
};


template<typename T>
T& Scene::getComponent(uint32_t entity)
{
	return this->registry.get<T>((entt::entity) entity);
}

template<typename T>
void Scene::setComponent(uint32_t entity, const T& component)
{
	this->registry.emplace_or_replace<T>((entt::entity) entity, component);
}
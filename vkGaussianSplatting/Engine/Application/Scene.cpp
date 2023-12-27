#include "pch.h"
#include "Scene.h"

uint32_t Scene::createEntity()
{
	uint32_t createdEntity = uint32_t(this->registry.create());

	this->setComponent<Transform>(createdEntity, Transform());

	return createdEntity;
}

Scene::Scene()
	: sceneManager(nullptr)
{
}

Scene::~Scene()
{
}

void Scene::setSceneManager(SceneManager& sceneManager)
{
	this->sceneManager = &sceneManager;
}

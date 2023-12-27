#pragma once

#include "../Engine/Application/Scene.h"

class ManyMeshesScene : public Scene
{
private:
#ifndef _DEBUG
	const uint32_t NUM_MESHES = 100'000;
	const float BOUNDS_SIZE = 300.0f;
#else
	const uint32_t NUM_MESHES = 1000;
	const float BOUNDS_SIZE = 100.0f;
#endif

public:
	ManyMeshesScene();
	~ManyMeshesScene();

	void init() override;
	void update() override;
};
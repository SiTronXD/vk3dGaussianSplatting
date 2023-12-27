#pragma once

#include "../Engine/Application/Scene.h"

class DragonScene : public Scene
{
public:
	DragonScene();
	~DragonScene();

	void init() override;
	void update() override;
};
#pragma once

#include "../Engine/Application/Scene.h"

class SimpleScene : public Scene
{
private:
	
public:
	SimpleScene();
	~SimpleScene();

	void init() override;
	void update() override;
};
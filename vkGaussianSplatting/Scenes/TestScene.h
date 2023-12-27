#pragma once

#include "../Engine/Application/Scene.h"

class TestScene : public Scene
{
private:
	
public:
	TestScene();
	~TestScene();

	void init() override;
	void update() override;
};
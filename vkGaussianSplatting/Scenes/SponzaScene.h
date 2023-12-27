#pragma once

#include "../Engine/Application/Scene.h"

class SponzaScene : public Scene
{
private:

public:
	SponzaScene();
	~SponzaScene();

	void init() override;
	void update() override;
};
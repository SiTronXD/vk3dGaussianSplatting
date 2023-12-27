#pragma once

#include <vector>

class Renderer;

class GfxState
{
private:
	friend Renderer;

	static uint32_t currentFrameIndex;

public:
	static inline const uint32_t& getFrameIndex() { return GfxState::currentFrameIndex; }
};
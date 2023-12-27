#pragma once

#include <string>

class Renderer;

struct GLFWwindow;

class Window
{
private:
	GLFWwindow* windowHandle;

public:
	Window();
	~Window();

	void init(Renderer& renderer, const std::string& title, int width, int height);
	void update();
	void awaitEvents() const;

	void getFramebufferSize(int& widthOutput, int& heightOutput) const;
	void getInstanceExtensions(const char**& extensions, uint32_t& extensionCount);

	bool isRunning() const;

	float getAspectRatio() const;

	inline GLFWwindow* getWindowHandle() { return this->windowHandle; }
};
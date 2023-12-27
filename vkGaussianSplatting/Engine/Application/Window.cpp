#include "pch.h"
#include "Window.h"
#include "../Graphics/Renderer.h"

static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto app = reinterpret_cast<Renderer*>(
		glfwGetWindowUserPointer(window)
		);
	app->framebufferResized = true;
}

Window::Window()
	: windowHandle(nullptr)
{
	
}

Window::~Window()
{
	// GLFW
	glfwDestroyWindow(this->windowHandle);
	glfwTerminate();
}

void Window::init(Renderer& renderer, const std::string& title, int width, int height)
{
	// Set pointer
	renderer.setWindow(*this);

	// Create window
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	this->windowHandle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

	// Set position to center of monitor
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowPos(
		this->windowHandle, 
		(mode->width - width) / 2, 
		(mode->height - height) / 2
	);

	// Set pointer for resize callback
	glfwSetWindowUserPointer(this->windowHandle, &renderer);
	glfwSetFramebufferSizeCallback(this->windowHandle, framebufferResizeCallback);

	// Set for input callback
	glfwSetKeyCallback(this->windowHandle, Input::glfwKeyCallback);
	glfwSetMouseButtonCallback(this->windowHandle, Input::glfwMouseButtonCallback);
}

void Window::update()
{
	Input::updateLastKeys();

	glfwPollEvents();

	// Close when clicking escape
#ifdef _DEBUG
	if (Input::isKeyDown(Keys::ESCAPE))
	{
		glfwSetWindowShouldClose(this->windowHandle, GLFW_TRUE);
	}
#endif

	// Update cursor position
	double cursorX, cursorY;
	glfwGetCursorPos(this->windowHandle, &cursorX, &cursorY);
	Input::setCursor((float) cursorX, (float) cursorY);
}

void Window::awaitEvents() const
{
	glfwWaitEvents();
}

void Window::getFramebufferSize(int& widthOutput, int& heightOutput) const
{
	glfwGetFramebufferSize(this->windowHandle, &widthOutput, &heightOutput);
}

void Window::getInstanceExtensions(const char**& extensions, uint32_t& extensionCount)
{
	extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
}

bool Window::isRunning() const
{
	return !glfwWindowShouldClose(this->windowHandle);
}

float Window::getAspectRatio() const
{
	int width, height;
	this->getFramebufferSize(width, height);

	return float(width) / float(height);
}

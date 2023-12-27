#include "pch.h"
#include "Input.h"

bool Input::keyDown[GLFW_MAX_NUM_KEYS]{};
bool Input::lastKeyDown[GLFW_MAX_NUM_KEYS]{};
bool Input::mouseButtonDown[GLFW_MAX_NUM_MOUSE_BUTTONS]{};

float Input::cursorX = 0.0f;
float Input::cursorY = 0.0f;
float Input::lastCursorX = 0.0f;
float Input::lastCursorY = 0.0f;

void Input::setCursor(const float& newCursorX, const float& newCursorY)
{
	// Update last position
	Input::lastCursorX = Input::cursorX;
	Input::lastCursorY = Input::cursorY;

	// Update current position
	Input::cursorX = newCursorX;
	Input::cursorY = newCursorY;
}

void Input::updateLastKeys()
{
	for (int i = 0; i < GLFW_MAX_NUM_KEYS; ++i)
		lastKeyDown[i] = keyDown[i];
}

void Input::glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Input::keyDown[key] = (action != GLFW_RELEASE);
}

void Input::glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	Input::mouseButtonDown[button] = (action == GLFW_PRESS);
}

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLFW_MAX_NUM_KEYS (GLFW_KEY_LAST + 1)
#define GLFW_MAX_NUM_MOUSE_BUTTONS (GLFW_MOUSE_BUTTON_8 + 1)

enum class Keys
{
	A = GLFW_KEY_A,
	B = GLFW_KEY_B,
	C = GLFW_KEY_C,
	D = GLFW_KEY_D,
	E = GLFW_KEY_E,
	F = GLFW_KEY_F,
	G = GLFW_KEY_G,
	H = GLFW_KEY_H,
	I = GLFW_KEY_I,
	J = GLFW_KEY_J,
	K = GLFW_KEY_K,
	L = GLFW_KEY_L,
	M = GLFW_KEY_M,
	N = GLFW_KEY_N,
	O = GLFW_KEY_O,
	P = GLFW_KEY_P,
	Q = GLFW_KEY_Q,
	R = GLFW_KEY_R,
	S = GLFW_KEY_S,
	T = GLFW_KEY_T,
	U = GLFW_KEY_U,
	V = GLFW_KEY_V,
	W = GLFW_KEY_W,
	X = GLFW_KEY_X,
	Y = GLFW_KEY_Y,
	Z = GLFW_KEY_Z,

	NUM_0 = GLFW_KEY_0,
	NUM_1 = GLFW_KEY_1,
	NUM_2 = GLFW_KEY_2,
	NUM_3 = GLFW_KEY_3,
	NUM_4 = GLFW_KEY_4,
	NUM_5 = GLFW_KEY_5,
	NUM_6 = GLFW_KEY_6,
	NUM_7 = GLFW_KEY_7,
	NUM_8 = GLFW_KEY_8,
	NUM_9 = GLFW_KEY_9,

	LEFT_SHIFT = GLFW_KEY_LEFT_SHIFT,
	ESCAPE = GLFW_KEY_ESCAPE,
};

enum class Mouse
{
	LEFT_BUTTON = GLFW_MOUSE_BUTTON_LEFT,
	RIGHT_BUTTON = GLFW_MOUSE_BUTTON_RIGHT
};

class Input
{
private:
	friend class Window;

	static bool keyDown[GLFW_MAX_NUM_KEYS];
	static bool lastKeyDown[GLFW_MAX_NUM_KEYS];
	static bool mouseButtonDown[GLFW_MAX_NUM_MOUSE_BUTTONS];

	static float cursorX;
	static float cursorY;
	static float lastCursorX;
	static float lastCursorY;

	static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

	static void setCursor(const float& newCursorX, const float& newCursorY);
	static void updateLastKeys();

public:
	static inline bool isKeyDown(const Keys& key) { return Input::keyDown[(int) key]; }
	static inline bool isKeyPressed(const Keys& key) { return Input::keyDown[(int)key] && !Input::lastKeyDown[(int)key]; }
	static inline bool isMouseButtonDown(const Mouse& mouse) { return mouseButtonDown[(int) mouse]; }
	static inline const float getMouseDeltaX() { return Input::lastCursorX - Input::cursorX; }
	static inline const float getMouseDeltaY() { return Input::lastCursorY - Input::cursorY; }
};
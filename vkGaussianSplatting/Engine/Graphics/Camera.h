#pragma once

#include "../SMath.h"

class Window;

class Camera
{
private:
	const float MOVEMENT_SPEED = 2.0f;

	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;

	glm::vec3 position;
	glm::vec3 forwardDir;
	glm::vec3 rightDir;
	glm::vec3 upDir;

	float yaw;
	float pitch;

	const Window* window;

	void updateDirVectors();
	void updateMatrices();

	void recalculate();

public:
	Camera();
	~Camera();

	void init(const Window& window);
	void update();

	void setPosition(const glm::vec3& newPos);
	void setRotation(float yaw, float pitch);

	inline const glm::mat4& getViewMatrix() const { return this->viewMatrix; }
	inline const glm::mat4& getProjectionMatrix() const { return this->projectionMatrix; }

	inline const glm::vec3& getPosition() const { return this->position; }
};
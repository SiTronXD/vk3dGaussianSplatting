#pragma once

#include "../SMath.h"

class Window;

enum class SphericalHarmonicsMode : uint32_t
{
	ALL_BANDS = 0,
	SKIP_FIRST_BAND = 1,
	ONLY_FIRST_BAND = 2
};

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

	SphericalHarmonicsMode shMode;

	const Window* window;

	void updateDirVectors();
	void updateMatrices();

	void recalculate();

public:
	const static float NEAR_PLANE;
	const static float FAR_PLANE;

	Camera();
	~Camera();

	void init(const Window& window);
	void update();

	void setPosition(const glm::vec3& newPos);
	void setRotation(float yaw, float pitch);

	inline float getYaw() const { return this->yaw; }
	inline float getPitch() const { return this->pitch; }

	inline SphericalHarmonicsMode getShMode() const { return this->shMode; }

	inline const glm::mat4& getViewMatrix() const { return this->viewMatrix; }
	inline const glm::mat4& getProjectionMatrix() const { return this->projectionMatrix; }

	inline const glm::vec3& getPosition() const { return this->position; }
};
#include "pch.h"
#include "Camera.h"

const float Camera::NEAR_PLANE = 0.1f;
const float Camera::FAR_PLANE = 100.0f;

void Camera::updateDirVectors()
{
	// Forward direction
	this->forwardDir = glm::vec3(
		(float) (sin(this->yaw) * cos(this->pitch)),
		(float) sin(this->pitch),
		(float) (cos(this->yaw) * cos(this->pitch))
	);

	this->forwardDir = glm::normalize(this->forwardDir);

	// Right direction
	this->rightDir = glm::cross(this->forwardDir, glm::vec3(0.0f, 1.0f, 0.0f));
	this->rightDir = glm::normalize(this->rightDir);

	// Up direction
	this->upDir = glm::cross(this->rightDir, this->forwardDir);
	this->upDir = glm::normalize(this->upDir);
}

void Camera::updateMatrices()
{
	// Get framebuffer size, to avoid division by 0 in aspect ratio
	int framebufferWidth, framebufferHeight;
	this->window->getFramebufferSize(framebufferWidth, framebufferHeight);

	// View/projection matrices
	this->viewMatrix = glm::lookAt(
		this->position,
		this->position + this->forwardDir,
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
	if (framebufferHeight != 0)
	{
		this->projectionMatrix = glm::perspective(
			glm::radians(90.0f),
			this->window->getAspectRatio(),
			Camera::NEAR_PLANE,
			Camera::FAR_PLANE
		);
	}
}

void Camera::recalculate()
{
	this->updateDirVectors();
	this->updateMatrices();
}

Camera::Camera()
	: window(nullptr),
	viewMatrix(glm::mat4(1.0f)),
	projectionMatrix(glm::mat4(1.0f)),

	position(0.0f, 0.0f, 2.0f),
	forwardDir(-1.0f, -1.0f, -1.0f),
	rightDir(-1.0f, -1.0f, -1.0f),
	upDir(-1.0f, -1.0f, -1.0f),
	
	yaw(SMath::PI),
	pitch(0.0f),
	shMode(SphericalHarmonicsMode::ALL_BANDS)
{
	
}

Camera::~Camera()
{
}

void Camera::init(const Window& window)
{
	this->window = &window;
}

void Camera::update()
{
	// Spherical harmonics mode
	if (Input::isKeyPressed(Keys::NUM_1))
	{
		this->shMode = SphericalHarmonicsMode::ALL_BANDS;
		Log::write("------------------------------------------");
		Log::write("Use spherical harmonic bands 0-3 (default)");
		Log::write("------------------------------------------");
	}
	else if (Input::isKeyPressed(Keys::NUM_2))
	{
		this->shMode = SphericalHarmonicsMode::SKIP_FIRST_BAND;
		Log::write("------------------------------------------------------------------------------------");
		Log::write("Use spherical harmonic bands 1-3 (visualize only the effect of view-dependent color)");
		Log::write("------------------------------------------------------------------------------------");
	}
	else if (Input::isKeyPressed(Keys::NUM_3))
	{
		this->shMode = SphericalHarmonicsMode::ONLY_FIRST_BAND;
		Log::write("--------------------------------------------------------------------");
		Log::write("Use only spherical harmonic band 0 (constant view-independent color)");
		Log::write("--------------------------------------------------------------------");
	}

	// Keyboard input
	float rightSpeed =
		(float) (Input::isKeyDown(Keys::D) - Input::isKeyDown(Keys::A));
	float forwardSpeed =
		(float) (Input::isKeyDown(Keys::W) - Input::isKeyDown(Keys::S));
	float upSpeed =
		(float) (Input::isKeyDown(Keys::E) - Input::isKeyDown(Keys::Q));
	float sprintSpeed =
		(float) Input::isKeyDown(Keys::LEFT_SHIFT) * 2.0f + 1.0f;

	// Move position
	this->position += 
		(rightSpeed * this->rightDir +
		forwardSpeed * this->forwardDir +
		upSpeed * this->upDir) * this->MOVEMENT_SPEED * sprintSpeed * Time::getDT();

	// Mouse input
	if (Input::isMouseButtonDown(Mouse::RIGHT_BUTTON))
	{
		this->yaw += Input::getMouseDeltaX() * 0.01f;
		this->pitch += Input::getMouseDeltaY() * 0.01f;
	}

	this->recalculate();
}

void Camera::setPosition(const glm::vec3& newPos)
{
	this->position = newPos;
}

void Camera::setRotation(float yaw, float pitch)
{
	this->yaw = yaw;
	this->pitch = pitch;
}

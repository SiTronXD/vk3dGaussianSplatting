#include "pch.h"
#include "Time.h"

float Time::deltaTime = 0.0f;
float Time::timeSinceStart = 0.0f;
bool Time::oneSecondPassed = false;

std::chrono::system_clock::time_point Time::lastTime = std::chrono::system_clock::time_point();
std::chrono::system_clock::time_point Time::currentTime = std::chrono::system_clock::time_point();
std::chrono::duration<float> Time::elapsedSeconds = std::chrono::duration<float>();

std::chrono::system_clock::time_point Time::userLastTime = std::chrono::system_clock::time_point();
std::chrono::duration<float> Time::userElapsedSeconds = std::chrono::duration<float>();
std::chrono::system_clock::time_point Time::godUserLastTime = std::chrono::system_clock::time_point();
std::chrono::duration<float> Time::godUserElapsedSeconds = std::chrono::duration<float>();
float Time::godUserTime = 0.0f;

void Time::init()
{
	lastTime = std::chrono::system_clock::now();
	updateDeltaTime();
}

void Time::updateDeltaTime()
{
	// Update elapsed time
	currentTime = std::chrono::system_clock::now();
	elapsedSeconds = currentTime - lastTime;
	lastTime = currentTime;

	// Update delta time
	deltaTime = elapsedSeconds.count();

	// Update time since start
	unsigned int beforeTime = (unsigned int) timeSinceStart;
	timeSinceStart += deltaTime;

	// Update if a second has passed
	oneSecondPassed = beforeTime != (unsigned int) timeSinceStart;
}

void Time::startTimer()
{
	userLastTime = std::chrono::system_clock::now();
}

float Time::endTimer()
{
	userElapsedSeconds = std::chrono::system_clock::now() - userLastTime;

	return userElapsedSeconds.count();
}

void Time::startGodTimer()
{
	godUserLastTime = std::chrono::system_clock::now();
}

void Time::endGodTimer()
{
	godUserElapsedSeconds = std::chrono::system_clock::now() - godUserLastTime;

	godUserTime = godUserElapsedSeconds.count();
}

float Time::getGodTime()
{
	return godUserTime;
}

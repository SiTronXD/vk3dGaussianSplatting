#pragma once

#include <chrono>

class Time
{
private:
	friend class Engine;

	static float deltaTime;
	static float timeSinceStart;
	static bool oneSecondPassed;

	static std::chrono::system_clock::time_point lastTime;
	static std::chrono::system_clock::time_point currentTime;
	static std::chrono::duration<float> elapsedSeconds;

	static std::chrono::system_clock::time_point userLastTime;
	static std::chrono::duration<float> userElapsedSeconds;
	static std::chrono::system_clock::time_point godUserLastTime;
	static std::chrono::duration<float> godUserElapsedSeconds;
	static float godUserTime;

	static void init();
	static void updateDeltaTime();

public:
	static const inline float& getDT() { return deltaTime; };
	static const inline float& getTimeSinceStart() { return timeSinceStart; }
	static const inline bool& hasOneSecondPassed() { return oneSecondPassed; }

	static void startTimer();
	static float endTimer();

	static void startGodTimer();
	static void endGodTimer();
	static float getGodTime();
};
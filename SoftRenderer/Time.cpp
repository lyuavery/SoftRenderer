#include "Time.h"
#include "Header.h"
float SR::Time::deltaTime = 0.033f;
UInt32 SR::Time::frameCnt = 0;
double SR::Time::lastUpdateTime = 0;
double SR::Time::clockPeriod = -1; //  ±÷”÷‹∆⁄
double SR::Time::startTime = -1;
float SR::Time::fpsStatLastTime = 0;
float SR::Time::fpsStatElapseTime = 0;
UInt32 SR::Time::fpsStatElapseFrameCnt = 0;
float SR::Time::fps = 30;

void SR::Time::Init()
{
	if (clockPeriod < 0) {
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		clockPeriod = 1 / (double)frequency.QuadPart;
	}
	if (startTime < 0)
	{
		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		startTime = counter.QuadPart * clockPeriod; // sec
	}
}

void SR::Time::Update()
{
	float now = TimeSinceStartup();
	deltaTime = now - lastUpdateTime;
	lastUpdateTime = now;
	++frameCnt;
	++fpsStatElapseFrameCnt;
	fpsStatElapseTime += deltaTime;
	if ((now - fpsStatLastTime) > 1)
	{
		fps = fpsStatElapseFrameCnt / fpsStatElapseTime;
		fpsStatLastTime = now;
		fpsStatElapseTime = 0;
		fpsStatElapseFrameCnt = 0;
	}
}

float SR::Time::TimeSinceStartup()
{
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return float(counter.QuadPart * clockPeriod - startTime);
}
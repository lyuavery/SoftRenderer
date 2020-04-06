#include <Windows.h>
#include "Time.h"

float SR::Time::deltaTime = 0.033f;
float SR::Time::lastUpdateTime = -1;
double SR::Time::clockPeriod = -1; //  ±÷”÷‹∆⁄
UInt32 SR::Time::frameCnt = 0;

void SR::Time::Init()
{
	if (clockPeriod < 0) {
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		clockPeriod = 1 / (double)frequency.QuadPart;
	}
	if (lastUpdateTime < 0)
	{
		lastUpdateTime = TimeSinceSinceStartup();
	}
}

void SR::Time::Update()
{
	float now = TimeSinceSinceStartup();
	deltaTime = now - lastUpdateTime;
	lastUpdateTime = now;
	++frameCnt;
}
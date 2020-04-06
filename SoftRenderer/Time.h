#pragma once
#include "Sealed.h"
#include "Header.h"
namespace SR
{
	class Time : virtual private ISealed<Time>
	{
		static float deltaTime;
		static float lastUpdateTime;
		static double clockPeriod;
		static UInt32 frameCnt;
	public:
		static inline UInt32 FrameCnt() { return frameCnt; }
		static inline float DeltaTime() { return deltaTime; }
		static inline float TimeSinceSinceStartup()
		{
			LARGE_INTEGER counter;
			QueryPerformanceCounter(&counter);
			return float(counter.QuadPart * clockPeriod);
		}
		static void Init();
		static void Update();
	};
}

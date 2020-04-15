#pragma once
#include "Sealed.h"
#include "TypeDef.h"
namespace SR
{
	class Time : virtual private ISealed<Time>
	{
		static float fpsStatLastTime;
		static float fpsStatElapseTime;
		static UInt32 fpsStatElapseFrameCnt;
		static float fps;
		static float deltaTime;
		static double startTime;
		static double lastUpdateTime;
		static double clockPeriod;
		static UInt32 frameCnt;
	public:
		static inline UInt32 FrameCnt() { return frameCnt; }
		static inline float DeltaTime() { return deltaTime; }
		static inline float FPS() { return fps; }
		static inline float TimeSinceStartup();
		static void Init();
		static void Update();
	};
}

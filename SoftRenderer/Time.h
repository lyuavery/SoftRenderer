#pragma once
#include <Windows.h>

namespace SR
{
	class Time
	{
	public:
		static float Now()
		{
			static double period = -1;
			LARGE_INTEGER counter;
			if (period < 0) {
				LARGE_INTEGER frequency;
				QueryPerformanceFrequency(&frequency);
				period = 1 / (double)frequency.QuadPart;
			}
			QueryPerformanceCounter(&counter);
			return float(counter.QuadPart * period);
		}
	};
}

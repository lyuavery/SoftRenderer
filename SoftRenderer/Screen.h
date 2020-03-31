#pragma once
#include <Windows.h>

namespace SR
{
	class Screen
	{
	public:
		static const int GetWidth() { return GetSystemMetrics(SM_CXSCREEN); }
		static const int GetHeight() { return GetSystemMetrics(SM_CYSCREEN); }
	};
}



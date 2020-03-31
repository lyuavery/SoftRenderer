#pragma once
#include <limits>

namespace sbm
{
	class Math
	{
	public:
		static const float PI;
		static constexpr int IntMax = (std::numeric_limits<int>::max)();
		static constexpr int IntMin = (std::numeric_limits<int>::min)();
		static constexpr float FloatMax = (std::numeric_limits<float>::max)();
		static constexpr float FloatMin = -(std::numeric_limits<float>::max)();
		static constexpr float FloatZeroLeftLimit = (std::numeric_limits<float>::min)();
		static constexpr float FloatZeroRightLimit = -(std::numeric_limits<float>::min)();
	};
}


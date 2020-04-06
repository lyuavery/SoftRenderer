#pragma once
#include <limits>

namespace sbm
{
	class Math
	{
	public:
		static constexpr float PI = 3.141592653589793f;
		static constexpr float HalfPI = 1.570796326794896f;
		static constexpr float PIx2 = 6.283185307179586f;
		static constexpr float Degree2Rad = 0.017453292519f;
		static constexpr float Rad2Degree = 57.29577951308f;
		static constexpr int IntMax = (std::numeric_limits<int>::max)();
		static constexpr int IntMin = (std::numeric_limits<int>::min)();
		static constexpr float FloatMax = (std::numeric_limits<float>::max)();
		static constexpr float FloatMin = -(std::numeric_limits<float>::max)();
		static constexpr float FloatZeroLeftLimit = (std::numeric_limits<float>::min)();
		static constexpr float FloatZeroRightLimit = -(std::numeric_limits<float>::min)();
	};
}


#include "Utility.h"
namespace sbm
{
	float sin(float rad)
	{
		return std::sin(rad);
	}

	float cos(float rad)
	{
		return std::cos(rad);
	}

	float tan(float rad)
	{
		return std::tan(rad);
	}

	float cot(float rad)
	{
		return 1.f / std::tan(rad);
	}

	float pow(float left, float right)
	{
		return std::powf(left, right);
	}

	/// in rad, [-pi/2, pi/2]
	float asin(float x)
	{
		return std::asinf(x);
	}

	/// in rad, [0, pi]
	float acos(float x)
	{
		return std::acosf(x);
	}

}
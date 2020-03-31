#pragma once
#include <initializer_list>
#include <cmath>
// TODO：<T>vec<int>泛型创建向量结构体
// TODO：数学库
namespace sbm
{
	template<typename T>
	inline T abs(T num)
	{
		return num >= 0 ? num : -num;
	}

	template<typename T>
	inline T max(T a, T b)
	{
		return a > b ? a : b;
	}

	template<typename T>
	T max(const std::initializer_list<T>& list)
	{
		auto b = list.begin(), e = list.end();
		T ret = *b;
		while (++b != e) { ret = *b > ret ? *b : ret; }
		return ret;
	}

	template<typename T>
	inline T min(T a, T b)
	{
		return a < b ? a : b;
	}

	template<typename T>
	T min(const std::initializer_list<T>& list)
	{
		auto b = list.begin(), e = list.end();
		T ret = *b;
		while (++b != e) { ret = *b < ret ? *b : ret; }
		return ret;
	}

	template<typename T>
	inline T clamp(T x, T a, T b)
	{
		return x < a ? a : (x > b ? b: x);
	}

	template<typename T>
	inline T clamp01(T x)
	{
		auto a = T(0), b = T(1);
		return x < a ? a : (x > b ? b : x);
	}

	float sin(float rad);
	float cos(float rad);
	float pow(float left, float right);

}
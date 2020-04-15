#pragma once
#include <initializer_list>
#include <cmath>
#include "Vector.h"
#include "Math.h"
// TODO��<T>vec<int>���ʹ��������ṹ��
// TODO����ѧ��
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

	template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
	inline T ceil(T x)
	{
		int _x = int(x);
		if (x < 0) return _x;
		if (_x < x) return int(x + 1);
		return _x;
	}

	template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
	inline T floor(T x)
	{
		int _x = int(x);
		if (x > 0) return _x;
		if (_x > x) return int(x - 1);
		return _x;
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

	template<typename T>
	sbm::Vec<T,3> reflect(const sbm::Vec<T, 3>& i, const sbm::Vec<T, 3>& n)
	{
		//Vec3 iNorm = i.Normalized();
		//Vec3 nNorm = n.Normalized();
		return i - 2 * Dot(i, n) * n;
	}
	
	template<typename T>
	inline T radians(T x)
	{
		return x*Math::Degree2Rad;
	}
	
	template<typename T>
	inline T degrees(T x)
	{
		return x * Math::Rad2Degree;
	}

	template<typename T>
	sbm::Vec<T, 3> barycentric(const sbm::Vec<T, 2>& v0, const sbm::Vec<T, 2>& v1, const sbm::Vec<T, 2>& v2, const sbm::Vec<T, 2>& p)
	{
		// (u, v, 1) �루AB.x, AC.x, PA.x) �ͣ�AB.y, AC.y, PA.y) ������
		sbm::Vec<T, 3> lambda = Cross(sbm::Vec<T, 3>(v1.x - v0.x, v2.x - v0.x, v0.x - p.x), sbm::Vec<T, 3>(v1.y - v0.y, v2.y - v0.y, v0.y - p.y));
		// ����ʹ���������꣬lambda.zΪ�������������������0ʱ֤���˻�
		if (sbm::abs(lambda.z) < 1e-4) return sbm::Vec<T, 3>(-1, 1, 1);
		float rcpArea = 1.0f / lambda.z;
		// ����lambda.z���淶���������Ϊ1
		return sbm::Vec<T, 3>(1.f - (lambda.x + lambda.y) * rcpArea, lambda.x * rcpArea, lambda.y * rcpArea);
	}

	float sin(float rad);
	float cos(float rad);
	float tan(float rad);
	float cot(float rad);
	float pow(float left, float right);
	float asin(float x);
	float acos(float x);
	
}
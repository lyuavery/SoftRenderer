#pragma once
#include "Vector.h"
#include <assert.h>
namespace sbm
{
	template<typename T>
	struct Quaternion
	{
		T x, y, z, w;
		T& operator[](int i) { assert(i >= 0 && i < 4);  return *(&x + i); }
		const T& operator[](int i) const { assert(i >= 0 && i < 4);  return *(&x + i);  }
		Quaternion() = default;h
		Quaternion(float angle, const Vec3<T, 3>& axis) {}
		Vec<T, 3> EulerAngles() const {}
		Quaternion operator *(const Quaternion& lhs, const Quaternion& rhs);
		Vec<T, 3> operator *(const Quaternion& rotation, const Vec3<T, 3>& point);
		bool operator ==(const Quaternion& rhs);
		bool operator !=(const Quaternion& rhs);
		static const Quaternion Identity;
		static Quaternion FromToRotation(const Vec<T,3> fromDirection, const Vec<T, 3>& toDirection);
		static Quaternion Euler(const Vec<T, 3>& euler);
		static Quaternion Euler(float x, float y, float z);
		static Quaternion Inverse(const Quaternion& rotation);
		static Quaternion LookRotation(const Vec<T, 3>& forward);
		static Quaternion LookRotation(const Vec<T, 3>& forward, const Vec<T, 3>& upwards);
	};
}

#pragma once
#include <cassert>
#include <cmath>
#include <iostream>
#include <type_traits>
namespace sbm
{
// Small Budget Math Lib

/* =================  Vector  ================= */
#define SBM_VEC_BODY(T, N) \
	explicit Vec() { \
		for (size_t i = 0; i < N; ++i) (&x)[i] = T(); \
	} \
	explicit Vec(T val) { \
		for (size_t i = 0; i < N; ++i) (&x)[i] = val; \
	} \
	T& operator[](size_t i) { assert(i < N && i >= 0); return (&x)[i]; } \
	const T& operator[](size_t i) const { assert(i < N && i >= 0); return (&x)[i]; } \
	T Magnitude() const { \
		T sum = T(0); \
		for (size_t i = 0; i < N; ++i) { \
			sum += (&x)[i] * (&x)[i]; \
		} \
		return std::sqrt(sum); \
	} \
	void Normalize() { \
		T mag = Magnitude(); \
		for (size_t i = 0; i < N; ++i) { \
				(&x)[i] /= mag; \
		} \
	} \
	Vec<T, N> Normalized() const { \
		Vec<T, N> v; \
		T mag = Magnitude(); \
		for (size_t i = 0; i < N; ++i) { \
				(&v.x)[i] = (&x)[i] / mag; \
		} \
		return v; \
	} \
	size_t Size() const { return N; } \
	template<size_t M> Vec<T, M> Expanded(const T& fill) { \
		static_assert(M > N, "Expanded vector should have more component than the current one."); \
		Vec<T, M> v; \
		for (size_t i = 0; i < N; ++i) { \
			(&v.x)[i] = (&x)[i]; \
		} \
		for (size_t i = N; i < M; ++i) { \
			(&v.x)[i] = fill; \
		} \
		return v; \
	} \
	template<size_t M> Vec<T, M> Truncated() { \
		static_assert(M < N, "Expanded vector should have less component than the current one."); \
		Vec<T, M> v; \
		for (size_t i = 0; i < M; ++i) { \
				(&v.x)[i] = (&x)[i]; \
		} \
		return v; \
	}

	template<typename T, size_t N>
	struct Vec {
		SBM_VEC_BODY(T, N)
	private:
		T x[N];
	};

	template<typename T>
	struct Vec<T, 2> {
		SBM_VEC_BODY(T, 2)

		explicit Vec(T a, T b) {
			x = a;
			y = b;
		}
		union {
			struct { T x, y; };
			struct { T r, g; };
			struct { T s, t; };
		};
	};

	template<typename T>
	struct Vec<T, 3> {
		SBM_VEC_BODY(T, 3)

		explicit Vec(T a, T b, T c) {
			x = a;
			y = b;
			z = c;
		}
		union {
			struct { T x, y, z; };
			struct { T r, g, b; };
			struct { T s, t, p; };
		};

		inline explicit operator Vec<T, 2>() { return Vec<T, 2>(x, y); }
	};

	template<typename T>
	struct Vec<T, 4> {
		SBM_VEC_BODY(T, 4)

		explicit Vec(T a, T b, T c, T d) {
			x = a;
			y = b;
			z = c;
			w = d;
		}
		union {
			struct { T x, y, z, w; };
			struct { T r, g, b, a; };
			struct { T s, t, p, q; };
		};

		inline explicit operator Vec<T, 2>() { return Vec<T, 2>(x, y); }
		inline explicit operator Vec<T, 3>() { return Vec<T, 3>(x, y, z); }
	};

	template<typename T, size_t N>
	std::ostream& operator<<(std::ostream& out, Vec<T, N> vector)
	{
		out << "(";
		for(size_t i = 0; i < N; ++i){
			out << vector[i] << (i == N - 1 ? "" : ",");
		}
		out << ")";
		return out;
	}

	template<typename T, size_t N>
	Vec<T, N> operator+(const Vec<T, N>& lhs, const Vec<T, N>& rhs)
	{
		Vec<T, N> ret;
		for (size_t i = 0; i < N; ++i) {
			ret[i] = lhs[i] + rhs[i];
		}
		return ret;
	}

	template<typename T, size_t N>
	Vec<T, N> operator+(const Vec<T, N>& lhs, T var)
	{
		Vec<T, N> ret;
		for (size_t i = 0; i < N; ++i) {
			ret[i] = lhs[i] + var;
		}
		return ret;
	}

	template<typename T, size_t N>
	Vec<T, N> operator+(T var, const Vec<T, N>& rhs)
	{
		Vec<T, N> ret;
		for (size_t i = 0; i < N; ++i) {
			ret[i] = rhs[i] + var;
		}
		return ret;
	}

	template<typename T, size_t N>
	Vec<T, N> operator-(const Vec<T, N>& lhs, const Vec<T, N>& rhs)
	{
		Vec<T, N> ret;
		for (size_t i = 0; i < N; ++i) {
			ret[i] = lhs[i] - rhs[i];
		}
		return ret;
	}

	template<typename T, size_t N, typename U, typename = typename std::enable_if<std::is_arithmetic<U>::value>::type>
	Vec<T, N> operator*(const Vec<T, N>& lhs, const U& scalar)
	{
		Vec<T, N> ret;
		for (size_t i = 0; i < N; ++i) {
			ret[i] = lhs[i] * scalar;
		}
		return ret;
	}

	template<typename T, size_t N, typename U, typename = typename std::enable_if<std::is_arithmetic<U>::value>::type>
	Vec<T, N> operator*(const U& scalar, const Vec<T, N>& rhs)
	{
		Vec<T, N> ret;
		for (size_t i = 0; i < N; ++i) {
			ret[i] = rhs[i] * scalar;
		}
		return ret;
	}

	template<typename T, size_t N>
	Vec<T, N> operator*(const Vec<T, N>& lhs, const Vec<T, N>& rhs)
	{
		Vec<T, N> ret;
		for (size_t i = 0; i < N; ++i) {
			ret[i] = lhs[i] * rhs[i];
		}
		return ret;
	}

	template<typename T, size_t N, typename U>
	Vec<T, N> operator/(const Vec<T, N>& lhs, const U& scalar)
	{
		Vec<T, N> ret;
		for (size_t i = 0; i < N; ++i) {
			ret[i] = lhs[i] / scalar;
		}
		return ret;
	}

	template<typename T, size_t N>
	Vec<T, N> operator-(const Vec<T, N>& lhs)
	{
		Vec<T, N> ret;
		for (size_t i = 0; i < N; ++i) {
			ret[i] = -lhs[i];
		}
		return ret;
	}

	/*template<typename T, size_t N>
	Vec<T, N> operator=(const Vec<T, N>& lhs);
	
	template<typename T, size_t N>
	Vec<T, N> operator==(const Vec<T, N>& lhs);

	template<typename T, size_t N>
	Vec<T, N> operator!=(const Vec<T, N>& lhs);

	template<typename T, size_t N>
	Vec<T, N> operator*=(const Vec<T, N>& lhs);

	template<typename T, size_t N>
	Vec<T, N> operator+=(const Vec<T, N>& lhs);

	template<typename T, size_t N>
	Vec<T, N> operator-=(const Vec<T, N>& lhs);

	*/

	template<typename T, size_t N>
	Vec<T, N>& operator/=(Vec<T, N>& lhs, const T& v)
	{
		for (size_t i = 0; i < N; ++i) {
			lhs[i] /= v;
		}
		return lhs;
	}

	template<typename T, size_t N>
	Vec<T, N>& operator*=(Vec<T, N>& lhs, const T& v)
	{
		for (size_t i = 0; i < N; ++i) {
			lhs[i] *= v;
		}
		return lhs;
	}

	template<typename T, size_t N>
	T Dot(const Vec<T, N>& lhs, const Vec<T, N>& rhs)
	{
		T ret = T(0);
		for (size_t i = 0; i < N; ++i) {
				ret += lhs[i] * rhs[i];
		}
		return ret;
	}

	template<typename T>
	Vec<T, 3> Cross(const Vec<T, 3>& lhs, const Vec<T, 3>& rhs)
	{
		return Vec<T, 3>(lhs.y * rhs.z - lhs.z * rhs.y, -lhs.x * rhs.z + lhs.z * rhs.x, lhs.x * rhs.y - lhs.y * rhs.x);
	}

	template<typename T>
	Vec<T, 4> Cross(const Vec<T, 4>& lhs, const Vec<T, 4>& rhs)
	{
		return Vec<T, 4>(lhs.y * rhs.z - lhs.z * rhs.y, -lhs.x * rhs.z + lhs.z * rhs.x, lhs.x * rhs.y - lhs.y * rhs.x, 0);
	}
	/*template<typename T>
	inline Vec<T, 4>::operator Vec<T, 2>()
	{
		return Vec<T, 2>(x, y);
	}*/
}

typedef sbm::Vec<float, 2> Vec2;
typedef sbm::Vec<float, 3> Vec3;
typedef sbm::Vec<float, 4> Vec4;
typedef sbm::Vec<double, 2> Vec2d;
typedef sbm::Vec<double, 3> Vec3d;
typedef sbm::Vec<double, 4> Vec4d;
typedef sbm::Vec<int, 2> Vec2i;
typedef sbm::Vec<int, 3> Vec3i;
typedef sbm::Vec<unsigned char, 4> Vec4uc;



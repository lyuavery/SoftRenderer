#pragma once

#include <cassert>
#include <cmath>
#include "Vector.h"
#include "Utility.h"
#include "Matrix.h"
#include <type_traits>
#include <algorithm>
#include <vector>
#include <ostream>

// 可以考虑写一个自定义类型T, 各元素视为符号，输出符号表示的运算结果
namespace sbm
{
#define MAT(m, r, c) (m).value[(r) + (c) * 4]
#define _M(r, c) value[(r) + (c) * 4]
	template<typename T>
	struct Matrix<4, 4, T>
	{
	protected:
		T value[16];// column major
	public:
		Matrix() = default;

		using ElemType = T;
		inline static constexpr size_t size() { return 16; }

		explicit Matrix(T& v)
		{
			for (int i = 0; i < 16; ++i) value[i] = 0;
			value[0] = value[5] = value[10] = value[15] = v;
		}

		explicit Matrix(T data[16])
		{
			for (int i = 0; i < 16; ++i) value[i] = data[i];
		}

		explicit Matrix(const std::vector<T>& data)
		{
			for (int i = 0; i < 16; ++i) value[i] = data[i];
		}

		inline void SetRow(int i, T x, T y, T z, T w);
		inline void SetRow(int i, const sbm::Vec<T, 4>& v4);
		inline sbm::Vec<T, 4> GetRow(int i);

		inline void SetColumn(int i, T x, T y, T z, T w);
		inline void SetColumn(int i, const sbm::Vec<T, 4>& v4);
		inline sbm::Vec<T, 4> GetColumn(int i);
		inline T& M(int r, int c);

		inline void SetIdentity();
		Matrix GetInversed();
		inline Matrix GetTransposed();

		inline Matrix operator*(const T& scale);
		inline Matrix operator*(const Matrix& m4);
		template<typename T> friend Matrix operator*(const T& scale, const Matrix<4,4,T>& m4);
		Matrix& operator*=(const Matrix& m4);
		inline Matrix& operator*=(const T& scale);
		sbm::Vec<T, 4> operator*(const sbm::Vec<T, 4>& v4);
		template<size_t NX> Matrix<4, NX, T> operator*(const Matrix<4, NX, T>& m4);
		template<typename T> friend std::ostream& operator<<(std::ostream& out, const Matrix<4, 4, T>& m4);
		template<typename T> friend sbm::Vec<T, 4> operator*(const sbm::Vec<T, 4>& v4, const Matrix<4, 4, T>& m4);

		inline const size_t RowSize() const { return 4; }
		inline const size_t ColSize() const { return 4; }

		static const Matrix<4, 4, T> Identity;
		static bool InverseMatrix(const Matrix<4, 4, T>& in, Matrix<4, 4, T>& out);
		static Matrix<4, 4, T> TRS(const Vec<T, 3>& translation, const Vec<T, 3>& rotation, const Vec<T, 3>& scale);
		static Matrix<4, 4, T> Translate(const Vec<T, 3>& translation);
		static Matrix<4, 4, T> Rotate(const Vec<T, 3>& rotation);
		static Matrix<4, 4, T> Scale(const Vec<T, 3>& scale);
		static Matrix<4, 4, T> LookAt(const Vec<T, 3>& eye, const Vec<T, 3>& position, const Vec<T, 3>& up);

		/*static Matrix<4, 4, T> Frustum(T left, T right, T bottom, T top, T nearClip, T farClip);
		static Matrix<4, 4, T> Ortho(const Vec3& eye, T nearClip, T farClip);*/
		//inline operator Matrix<3, 3, T>();
	};

	template<typename T>
	inline void Matrix<4, 4, T>::SetIdentity()
	{
		_M(0, 0) = T(1); _M(0, 1) = T(0); _M(0, 2) = T(0); _M(0, 3) = T(0);
		_M(1, 0) = T(0); _M(1, 1) = T(1); _M(1, 2) = T(0); _M(1, 3) = T(0);
		_M(2, 0) = T(0); _M(2, 1) = T(0); _M(2, 2) = T(1); _M(2, 3) = T(0);
		_M(3, 0) = T(0); _M(3, 1) = T(0); _M(3, 2) = T(0); _M(3, 3) = T(1);
	}

	template<typename T>
	inline void Matrix<4, 4, T>::SetRow(int i, T x, T y, T z, T w)
	{
		assert(i >= 0 && i < 4);
		_M(i, 0) = x; _M(i, 1) = y;
		_M(i, 2) = z; _M(i, 3) = w;
	}

	template<typename T>
	inline void Matrix<4, 4, T>::SetRow(int i, const sbm::Vec<T, 4>& v4)
	{
		assert(i >= 0 && i < 4);
		_M(i, 0) = v4[0]; _M(i, 1) = v4[1];
		_M(i, 2) = v4[2]; _M(i, 3) = v4[3];
	}

	template<typename T>
	inline sbm::Vec<T, 4> Matrix<4, 4, T>::GetRow(int i)
	{
		assert(i >= 0 && i < 4);
		return sbm::Vec<T, 4>(_M(i, 0), _M(i, 1), _M(i, 2), _M(i, 3));
	}

	template<typename T>
	inline void Matrix<4, 4, T>::SetColumn(int i, T x, T y, T z, T w)
	{
		assert(i >= 0 && i < 4);
		_M(0, i) = x; _M(1, i) = y;
		_M(2, i) = z; _M(3, i) = w;
	}

	template<typename T>
	inline void Matrix<4, 4, T>::SetColumn(int i, const sbm::Vec<T, 4>& v4)
	{
		assert(i >= 0 && i < 4);
		_M(0, i) = v4[0]; _M(1, i) = v4[1];
		_M(2, i) = v4[2]; _M(3, i) = v4[3];
	}

	template<typename T>
	inline sbm::Vec<T, 4> Matrix<4, 4, T>::GetColumn(int i)
	{
		assert(i >= 0 && i < 4);
		return sbm::Vec<T, 4>(_M(0, i), _M(1, i), _M(2, i), _M(3, i));
	}

	template<typename T>
	inline T& Matrix<4, 4, T>::M(int r, int c)
	{
		assert(r >= 0 && r < 4 && c >= 0 && c < 4);
		return _M(r, c);
	}

	template<typename T>
	sbm::Vec<T, 4> Matrix<4, 4, T>::operator*(const sbm::Vec<T, 4>& v4)
	{
		return sbm::Vec<T, 4>(
			_M(0, 0) * v4[0] + _M(0, 1) * v4[1] + _M(0, 2) * v4[2] + _M(0, 3) * v4[3],
			_M(1, 0) * v4[0] + _M(1, 1) * v4[1] + _M(1, 2) * v4[2] + _M(1, 3) * v4[3],
			_M(2, 0) * v4[0] + _M(2, 1) * v4[1] + _M(2, 2) * v4[2] + _M(2, 3) * v4[3],
			_M(3, 0) * v4[0] + _M(3, 1) * v4[1] + _M(3, 2) * v4[2] + _M(3, 3) * v4[3]
			);
	}

	template<typename T>
	Matrix<4, 4, T> Matrix<4, 4, T>::operator*(const T& scale)
	{
		Matrix<4, 4, T> temp(*this);
		return temp *= scale;
	}

	template<typename T>
	inline Matrix<4, 4, T> operator*(const T& scale, const Matrix<4, 4, T>& m4)
	{
		Matrix<4, 4, T> temp(m4);
		return temp *= scale;
	}

	template<typename T>
	Matrix<4, 4, T> Matrix<4, 4, T>::operator*(const Matrix& m4)
	{
		Matrix<4, 4, T> temp(*this);
		return temp *= m4;
	}

	template<typename T>
	Matrix<4, 4, T>& Matrix<4, 4, T>::operator*=(const Matrix<4, 4, T>& m4)
	{
		//assert(&m4 != this);
		Matrix<4, 4, T> temp; // 必须创建临时矩阵，避免元素覆盖
		for (int i = 0; i < 4; ++i)
		{
			MAT(temp, 0, i) = _M(0, 0) * MAT(m4, 0, i) + _M(0, 1) * MAT(m4, 1, i) + _M(0, 2) * MAT(m4, 2, i) + _M(0, 3) * MAT(m4, 3, i);
			MAT(temp, 1, i) = _M(1, 0) * MAT(m4, 0, i) + _M(1, 1) * MAT(m4, 1, i) + _M(1, 2) * MAT(m4, 2, i) + _M(1, 3) * MAT(m4, 3, i);
			MAT(temp, 2, i) = _M(2, 0) * MAT(m4, 0, i) + _M(2, 1) * MAT(m4, 1, i) + _M(2, 2) * MAT(m4, 2, i) + _M(2, 3) * MAT(m4, 3, i);
			MAT(temp, 3, i) = _M(3, 0) * MAT(m4, 0, i) + _M(3, 1) * MAT(m4, 1, i) + _M(3, 2) * MAT(m4, 2, i) + _M(3, 3) * MAT(m4, 3, i);
		}
		*this = temp;
		return *this;
	}

	template<typename T>
	Matrix<4, 4, T>& Matrix<4, 4, T>::operator*=(const T& scale)
	{
		_M(0, 0) *= scale; _M(0, 1) *= scale; _M(0, 2) *= scale; _M(0, 3) *= scale;
		_M(1, 0) *= scale; _M(1, 1) *= scale; _M(1, 2) *= scale; _M(1, 3) *= scale;
		_M(2, 0) *= scale; _M(2, 1) *= scale; _M(2, 2) *= scale; _M(2, 3) *= scale;
		_M(3, 0) *= scale; _M(3, 1) *= scale; _M(3, 2) *= scale; _M(3, 3) *= scale;
		return *this;
	}

#define SWAP_ROWS(a, b) { float *_tmp = a; (a)=(b); (b)=_tmp; }
#define RETURN_ZERO(m) \
{ \
	for (int i=0;i<16;++i) \
		(m).value[i] = 0.0f; \
	return false; \
}

	template<typename T>
	bool Matrix<4, 4, T>::InverseMatrix(const Matrix<4, 4, T>& in, Matrix<4, 4, T>& out)
	{
		float wtmp[4][8];
		float m0, m1, m2, m3, s;
		float *r0, *r1, *r2, *r3;
		r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];

		r0[0] = MAT(in, 0, 0), r0[1] = MAT(in, 0, 1),
		r0[2] = MAT(in, 0, 2), r0[3] = MAT(in, 0, 3),
		r0[4] = 1.0, r0[5] = r0[6] = r0[7] = 0.0,

		r1[0] = MAT(in, 1, 0), r1[1] = MAT(in, 1, 1),
		r1[2] = MAT(in, 1, 2), r1[3] = MAT(in, 1, 3),
		r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0,

		r2[0] = MAT(in, 2, 0), r2[1] = MAT(in, 2, 1),
		r2[2] = MAT(in, 2, 2), r2[3] = MAT(in, 2, 3),
		r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0,

		r3[0] = MAT(in, 3, 0), r3[1] = MAT(in, 3, 1),
		r3[2] = MAT(in, 3, 2), r3[3] = MAT(in, 3, 3),
		r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;

		/* choose pivot - or die */
		if (sbm::abs(r3[0]) > sbm::abs(r2[0])) SWAP_ROWS(r3, r2);
		if (sbm::abs(r2[0]) > sbm::abs(r1[0])) SWAP_ROWS(r2, r1);
		if (sbm::abs(r1[0]) > sbm::abs(r0[0])) SWAP_ROWS(r1, r0);
		if (0.0F == r0[0]) RETURN_ZERO(out);

		/* eliminate first variable */
		m1 = r1[0] / r0[0]; m2 = r2[0] / r0[0]; m3 = r3[0] / r0[0];		// normalize col 0
		s = r0[1]; r1[1] -= m1 * s; r2[1] -= m2 * s; r3[1] -= m3 * s;
		s = r0[2]; r1[2] -= m1 * s; r2[2] -= m2 * s; r3[2] -= m3 * s;
		s = r0[3]; r1[3] -= m1 * s; r2[3] -= m2 * s; r3[3] -= m3 * s;
		s = r0[4];
		if (s != 0.0F) { r1[4] -= m1 * s; r2[4] -= m2 * s; r3[4] -= m3 * s; }
		s = r0[5];
		if (s != 0.0F) { r1[5] -= m1 * s; r2[5] -= m2 * s; r3[5] -= m3 * s; }
		s = r0[6];
		if (s != 0.0F) { r1[6] -= m1 * s; r2[6] -= m2 * s; r3[6] -= m3 * s; }
		s = r0[7];
		if (s != 0.0F) { r1[7] -= m1 * s; r2[7] -= m2 * s; r3[7] -= m3 * s; }

		/* choose pivot - or die */
		if (sbm::abs(r3[1]) > sbm::abs(r2[1])) SWAP_ROWS(r3, r2);
		if (sbm::abs(r2[1]) > sbm::abs(r1[1])) SWAP_ROWS(r2, r1);
		if (0.0F == r1[1]) RETURN_ZERO(out);

		/* eliminate second variable */
		m2 = r2[1] / r1[1]; m3 = r3[1] / r1[1];
		r2[2] -= m2 * r1[2]; r3[2] -= m3 * r1[2];
		r2[3] -= m2 * r1[3]; r3[3] -= m3 * r1[3];
		s = r1[4]; if (0.0F != s) { r2[4] -= m2 * s; r3[4] -= m3 * s; }
		s = r1[5]; if (0.0F != s) { r2[5] -= m2 * s; r3[5] -= m3 * s; }
		s = r1[6]; if (0.0F != s) { r2[6] -= m2 * s; r3[6] -= m3 * s; }
		s = r1[7]; if (0.0F != s) { r2[7] -= m2 * s; r3[7] -= m3 * s; }

		/* choose pivot - or die */
		if (sbm::abs(r3[2]) > sbm::abs(r2[2])) SWAP_ROWS(r3, r2);
		if (0.0F == r2[2]) RETURN_ZERO(out);

		/* eliminate third variable */
		m3 = r3[2] / r2[2];
		r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4],
			r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6],
			r3[7] -= m3 * r2[7];

		/* last check */
		if (0.0F == r3[3]) RETURN_ZERO(out);

		s = 1.0F / r3[3];             /* now back substitute row 3 */
		r3[4] *= s; r3[5] *= s; r3[6] *= s; r3[7] *= s;

		m2 = r2[3];                 /* now back substitute row 2 */
		s = 1.0F / r2[2];
		r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),
		r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
		m1 = r1[3];
		r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1,
		r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
		m0 = r0[3];
		r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0,
		r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;

		m1 = r1[2];                 /* now back substitute row 1 */
		s = 1.0F / r1[1];
		r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1),
		r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);
		m0 = r0[2];
		r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0,
		r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;

		m0 = r0[1];                 /* now back substitute row 0 */
		s = 1.0F / r0[0];
		r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0),
		r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);

		MAT(out, 0, 0) = r0[4]; MAT(out, 0, 1) = r0[5], MAT(out, 0, 2) = r0[6]; MAT(out, 0, 3) = r0[7],
		MAT(out, 1, 0) = r1[4]; MAT(out, 1, 1) = r1[5], MAT(out, 1, 2) = r1[6]; MAT(out, 1, 3) = r1[7],
		MAT(out, 2, 0) = r2[4]; MAT(out, 2, 1) = r2[5], MAT(out, 2, 2) = r2[6]; MAT(out, 2, 3) = r2[7],
		MAT(out, 3, 0) = r3[4]; MAT(out, 3, 1) = r3[5], MAT(out, 3, 2) = r3[6]; MAT(out, 3, 3) = r3[7];

		return true;
	}

	template<typename T>
	Matrix<4, 4, T> Matrix<4, 4, T>::GetInversed()
	{
		Matrix<4, 4, T> temp;
		bool suc = InverseMatrix(*this, temp);
		//if (!suc) LogWarning("Matrix irreversible.");
		return temp;
	}
#undef SWAP_ROWS
#undef RETURN_ZERO

	template<typename T>
	Matrix<4, 4, T> Matrix<4, 4, T>::GetTransposed()
	{
		Matrix<4, 4, T> temp = *this;
		for (int i = 0; i < 3; ++i)
		{
			for (int j = i + 1; j < 4; ++j)
			{
				std::swap(MAT(temp, i, j), MAT(temp, j, i));
			}
		}
		return temp;
	}

	template<typename T>
	Matrix<4, 4, T> Matrix<4, 4, T>::TRS(const Vec<T, 3>& translation, const Vec<T, 3>& rotation, const Vec<T, 3>& scale)
	{
		float radx = sbm::radians(rotation.x), rady = sbm::radians(rotation.y), radz = sbm::radians(rotation.z);
		float cosx = sbm::cos(radx), sinx = sbm::sin(radx), cosy = sbm::cos(rady), siny = sbm::sin(rady), cosz = sbm::cos(radz), sinz = sbm::sin(radz);
		/*
		auto rotx = Matrix<4, 4, T>::Identity, roty = Matrix<4, 4, T>::Identity, rotz = Matrix<4, 4, T>::Identity;
		MAT(rotx, 1, 1) = cosx; MAT(rotx, 1, 2) = -sinx; MAT(rotx, 2, 1) = sinx; MAT(rotx, 2, 2) = cosx;
		MAT(roty, 0, 0) = cosy; MAT(roty, 0, 2) = siny; MAT(roty, 2, 0) = -siny; MAT(roty, 2, 2) = cosy;
		MAT(rotz, 0, 0) = cosz; MAT(rotz, 0, 1) = -sinz; MAT(rotz, 1, 0) = sinz; MAT(rotz, 1, 1) = cosz;
		auto trs = roty * rotx * rotz;
		*/

		// unroll版
		// Y * X * Z * vec，顺规ZXY
		auto trs = Matrix<4, 4, T>::Identity; 
		MAT(trs, 0, 0) = cosz * cosy + sinx * siny * sinz; MAT(trs, 0, 1) = -sinz * cosy + sinx * siny * cosz; MAT(trs, 0, 2) = cosx * siny;
		MAT(trs, 1, 0) = cosx * sinz; MAT(trs, 1, 1) = cosx * cosz; MAT(trs, 1, 2) = -sinx;
		MAT(trs, 2, 0) = -cosz * siny + sinx * cosy * sinz; MAT(trs, 2, 1) = sinz * siny + sinx * cosy * cosz; MAT(trs, 2, 2) = cosx * cosy;
		MAT(trs, 0, 0) *= scale.x; MAT(trs, 1, 0) *= scale.x; MAT(trs, 2, 0) *= scale.x;
		MAT(trs, 0, 1) *= scale.y; MAT(trs, 1, 1) *= scale.y; MAT(trs, 2, 1) *= scale.y;
		MAT(trs, 0, 2) *= scale.z; MAT(trs, 1, 2) *= scale.z; MAT(trs, 2, 2) *= scale.z;
		MAT(trs, 0, 3) = translation.x; MAT(trs, 1, 3) = translation.y; MAT(trs, 2, 3) = translation.z;
		return trs;
	}

	template<typename T>
	Matrix<4, 4, T> Matrix<4, 4, T>::Translate(const Vec<T, 3>& translation)
	{
		Matrix<4, 4, T> mat = Matrix<4, 4, T>::Identity;
		MAT(mat, 0, 3) = translation.x;
		MAT(mat, 1, 3) = translation.y;
		MAT(mat, 2, 3) = translation.z;
		return mat;
	}

	template<typename T>
	Matrix<4, 4, T> Matrix<4, 4, T>::Rotate(const Vec<T, 3>& rotation)
	{
		float radx = sbm::radians(rotation.x), rady = sbm::radians(rotation.y), radz = sbm::radians(rotation.z);
		float cosx = sbm::cos(radx), sinx = sbm::sin(radx), cosy = sbm::cos(rady), siny = sbm::sin(rady), cosz = sbm::cos(radz), sinz = sbm::sin(radz);
		auto mat = Matrix<4, 4, T>::Identity;
		MAT(mat, 0, 0) = cosz * cosy + sinx * siny * sinz; MAT(mat, 0, 1) = -sinz * cosy + sinx * siny * cosz; MAT(mat, 0, 2) = cosx * siny;
		MAT(mat, 1, 0) = cosx * sinz; MAT(mat, 1, 1) = cosx * cosz; MAT(mat, 1, 2) = -sinx;
		MAT(mat, 2, 0) = -cosz * siny + sinx * cosy * sinz; MAT(mat, 2, 1) = sinz * siny + sinx * cosy * cosz; MAT(mat, 2, 2) = cosx * cosy;
		return mat;
	}

	template<typename T>
	Matrix<4, 4, T> Matrix<4, 4, T>::Scale(const Vec<T, 3>& scale)
	{
		auto mat = Matrix<4, 4, T>::Identity;
		MAT(mat, 0, 0) = scale.x;
		MAT(mat, 1, 1) = scale.y;
		MAT(mat, 2, 2) = scale.z;
		return mat;
	}

	template<typename T>
	Matrix<4, 4, T> Matrix<4, 4, T>::LookAt(const Vec<T, 3>& eye, const Vec<T, 3>& center, const Vec<T, 3>& up)
	{
		auto view = Matrix<4, 4, T>::Identity;
		auto front = (center - eye).Normalized();
		auto right = Cross(front, up).Normalized();
		auto upNorm = Cross(right, front).Normalized();
		view.SetRow(0, right.x, right.y, right.z, -Dot(right, eye));
		view.SetRow(1, upNorm.x, upNorm.y, upNorm.z, -Dot(upNorm, eye));
		view.SetRow(2, -front.x, -front.y, -front.z, Dot(front, eye)); // 右手系的相机空间也是右手系的
		view.SetRow(3, 0, 0, 0, 1.f);
		return view;
	}

	template<typename T>
	inline sbm::Vec<T, 4> operator*(const sbm::Vec<T, 4>& v4, const sbm::Matrix<4, 4, T>& m4)
	{
		return sbm::Vec<T, 4>(
			v4[0] * MAT(m4, 0, 0) + v4[1] * MAT(m4, 1, 0) + v4[2] * MAT(m4, 2, 0) + v4[3] * MAT(m4, 3, 0),
			v4[0] * MAT(m4, 0, 1) + v4[1] * MAT(m4, 1, 1) + v4[2] * MAT(m4, 2, 1) + v4[3] * MAT(m4, 3, 1),
			v4[0] * MAT(m4, 0, 2) + v4[1] * MAT(m4, 1, 2) + v4[2] * MAT(m4, 2, 2) + v4[3] * MAT(m4, 3, 2),
			v4[0] * MAT(m4, 0, 3) + v4[1] * MAT(m4, 1, 3) + v4[2] * MAT(m4, 2, 3) + v4[3] * MAT(m4, 3, 3)
			);
	}

	template<typename T>
	template<size_t NX>
	Matrix<4, NX, T> Matrix<4, 4, T>::operator*(const Matrix<4, NX, T>& m4x)
	{
		Matrix<4, NX, T> temp;
		for (int x = 0; x < NX; ++x)
		{
			temp[4 * x] = _M(0, 0) * MAT(m4x, 0, x) + _M(0, 1) * MAT(m4x, 1, x) + _M(0, 2) * MAT(m4x, 2, x) + _M(0, 3) * MAT(m4x, 3, x);
			temp[4 * x + 1] = _M(1, 0) * MAT(m4x, 0, x) + _M(1, 1) * MAT(m4x, 1, x) + _M(1, 2) * MAT(m4x, 2, x) + _M(1, 3) * MAT(m4x, 3, x);
			temp[4 * x + 2] = _M(2, 0) * MAT(m4x, 0, x) + _M(2, 1) * MAT(m4x, 1, x) + _M(2, 2) * MAT(m4x, 2, x) + _M(2, 3) * MAT(m4x, 3, x);
			temp[4 * x + 3] = _M(3, 0) * MAT(m4x, 0, x) + _M(3, 1) * MAT(m4x, 1, x) + _M(3, 2) * MAT(m4x, 2, x) + _M(3, 3) * MAT(m4x, 3, x);
		}
		return temp;
	}

	template<typename T>
	std::ostream& operator<<(std::ostream& out, const Matrix<4, 4, T>& m4)
	{
		out << MAT(m4, 0, 0) << " " << MAT(m4, 0, 1) << " " << MAT(m4, 0, 2) << " " << MAT(m4, 0, 3) << "\n"
			<< MAT(m4, 1, 0) << " " << MAT(m4, 1, 1) << " " << MAT(m4, 1, 2) << " " << MAT(m4, 1, 3) << "\n"
			<< MAT(m4, 2, 0) << " " << MAT(m4, 2, 1) << " " << MAT(m4, 2, 2) << " " << MAT(m4, 2, 3) << "\n"
			<< MAT(m4, 3, 0) << " " << MAT(m4, 3, 1) << " " << MAT(m4, 3, 2) << " " << MAT(m4, 3, 3) << "\n";
		return out;
	}

#undef MAT
#undef _M

}

namespace
{
	template<typename T>
	sbm::Matrix<4, 4, T> CreateIdentityMatrix()
	{
		sbm::Matrix<4, 4, T> temp;
		temp.SetIdentity();
		return temp;
	}
}

namespace sbm
{
	template<typename T>
	const Matrix<4, 4, T> Matrix<4, 4, T>::Identity = CreateIdentityMatrix<T>();
}

typedef sbm::Matrix<4, 4, float> Mat4;

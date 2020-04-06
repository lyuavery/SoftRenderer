#pragma once

#include <cassert>
#include <cmath>
#include "Vector.h"
#include "Utility.h"
#include <type_traits>
#include <algorithm>
#include <vector>
namespace sbm
{
#define MAT(m, r, c) (m).value[(r) + (c) * NR]
#define _M(r, c) value[(r) + (c) * NR]
#define NTOTAL (NR) * (NC)
	template<size_t NR, size_t NC, typename T = float>
	struct Matrix
	{
	protected:
		T value[NR*NC];// column major
	public:
		Matrix() = default;
		Matrix(const Matrix&) = default;

		explicit Matrix(T& v);
		explicit Matrix(T data[NTOTAL]);
		explicit Matrix(const std::vector<T>& data);
		
		void SetRow(int i, const std::vector<T>& row);
		void SetRow(int i, const sbm::Vec<T, NC>& row);
		sbm::Vec<T, NC> GetRow(int i);

		void SetColumn(int i, const std::vector<T>& col);
		void SetColumn(int i, const sbm::Vec<T, NR>& col);
		sbm::Vec<T, NR> GetColumn(int i);

		T& M(int r, int c);
		const T& M(int r, int c) const;

		void SetIdentity();
		Matrix GetInversed();
		static bool InverseMatrix(const Matrix& in, Matrix& out);
		Matrix<NC, NR, T> GetTransposed();

		T& operator[](int i);
		const T& operator[](int i) const;

		sbm::Vec<T, NR> operator*(const sbm::Vec<T, NC>& v);
		template<size_t NR, size_t NC, typename T> friend sbm::Vec<T, 4> operator*(const sbm::Vec<T, NR>& v, const Matrix<NR, NC, T>& m);

		template<size_t NX> Matrix<NR, NX, T> operator*(const Matrix<NC, NX, T>& m);

		inline const size_t RowSize() const { return NR; }
		inline const size_t ColSize() const { return NC; }

	};

	template<size_t NR, size_t NC, typename T>
	Matrix<NR,NC,T>::Matrix(T& v)
	{
		static_assert(NR == NC, "Matrix is not squared.");
		for (int c = 0; c < NC; ++c)
		{
			for (int r = 0; r < NR; ++r)
			{
				_M(r, c) = r == c ? v : T(0);
			}
		}
	}

	template<size_t NR, size_t NC, typename T>
	Matrix<NR, NC, T>::Matrix(T data[NTOTAL])
	{
		int i = 0;
		for (T& val : data) value[i++] = val;
	}

	template<size_t NR, size_t NC, typename T>
	Matrix<NR, NC, T>::Matrix(const std::vector<T>& data)
	{
		int i = 0;
		for (const T& val : data) value[i++] = val;
	}

	template<size_t NR, size_t NC, typename T>
	inline void Matrix<NR, NC, T>::SetIdentity()
	{
		static_assert(NR == NC, "Matrix is not squared.");
		for (int i = 0; i < NC; ++i) _M(i,i) = T(1);
	}

	template<size_t NR, size_t NC, typename T>
	inline void Matrix<NR, NC, T>::SetRow(int r, const std::vector<T>& row)
	{
		assert(r >= 0 && r < NR);
		for (int c = 0; c < NC; ++c) _M(r, c) = row[c];
	}

	template<size_t NR, size_t NC, typename T>
	inline void Matrix<NR, NC, T>::SetRow(int r, const sbm::Vec<T, NC>& row)
	{
		assert(r >= 0 && r < NR);
		for (int c = 0; c < NC; ++c) _M(r, c) = row.M(r,c);
	}

	template<size_t NR, size_t NC, typename T>
	inline sbm::Vec<T, NC> Matrix<NR, NC, T>::GetRow(int r)
	{
		assert(r >= 0 && r < NR);
		sbm::Vec<T, NC> v;
		for (int c = 0; c < NC; ++c) v[c] = _M(r, c);
		return v;
	}

	template<size_t NR, size_t NC, typename T>
	inline void Matrix<NR, NC, T>::SetColumn(int c, const std::vector<T>& col)
	{
		assert(c >= 0 && c < NC);
		for (int r = 0; r < NC; ++r) _M(r, c) = col[r];
	}

	template<size_t NR, size_t NC, typename T>
	inline void Matrix<NR, NC, T>::SetColumn(int c, const sbm::Vec<T, NR>& col)
	{
		assert(c >= 0 && c < NC);
		for (int r = 0; r < NR; ++r) _M(r, c) = col[r];
	}

	template<size_t NR, size_t NC, typename T>
	inline sbm::Vec<T, NR> Matrix<NR, NC, T>::GetColumn(int c)
	{
		assert(c >= 0 && c < NC);
		sbm::Vec<T, NR> v;
		for (int r = 0; r < NR; ++r) v[r] = _M(r, c);
		return v;
	}

	template<size_t NR, size_t NC, typename T>
	inline T& Matrix<NR, NC, T>::M(int r, int c)
	{
		assert(r >= 0 && r < NR && c >= 0 && c < NC);
		return _M(r, c);
	}

	template<size_t NR, size_t NC, typename T>
	inline const T& Matrix<NR, NC, T>::M(int r, int c) const
	{
		assert(r >= 0 && r < NR && c >= 0 && c < NC);
		return _M(r, c);
	}

	template<size_t NR, size_t NC, typename T>
	inline T& Matrix<NR, NC, T>::operator[](int i)
	{
		assert(i >= 0 && i < NTOTAL);
		return value[i];
	}

	template<size_t NR, size_t NC, typename T>
	inline const T& Matrix<NR, NC, T>::operator[](int i) const
	{
		assert(i >= 0 && i < NTOTAL);
		return value[i];
	}

	template<size_t NR, size_t NC, typename T>
	sbm::Vec<T, NR> Matrix<NR, NC, T>::operator*(const sbm::Vec<T, NC>& v)
	{
		sbm::Vec<T, NR> temp;
		for (int r = 0; r < NR; ++r)
		{
			T sum = T(0);
			for (int c = 0; c < NC; ++c)
			{
				sum += _M(r, c) * v[c];
			}
			temp[r] = sum;
		}
		return temp;
	}

	template<size_t NR, size_t NC, typename T>
	template<size_t NX>
	Matrix<NR, NX, T> Matrix<NR, NC, T>::operator*(const Matrix<NC, NX, T>& m)
	{
		Matrix<NR, NX, T> temp;
		for (int x = 0; x < NX; ++x)
		{
			for (int r = 0; r < NR; ++r)
			{
				T sum = T(0);
				for (int c = 0; c < NC; ++c)
				{
					sum += _M(r, c) * m.M(c, x);
				}
				temp.M(r, x) = sum;
			}
		}
		return temp;
	}

#define SWAP_ROWS(a, b) { auto _tmp = a; (a)=(b); (b)=_tmp; }
#define RETURN_ZERO(m) \
{ \
	for (int i=0;i<NTOTAL;++i) \
		(m).value[i] = T(0); \
	return false; \
}

	template<size_t NR, size_t NC, typename T>
	bool Matrix<NR, NC, T>::InverseMatrix(const Matrix<NR, NC, T>& in, Matrix<NR, NC, T>& out)
	{
		static_assert(NR == NC, "Matrix is not squared.");
		const int NCx2 = NC * 2;
		T wtmp[NR][NCx2];

		for (int r = 0; r < NR; ++r)
		{
			for (int c = 0; c < NC; ++c)
			{
				wtmp[r][c] = in.M(r, c);
			}
			for (int c = NC; c < NCx2; ++c)
			{
				wtmp[r][c] = ((c - NC) == r) ? T(1) : T(0);
			}
		}

		T* rowPtr[NR];
		for (int r = 0; r < NR; ++r) { rowPtr[r] = wtmp[r]; }

		// Pass 1 行阶梯形矩阵Row-Echelon Form
		for (int i = 0; i < NR - 1; ++i) // 正在整理出阶梯阵的第i行
		{
			for (int r = NR - 1; r >= i + 1; --r) { // 冒泡出正确的行到第i行
				if (sbm::abs(rowPtr[r][0]) > sbm::abs(rowPtr[r - 1][0])) SWAP_ROWS(rowPtr[r], rowPtr[r - 1]);
			}
			if (T(0) == rowPtr[i][i]) RETURN_ZERO(out); // 非零位没有严格增大，不可逆
			for (int j = i + 1; j < NR; ++j) // 以第i行为枢纽，应用变换到i+1行及以下行，以消除第i行的首非零元位对应其他行的列位
			{
				T coeff = rowPtr[j][i];
				if (T(0) == coeff) continue;
				for (int k = i + 1; k < NCx2; ++k)
				{
					rowPtr[j][k] -= coeff * rowPtr[i][k];
				}
			}
		}

		// Pass 2 回代和消元
		T coeff = T(1) / rowPtr[NR - 1][NR - 1];
		for (int i = NC; i < NCx2; ++i) rowPtr[NR - 1][i] *= coeff; // 规范化最下面的一行
		for (int i = NR - 1; i >= 1; --i) // 正在以第i行位基础消除各行第i列
		{
			coeff = T(1) / rowPtr[i-1][i-1];
			for (int j = 0; j < i; ++j) // 正在消除第j行
			{
				for (int k = NC; k < NCx2; ++k)
				{
					rowPtr[j][k] -= rowPtr[j][i] * rowPtr[i][k]; // 正在消除第j行各个元素k
					if (j == i - 1) rowPtr[j][k] *= coeff; // 第i-1行需要规范化
				}
			}
		}

		for (int c = 0; c < NC; ++c)
		{
			for (int r = 0; r < NR; ++r)
			{
				out.M(r, c) = rowPtr[r][c + NC];
			}
		}
		return true;
	}

	template<size_t NR, size_t NC, typename T>
	Matrix<NR, NC, T> Matrix<NR, NC, T>::GetInversed()
	{
		Matrix<NR, NC, T> temp;
		bool suc = InverseMatrix(*this, temp);
		//if (!suc) LogWarning("Matrix irreversible.");
		return temp;
	}
#undef SWAP_ROWS
#undef RETURN_ZERO

	template<size_t NR, size_t NC, typename T>
	Matrix<NC, NR, T> Matrix<NR, NC, T>::GetTransposed()
	{
		Matrix<NC, NR, T> temp;
		for (int c = 0; c < NC; ++c)
		{
			for (int r = 0; r < NR; ++r)
			{
				MAT(temp, c, r) = _M(r, c);
			}
		}
		return temp;
	}

	template<size_t NR, size_t NC, typename T>
	sbm::Vec<T, NC> operator*(const sbm::Vec<T, NR>& v, const sbm::Matrix<NR, NC, T>& m)
	{
		sbm::Vec<T, NC> temp;
		for (int c = 0; c < NC; ++c)
		{
			T sum = T(0);
			for (int r = 0; r < NR; ++r)
			{
				sum += m.M(r, c) * v[r];
			}
			temp[c] = sum;
		}
		return temp;
	}
#undef MAT
#undef _M
#undef NTOTAL
}

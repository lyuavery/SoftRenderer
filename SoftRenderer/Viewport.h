#pragma once
#include "Math/Matrix4x4.h"
#include "Math/Vector.h"
namespace SR
{
	class Viewport
	{
	public:
		int x;
		int y;
		int width;
		int height;
		double nearVal;
		double farVal;
		bool bTopDown;
		Viewport() = delete;
		Viewport(int xOffset, int yOffset, int w, int h, bool td, double n = 0, double f = 1) // n,f是相当于glDepthRange，将NDC.z映射到window space时的变换
			:x(xOffset), y(yOffset), width(w), height(h), bTopDown(td),nearVal(n),farVal(f)
		{}

		inline Mat4 GetViewportMatrix() const
		{
			return GetViewportMatrix(x, y, width, height, nearVal, farVal, bTopDown);
		}

		template<typename T>
		inline void ApplyViewportTransform(sbm::Vec<T,4>& v) const {
			float halfW = width * 0.5f, halfH = height * .5f;
			int flipH = bTopDown ? -1 : 1;
			v.x = halfW * v.x + (halfW + x);
			v.y = flipH * halfH * v.y + (halfH + y);
			v.z = ((farVal - nearVal) / 2.) * v.z + (farVal + nearVal) / 2.;
		}

		inline static Mat4 GetViewportMatrix(int x, int y, int width, int height, double nearV, double farV, bool topDown)
		{
			Mat4 mat = Mat4::Identity;
			float halfW = width * 0.5f, halfH = height * .5f;
			int flipH = topDown ? -1 : 1;
			mat.M(0, 0) = halfW; mat.M(0, 3) = halfW + x;
			// 因为windows buffer设置成了top-down，这也符合对buffer起始位置的直观理解，所以要想避免绘制后垂直翻转，就需要对视口进行翻转，所以这里y坐标系数需要乘以-1
			mat.M(1, 1) = flipH * halfH; mat.M(1, 3) = halfH + y;
			mat.M(2, 2) = (farV - nearV) / 2.; mat.M(2, 3) = (farV + nearV) / 2.;
			return mat;
		}

		static Viewport main;
	};
}


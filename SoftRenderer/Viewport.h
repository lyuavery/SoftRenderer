#pragma once
#include "Math/Matrix4x4.h"
namespace SR
{
	class Viewport
	{
	public:
		float x;
		float y;
		float width;
		float height;
		bool bTopDown;
		Viewport() = delete;
		Viewport(int xOffset, int yOffset, int w, int h, bool td) :x(xOffset), y(yOffset), width(w), height(h), bTopDown(td) {}
		inline Mat4 GetViewportMatrix() const
		{
			return GetViewportMatrix(x, y, width, height, bTopDown);
		}
		inline static Mat4 GetViewportMatrix(int x, int y, int width, int height, bool topDown)
		{
			Mat4 mat = Mat4::Identity;
			float halfW = width * 0.5f, halfH = height * .5f;
			int flipH = topDown ? -1 : 1;
			mat.M(0, 0) = halfW; mat.M(0, 3) = halfW + x;
			// ��Ϊwindows buffer���ó���top-down����Ҳ���϶�buffer��ʼλ�õ�ֱ����⣬����Ҫ�������ƺ�ֱ��ת������Ҫ���ӿڽ��з�ת����������y����ϵ����Ҫ����-1
			mat.M(1, 1) = flipH * halfH; mat.M(1, 3) = halfH + y;
			return mat;
		}
	};
}


#pragma once
#include "RenderStatus.h"
#include "Texture.h"
namespace SR
{
	class FragmentShader
	{
	public:
		static SR::Color texture(const SR::Texture* tex, const Vec2& uv)
		{
			if (!tex) return SR::Color::black;
			int x = (tex->GetWidth() - 1) * uv.x, y = (tex->GetHeight() - 1) * uv.y;
			SR::Color col;
			tex->Get(x, y, col.r, col.g, col.b, col.a);
			return col;
		}
		virtual SR::Color operator()(FragmentShaderInput& in, const std::shared_ptr<const Varying>& varying, const std::shared_ptr<const Uniform>& uniform) = 0;
		virtual FragmentShader* Clone(bool bCopy = false) const = 0;
	};

	template<typename T>
	class IFragmentShader : public FragmentShader
	{
	public:
		virtual FragmentShader* Clone(bool bCopy = false) const override { return new T; }
	};

	
}
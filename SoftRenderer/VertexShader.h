#pragma once
#include "RenderState.h"
namespace SR
{
	class VertexShader
	{
	public:
		virtual VertexShaderOutput operator()(
			VertexShaderInput& attribute
			, const std::shared_ptr<Varying>& varying
			, const std::shared_ptr<const Uniform>& uniform) const = 0;
		virtual VertexShader* Clone(bool bCopy = false) const = 0;
	};

	template<typename T>
	class IVertexShader : public VertexShader
	{
	public:
		virtual VertexShader* Clone(bool bCopy = false) const override { return new T; }
	};

	class CommonVert : public IVertexShader<CommonVert>
	{
	public:
		virtual VertexShaderOutput operator()(VertexShaderInput& in, const std::shared_ptr<Varying>& v, const std::shared_ptr<const Uniform>& u) const override
		{
			VertexShaderOutput out;
			out.gl_Position = Vec4(0, 0, 0, 1);
			return out;
		}
	};
}

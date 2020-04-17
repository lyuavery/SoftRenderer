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

}

#pragma once
#include "RenderState.h"
namespace SR
{
	class FragmentShader
	{
	public:
		virtual SR::Color operator()(FragmentShaderInput& in, const std::shared_ptr<const Varying>& varying, const std::shared_ptr<const Uniform>& uniform) = 0;
		virtual FragmentShader* Clone(bool bCopy = false) const = 0;
	};
}
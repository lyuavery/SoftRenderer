#pragma once
#include "RenderStatus.h"
namespace SR
{
	class FragmentShader
	{
	public:
		virtual SR::Color operator()(FragmentShaderInput& in, const std::shared_ptr<const Varying>& varying, const std::shared_ptr<const Uniform>& uniform) = 0;
		virtual FragmentShader* Clone(bool bCopy = false) const = 0;
	};

	template<typename T>
	class IFragmentShader : public FragmentShader
	{
	public:
		virtual FragmentShader* Clone(bool bCopy = false) const override { return new T; }
	};

#pragma pack(push, 4)
	struct BlinnPhongVarying : IVarying<BlinnPhongVarying>
	{
		
	};
#pragma pack(pop)

	struct BlinnPhongUniform : IUniform<BlinnPhongUniform>
	{

	};

	class BlinnPhongFrag : public IFragmentShader<BlinnPhongFrag>
	{
	public:
		virtual SR::Color operator()(FragmentShaderInput& in, const std::shared_ptr<const Varying>& varying, const std::shared_ptr<const Uniform>& uniform) override
		{
			return SR::Color::black;
		}
	};
}
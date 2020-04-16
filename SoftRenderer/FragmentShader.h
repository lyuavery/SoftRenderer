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

	template<typename T>
	class IFragmentShader : public FragmentShader
	{
	public:
		virtual FragmentShader* Clone(bool bCopy = false) const override { return new T; }
	};

	class CommonFrag : public IFragmentShader<CommonFrag>
	{
	public:
		virtual SR::Color operator()(FragmentShaderInput& in, const std::shared_ptr<const Varying>& v, const std::shared_ptr<const Uniform>& u) override
		{
			return SR::Color::black;
		}
	};

#pragma pack(push, 4)
	struct GauroudVarying : IVarying<GauroudVarying>
	{
	};
#pragma pack(pop)

	struct GauroudUniform : IUniform<GauroudUniform>
	{
	};

	class GauroudFrag : public IFragmentShader<GauroudFrag>
	{
	public:
		virtual SR::Color operator()(FragmentShaderInput& in, const std::shared_ptr<const Varying>& v, const std::shared_ptr<const Uniform>& u) override
		{
			auto varying = std::dynamic_pointer_cast<const GauroudVarying>(v);
			auto uniform = std::dynamic_pointer_cast<const GauroudUniform>(u);

			return SR::Color::black;
		}
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
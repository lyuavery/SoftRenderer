#pragma once
#include "RenderStatus.h"
#include "VertexShader.h"
#include "FragmentShader.h"
#include "Math/Vector.h"
#include "Math/Matrix4x4.h"
#include "Texture.h"
namespace SR
{
#pragma pack(push,4)
	struct DefaultWireframeVarying : IVarying<DefaultWireframeVarying>
	{
	};
#pragma pack(pop)

	struct DefaultWireframeUniform : IUniform<DefaultWireframeUniform>
	{
	};

	class DefaultWireframeVert : public IVertexShader<DefaultWireframeVert>
	{
	public:
		virtual VertexShaderOutput operator()(VertexShaderInput& in, const std::shared_ptr<Varying>& v, const std::shared_ptr<const Uniform>& u) const override
		{
			VertexShaderOutput out;
			auto uniform = std::dynamic_pointer_cast<const DefaultWireframeUniform>(u);
			if (uniform) 
				out.gl_Position = uniform->mat_ObjectToClip * in.position.Expanded<4>(1);
			return out;
		}
	};

	class DefaultWireframeFrag : public IFragmentShader<DefaultWireframeFrag>
	{
	public:
		virtual SR::Color operator()(FragmentShaderInput& in, const std::shared_ptr<const Varying>& v, const std::shared_ptr<const Uniform>& u) override
		{
			//float c = in.gl_FragCoord.z / in.gl_FragCoord.w;
			//return SR::Color(c,c,c,1);
			return SR::Color::black;
		}
	};
}

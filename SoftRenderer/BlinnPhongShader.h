#pragma once
#pragma once
#include "RenderStatus.h"
#include "VertexShader.h"
#include "FragmentShader.h"
#include "Math/Vector.h"
#include "Math/Matrix4x4.h"

namespace SR
{

#pragma pack(push, 4)
	struct BlinnPhongVarying : IVarying<BlinnPhongVarying>
	{
		Vec2 uv;
		Vec3 worldPos;
		Vec3 T2W0;
		Vec3 T2W1;
		Vec3 T2W2;
	};
#pragma pack(pop)

	struct BlinnPhongUniform : IUniform<BlinnPhongUniform>
	{
		Vec3 worldLightDir;
		Vec3 worldCamPos;
		std::shared_ptr<SR::Texture> albedo;
		std::shared_ptr<SR::Texture> spec;
		std::shared_ptr<SR::Texture> normal;
	};

	class BlinnPhongVert : public IVertexShader<BlinnPhongVert>
	{
	public:
		virtual VertexShaderOutput operator()(VertexShaderInput& in, const std::shared_ptr<Varying>& v, const std::shared_ptr<const Uniform>& u) const override
		{
			VertexShaderOutput out;
			auto varying = std::dynamic_pointer_cast<BlinnPhongVarying>(v);
			auto uniform = std::dynamic_pointer_cast<const BlinnPhongUniform>(u);
			if (uniform) out.gl_Position = uniform->mat_ObjectToClip * in.position.Expanded<4>(1);
			if (varying)
			{
				varying->uv = Vec2(in.uv.x, 1 - in.uv.y);
				// no local transform
				Vec3 tangent = in.tangent.Truncated<3>().Normalized();
				Vec3 normal = in.normal.Normalized();
				Vec3 bitangent = in.tangent.w * sbm::Cross(tangent, in.normal).Normalized();
				varying->T2W0 = Vec3(tangent.x, bitangent.x, normal.x);
				varying->T2W1 = Vec3(tangent.y, bitangent.y, normal.y);
				varying->T2W2 = Vec3(tangent.z, bitangent.z, normal.z);
				varying->worldPos = in.position;
			}
			return out;
		}
	};

	class BlinnPhongFrag : public IFragmentShader<BlinnPhongFrag>
	{
	public:
		virtual SR::Color operator()(FragmentShaderInput& in, const std::shared_ptr<const Varying>& v, const std::shared_ptr<const Uniform>& u) override
		{
			auto varying = std::dynamic_pointer_cast<const BlinnPhongVarying>(v);
			auto uniform = std::dynamic_pointer_cast<const BlinnPhongUniform>(u);
			auto albedo =  ColorToVec(texture(uniform->albedo.get(), varying->uv));
			float gloss = texture(uniform->spec.get(), varying->uv).a * 255;
			auto n = texture(uniform->normal.get(), varying->uv).Truncated<3>().Normalized();// * 255
			Vec3 lightDir = (-uniform->worldLightDir).Normalized();
			Vec3 viewDir = (uniform->worldCamPos - varying->worldPos).Normalized();
			Vec3 worldNormal = Vec3(
				sbm::Dot(varying->T2W0, n),
				sbm::Dot(varying->T2W1, n),
				sbm::Dot(varying->T2W2, n)
			).Normalized();
			
			// Diffuse
			auto diffuse = sbm::Dot(worldNormal, lightDir) * 0.5f + 0.5f;
			auto ambient = Vec4(0.2f, 0.2f, 0.2f, 1);
			auto spec = sbm::pow(sbm::clamp01(sbm::Dot((lightDir + viewDir).Normalized(), worldNormal)), gloss);
			return VecToColor(albedo * (diffuse + spec  + ambient ));
		}
	};
}

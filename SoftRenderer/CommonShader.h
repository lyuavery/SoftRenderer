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
	struct CommonVarying : IVarying<CommonVarying>
	{
		Vec2 uv;
		Vec3 worldPos;
		Vec3 worldNormal;
	};
#pragma pack(pop)

	struct CommonUniform : IUniform<CommonUniform>
	{
		Vec3 worldLightDir;
		Vec3 worldCamPos;
		Mat4 mat_WorldToObject;
	};

	class CommonVert : public IVertexShader<CommonVert>
	{
	public:
		virtual VertexShaderOutput operator()(VertexShaderInput& in, const std::shared_ptr<Varying>& v, const std::shared_ptr<const Uniform>& u) const override
		{
			VertexShaderOutput out;
			auto varying = std::dynamic_pointer_cast<CommonVarying>(v);
			auto uniform = std::dynamic_pointer_cast<const CommonUniform>(u);
			if(uniform) out.gl_Position = uniform->mat_ObjectToClip * in.position.Expanded<4>(0);
			if (varying)
			{
				varying->uv = Vec2(in.uv.x, 1 - in.uv.y);
				varying->worldPos = (uniform->mat_ObjectToWorld * in.position.Expanded<4>(1)).Truncated<3>();
				varying->worldNormal = (in.normal.Expanded<4>(0) * uniform->mat_WorldToObject).Truncated<3>();
				//int x = 1;
				//varying->worldNormal = in.tangent.Truncated<3>();
				//varying->worldNormal = Mat3(uniform->mat_ObjectToWorld) * varying->worldNormal;
			}
			return out;
		}
	};

	class CommonFrag : public IFragmentShader<CommonFrag>
	{
	public:
		virtual SR::Color operator()(FragmentShaderInput& in, const std::shared_ptr<const Varying>& v, const std::shared_ptr<const Uniform>& u) override
		{
			auto varying = std::dynamic_pointer_cast<const CommonVarying>(v);
			auto uniform = std::dynamic_pointer_cast<const CommonUniform>(u);
			
			Vec3 lightDir = (-uniform->worldLightDir).Normalized();
			Vec3 viewDir = (uniform->worldCamPos - varying->worldPos).Normalized();

			Vec3 worldNormal = varying->worldNormal.Normalized();
			auto refl = reflect(uniform->worldLightDir, worldNormal).Normalized();
				// Diffuse
			auto diffuse = sbm::Dot(worldNormal, lightDir) * 0.5f + 0.5f;
			auto ambient = Vec4(0.1f, 0.1f, 0.1f, 1);
			auto spec = sbm::pow(sbm::clamp01(sbm::Dot((lightDir + viewDir).Normalized(), worldNormal)), 32.f);

			return VecToColor( (0.8f * diffuse + 0.3f * spec + ambient));
				
				// Test Box
				//ºÚ£¬»Ò£¬°×£¬
				//°ëºì£¬°ëÂÌ£¬°ëÀ¶£¬
				//ºì£¬ÂÌ£¬À¶£¬
				//»Æ£¬×Ï£¬µå
				/*switch (in.gl_PrimitiveID)
				{
				case 0:
					return SR::Color(0, 0, 0, 1);
					break;
				case 1:
					return SR::Color(.25f, .25f, .25f, 1);
					break;
				case 2:
					return SR::Color(1,1,1, 1);
					break;
				case 3:
					return SR::Color(.5f, 0, 0, 1);
					break;
				case 4:
					return SR::Color(0, .5f, 0, 1);
					break;
				case 5:
					return SR::Color(0, 0, .5f, 1);
					break;
				case 6:
					return SR::Color(1, 0, 0, 1);
					break;
				case 7:
					return SR::Color(0, 1, 0, 1);
					break;
				case 8:
					return SR::Color(0, 0, 1, 1);
					break;
				case 9:
					return SR::Color(1, 1, 0, 1);
					break;
				case 10:
					return SR::Color(1, 0, 1, 1);
					break;
				case 11:
					return SR::Color(0, 1, 1, 1);
					break;
				}*/

			//return SR::Color::black;
		}
	};
}

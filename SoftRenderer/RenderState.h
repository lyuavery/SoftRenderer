#pragma once
#include <memory>
#include <vector>
#include "Math/Vector.h"
#include "Color.h"
namespace SR
{
	struct Varying { // ÷ªΩ” ‹float
		virtual int size() const final { return sizeof(*this) / 4; }
		virtual Varying* Clone(bool bCopy = false) const = 0;
	};

	struct Uniform {
		virtual Uniform* Clone(bool bCopy = false) const = 0;
	};

	enum class BlendOp
	{
		One,
		Zero,
	};

	enum class FrontFace
	{
		CCW,
		CW,
	};

	enum class Culling
	{
		Front,
		Back,
		Off
	};

	enum class PrimitiveAssemblyMode
	{
		Triangles
	};

	enum class DepthFunc
	{
		Greater,
		Less,
		LEqual,
		GEqual,
		Equal,
		Always
	};

	enum class InterpolationMode
	{
		Nonperspective,
		Smooth,
	};

	struct VertexShaderInput
	{
		Vec3 position;
		Vec3 normal;
		Vec2 uv;
		Vec4 color;
		Vec4 tangent;
		int gl_VertexID;
	};

	struct VertexShaderOutput
	{
		int gl_VertexID;
		Vec4 gl_Position;
		std::shared_ptr<Varying> varying;
	};

	struct PrimitiveAssemblyOutput
	{
		// and the result is still in the cache, https://www.khronos.org/opengl/wiki/Vertex_Shader
		int gl_PrimitiveID; // out
		bool gl_FrontFacing; // out
		std::vector<std::shared_ptr<VertexShaderOutput>> vertices;
	};

	struct RasterOutput
	{
		int gl_PrimitiveID; // in
		Vec4 gl_FragCoord; // in
		bool gl_FrontFacing; // in
		float gl_FragDepth; // in
		std::shared_ptr<Varying> varying;
	};

	typedef RasterOutput FragmentShaderInput;

	struct FragmentShaderOutput
	{
		Vec4 gl_FragCoord; // in
		float gl_FragDepth; // in
		SR::Color color;
	};

	struct RenderState
	{
		bool bEarlyDepthTest = true;
		bool bPostTransformCache = true;
		BlendOp srcOp = BlendOp::One;
		BlendOp dstOp = BlendOp::Zero;
		DepthFunc depthFunc = DepthFunc::Always;
		Culling cullFace = Culling::Off;
		FrontFace face = FrontFace::CCW;
		PrimitiveAssemblyMode primitiveAssemblyMode = PrimitiveAssemblyMode::Triangles;
		InterpolationMode interpolationMode = InterpolationMode::Smooth;
	};
}

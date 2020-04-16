#pragma once

#include "RenderState.h"
#include "VertexShader.h"
#include "FragmentShader.h"
#include "Log.h"
#include "Math/Vector.h"
#include "Color.h"
#include "Header.h"
#include "Texture.h"
#include "Math/Utility.h"
#include "Mesh.h"
#include "FrameBuffer.h"
#include "Viewport.h"

#include <queue>
#include <unordered_map>
#include <memory>


namespace SR
{
	// class Rasterizer
	// class Interpolator
	// class PrimitiveAssembler
	// class Blender;

	class PostTransformCache
	{
		friend class Renderer;
		std::unordered_map<int, std::shared_ptr<VertexShaderOutput>> data;
		std::queue<int> fifo;
		int max;
		int cursize;
	public:
		PostTransformCache(int size) :data(size), max(size), cursize(0)
		{}

		inline bool Get(int vid)
		{
			if (data.find(vid) == data.end()) return false;
			return true;
		}

		inline bool Get(int vid, std::shared_ptr<VertexShaderOutput>& out) // 这里要lock data
		{
			if (data.find(vid) == data.end()) return false;
			out = data[vid];
			return true;
		}

		void Cache(int vid, std::shared_ptr<VertexShaderOutput>& in)
		{
			if (data.find(vid) != data.end()) return;
			if (cursize >= max) {
				auto i = fifo.front();
				if (i == vid) return;
				fifo.pop();
				data.erase(i);
			}
			fifo.push(vid);
			data[vid] = in;
		}
	};

	class VSDispatcher
	{
		friend class Renderer;
	public:

		std::shared_ptr<VertexShader> shader;
		std::shared_ptr<Varying> varying;
		std::shared_ptr<const Uniform> uniform;
		VertexShaderOutput Dispatch(int iid, int vid, VertexShaderInput& input) // TODO：做多线程分发的
		{
			std::shared_ptr<Varying> v(varying ? varying->Clone() : nullptr);
			VertexShaderOutput output = (*shader)(input, v, uniform);
			if (output.gl_Position.w == 0) output.gl_Position.w = 0.0000001f;
			output.gl_VertexID = vid;
			output.varying = v;
			return output;
		}
		//VertexShaderOutput Wrapper();
	};

	class FSDispatcher
	{
		friend class Renderer;
	public:
		std::shared_ptr<FragmentShader> shader;
		std::shared_ptr<const Uniform> uniform;
		bool bEarlyDepthTest = false;
		FragmentShaderOutput Dispatch(FragmentShaderInput& input) // TODO：做多线程分发的
		{
			FragmentShaderOutput output;
			std::shared_ptr<const Varying> varying = std::const_pointer_cast<const Varying>(input.varying);
			input.varying.reset();
			output.gl_FragCoord = input.gl_FragCoord;
			output.color = (*shader)(input, varying, uniform);
			if (!bEarlyDepthTest) output.gl_FragDepth = input.gl_FragDepth;
			return output;
		}
	};

	class RenderTask
	{
		friend class Renderer;
		std::shared_ptr<SR::Mesh> mesh;
		std::shared_ptr<Uniform> uniform;
		std::shared_ptr<Varying> varying;
		std::shared_ptr<VertexShader> vert;
		std::shared_ptr<FragmentShader> frag;
		std::shared_ptr<SR::Viewport> viewport;
	public:
		RenderState state;
		std::shared_ptr<FrameBuffer> frameBuffer;
		void Viewport(const SR::Viewport* const v)
		{
			if (v) viewport.reset(new SR::Viewport(*v));;
		}

		void Bind(const Uniform* const u)
		{
			if(u) uniform.reset(u->Clone());
		}

		void Bind(const VertexShader* const vs, const Varying* const v = nullptr)
		{
			if(vs) vert.reset(vs->Clone());
			if(v) varying.reset(v->Clone());
		}

		void Bind(const FragmentShader* const f)
		{
			if (f) frag.reset(f->Clone());
		}

		void Bind(const SR::Mesh* const m, int attrib = -1)
		{
			if(m) mesh.reset(new Mesh(*m, attrib));
		}

		void Submit();
	};

	class Renderer
	{
		std::queue<std::shared_ptr<VertexShaderOutput>> vsOutputs;
		std::queue<std::shared_ptr<PrimitiveAssemblyOutput>> paOutputs;
		std::queue<std::shared_ptr<RasterOutput>> rasterOutputs;
		std::queue<std::shared_ptr<FragmentShaderOutput>> fsOutputs;
		std::queue<RenderTask> tasks;

		std::unique_ptr<VSDispatcher> vsDispatcher;
		std::unique_ptr<PostTransformCache> cache;
		//std::unique_ptr<PrimitiveAssembler> primitiveAssembler;
		//std::unique_ptr<Interpolator> interpolator;
		//std::unique_ptr<Rasterizer> rasterizer;
		std::unique_ptr<FSDispatcher> fsDispatcher;
		//std::unique_ptr<Blender> blender;

		static const int CACHE_SIZE = 30;
		Renderer()
			:vsDispatcher(new VSDispatcher)
			, cache(new PostTransformCache(CACHE_SIZE))
			//,primitiveAssembler(new PrimitiveAssembler)
			//, rasterizer(new Rasterizer)
			, fsDispatcher(new FSDispatcher)
			//,blender(new Blender)
		{
		}
	public:

		void Push(const RenderTask& task)
		{
			tasks.push(task);
		}

		static Renderer& GetInstance()
		{
			static Renderer* instance = new Renderer;

			return *instance;
		}

		//std::vector<std::unique_ptr< // cache hit要求gl_VertexID和gl_InstanceID 都与cache相等，这里不考虑实例化渲染，只考虑gl_VertexID
		void VertexProcessing(const std::shared_ptr<SR::Mesh>& mesh, bool cacheEnabled);

		// 前一个drawcall的图元的处理一定先于后一个drawcall的所有图元 https://www.khronos.org/opengl/wiki/Primitive_Assembly
		// Assembly, Face Culling
		void PrimitiveAssembly(PrimitiveAssemblyMode paMode, Culling cullFace, FrontFace face);

		// Clipping, Perspective Divide, Viewport Transform
		void VertexPostProcessing(const Viewport& viewport);

		// Interpolation, Perspective Correction
		void Rasterizing(PrimitiveAssemblyMode primitiveAssemblyMode, InterpolationMode interpolationMode, const SR::Viewport& viewport);
		
		void Shading();
		
		template<typename T>
		void DepthTesting(DepthFunc f, const std::shared_ptr<const FrameBuffer>& fb, T& data); // Early Depth Test
		
		void Blending(BlendOp srcOp, BlendOp dstOp, const std::shared_ptr<FrameBuffer>& fb);
		
		void RenderAll();
		
	};

}


template<typename T>
void SR::Renderer::DepthTesting(DepthFunc f, const std::shared_ptr<const FrameBuffer>& fb, T& data) // Early Depth Test
{
	if (f == DepthFunc::Always || !fb->depthBuf) return;
	while (!data.empty())
	{
		auto fragmentPtr = data.front();
		auto& fragment = *fragmentPtr;
		data.pop();
		float z = fragment.gl_FragDepth;
		float bufZ, tmp;
		fb->depthBuf->Get(fragment.gl_FragCoord.x, fragment.gl_FragCoord.y, bufZ, tmp, tmp, tmp);
		bool pass = true;
		switch (f)
		{
		case DepthFunc::Greater: { pass = z > bufZ; break; }
		case DepthFunc::Less: { pass = z < bufZ; break; }
		case DepthFunc::LEqual: { pass = z <= bufZ; break; }
		case DepthFunc::GEqual: { pass = z >= bufZ; break; }
		case DepthFunc::Equal: { pass = z == bufZ; break; }
		}
		if (pass) data.push(fragmentPtr);
	}
}

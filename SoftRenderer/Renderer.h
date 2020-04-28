#pragma once

#include "RenderStatus.h"
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
#include "DefaultWireframeShader.h"

#include <queue>
#include <unordered_map>
#include <memory>

namespace SR
{
	class PostTransformCache
	{
		std::unordered_map<int, VertexShaderOutput> data;
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

		inline bool Get(int vid, VertexShaderOutput& out) // 这里要lock data
		{
			if (data.find(vid) == data.end()) return false;
			out = data[vid];
			return true;
		}

		void Cache(int vid, const VertexShaderOutput& in)
		{
			if (data.find(vid) != data.end()) return;
			if (cursize == max) {
				cursize = max;
				auto i = fifo.front();
				if (i == vid) return;
				fifo.pop();
				data.erase(i);
			}
			else
			{
				++cursize;
			}
			fifo.push(vid);
			data[vid] = in;
		}
	
		void Clear()
		{
			cursize = 0;
			data.clear();
			std::queue<int> tmp;
			fifo.swap(tmp);
		}
	};

	class RendererComponent
	{
	public:
		enum class State
		{
			Running, // 正在准备输出
			Waiting, // 等待输入
			Done, // 空闲
		};
		virtual void Reset() = 0;
	};

	class VertexProcessor : public RendererComponent
	{
	public:
		static const int CACHE_SIZE = 30;

		// args
		std::shared_ptr<SR::VertexShader> vert;
		std::shared_ptr<SR::Varying> varying;
		std::shared_ptr<const SR::Uniform> uniform;
		bool bCacheEnabled = true;

		std::queue<std::shared_ptr<VertexShaderOutput>> outputs;

		State Processing(std::shared_ptr<SR::Mesh>& mesh);
		//State PostProcessing(std::queue<std::shared_ptr<PrimitiveAssemblyOutput>>& data);
		virtual void Reset() override { 
			begin = 0; 
			decltype(outputs) tmp; outputs.swap(tmp);
			cache.Clear();
		}
	private:
		UInt32 begin;
		PostTransformCache cache = PostTransformCache(CACHE_SIZE);
		VertexShaderOutput Dispatch(int vid, VertexShaderInput& input) // TODO：做多线程分发的
		{
			std::shared_ptr<Varying> v(varying ? varying->Clone() : nullptr);
			VertexShaderOutput output = (*vert)(input, v, uniform);
			if (output.gl_Position.w == 0) output.gl_Position.w = 0.0000001f;
			output.gl_VertexID = vid;
			output.varying = v;
			return output;
		}
	};

	class PrimitiveAssembler : public RendererComponent
	{
		int id;
		static constexpr float W_CLIPPED_PLANE = 1E-5;
		enum Axis {
			NEGATIVE_Z = -3,
			NEGATIVE_Y = -2,
			NEGATIVE_X = -1,
			W = 0,
			POSITIVE_X = 1,
			POSITIVE_Y = 2,
			POSITIVE_Z = 3,
		};
		static std::vector<std::shared_ptr<VertexShaderOutput>> HomogeneousClipping(const std::vector<std::shared_ptr<VertexShaderOutput>>& polygon);
		static std::vector<std::shared_ptr<VertexShaderOutput>> ClippingAgainstPlane(const std::vector<std::shared_ptr<VertexShaderOutput>>& polygon, Axis axis);
		static float CalIntersectRatio(const Vec4& ps, const Vec4& pe, Axis axis);
		static inline bool IsVisible(const Vec4& v)
		{
			return (sbm::abs(v.x) <= v.w) && (sbm::abs(v.y) <= v.w) && (sbm::abs(v.z) <= v.w);
		}
		static inline bool IsInside(const Vec4& v, Axis axis)
		{
			bool b = true;
			switch (axis)
			{
			case SR::PrimitiveAssembler::W:
				b = v.w >= W_CLIPPED_PLANE;
				break;
			case SR::PrimitiveAssembler::POSITIVE_X:
				b = v.x <= v.w;
				break;
			case SR::PrimitiveAssembler::NEGATIVE_X:
				b = v.x >= -v.w;
				break;
			case SR::PrimitiveAssembler::POSITIVE_Y:
				b = v.y <= v.w;
				break;
			case SR::PrimitiveAssembler::NEGATIVE_Y:
				b = v.y >= -v.w;
				break;
			case SR::PrimitiveAssembler::POSITIVE_Z:
				b = v.z <= v.w;
				break;
			case SR::PrimitiveAssembler::NEGATIVE_Z:
				b = v.z >= -v.w;
				break;
			default:
				assert(0);
				break;
			}
			return b;
		}
	public:
		// args
		PrimitiveAssemblyMode mode;
		Culling cullFace;
		FrontFace face;
		std::shared_ptr<SR::Viewport> viewport;
		std::queue<std::shared_ptr<PrimitiveAssemblyOutput>> outputs;

		State Assembly(std::queue<std::shared_ptr<VertexShaderOutput>>& data);
		virtual void Reset() override {
			id = 0;
			decltype(outputs) tmp; outputs.swap(tmp);
		}

		static int PrimitiveRequiredVertices(PrimitiveAssemblyMode mode)
		{
			int vnum = 0;
			switch (mode)
			{
			case PrimitiveAssemblyMode::Triangles: { vnum = 3; break; }
			}
			return vnum;
		}
	};

	class Rasterizer : public RendererComponent
	{
		void RasterizeLine(const Vec4& v0, const Vec4& v1, bool frontFace, int primitiveID);
		void RasterizeWireTriangle(const SR::PrimitiveAssemblyOutput& primitive);
		void RasterizeFilledTriangle(const SR::PrimitiveAssemblyOutput& primitive);
	public:
		
		std::shared_ptr<VertexShader> defaultWireframeVert = std::make_shared<DefaultWireframeVert>();
		std::shared_ptr<FragmentShader> defaultWireframeFrag = std::make_shared<DefaultWireframeFrag>();
		std::shared_ptr<Varying> defaultWireframeVarying = std::make_shared<DefaultWireframeVarying>();
		std::shared_ptr<Uniform> defaultWireframeUniform = std::make_shared<DefaultWireframeUniform>();
		// args
		PrimitiveAssemblyMode primitiveAssemblyMode;
		InterpolationMode interpolationMode;
		std::shared_ptr<SR::Viewport> viewport;
		RasterizationMode rasterizationMode;
		std::queue<std::shared_ptr<RasterOutput>> outputs;

		State Rasterizing(std::queue<std::shared_ptr<PrimitiveAssemblyOutput>>& data, SR::RasterizationMode mode);
		virtual void Reset() override {
			decltype(outputs) tmp; outputs.swap(tmp);
		}
	};

	class FragmentProcessor : public RendererComponent
	{
		FragmentShaderOutput Dispatch(FragmentShaderInput& input) // TODO：做多线程分发的
		{
			FragmentShaderOutput output;
			auto varying = std::const_pointer_cast<const Varying>(input.varying);
			input.varying.reset();
			output.gl_FragCoord = input.gl_FragCoord;
			output.gl_FragDepth = input.gl_FragDepth;
			output.color = (*frag)(input, varying, uniform);
			if (!bEarlyDepthTest) output.gl_FragDepth = input.gl_FragDepth;
			return output;
		}
	public:
		// args
		std::shared_ptr<SR::FragmentShader> frag;
		std::shared_ptr<SR::FrameBuffer> frameBuffer;
		std::shared_ptr<const SR::Uniform> uniform;
		DepthFunc depthFunc;
		BlendOp srcOp;
		BlendOp dstOp;
		bool bEarlyDepthTest;

		std::queue<std::shared_ptr<FragmentShaderOutput>> outputs;

		template <typename T, 
			typename = std::enable_if_t<
			std::is_same_v<T, RasterOutput>||std::is_same_v<T, FragmentShaderOutput>, 
			void>>
		State DepthTesting(std::queue<std::shared_ptr<T>>& data);
		State Shading(std::queue<std::shared_ptr<RasterOutput>>& data);
		State Blending(std::queue<std::shared_ptr<FragmentShaderOutput>>& data);
		virtual void Reset() override {
			decltype(outputs) tmp; outputs.swap(tmp);
		}
	};

	class RenderTask
	{
		friend class Renderer;
		std::shared_ptr<SR::Mesh> mesh;
		std::shared_ptr<Varying> varying;
		std::shared_ptr<VertexShader> vert;
		std::shared_ptr<FragmentShader> frag;
		std::shared_ptr<SR::Viewport> viewport;
	public:
		RenderStatus status;
		std::shared_ptr<Uniform> uniform;
		std::shared_ptr<FrameBuffer> frameBuffer;
		void Viewport(const SR::Viewport* const v)
		{
			if (v) viewport.reset(new SR::Viewport(*v));;
		}

		void Bind(const Uniform* const u)
		{
			if(u) uniform.reset(u->Clone(true));
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
		std::queue<RenderTask> tasks;
		
		VertexProcessor vertexProcessor;
		PrimitiveAssembler primitiveAssembler;
		Rasterizer rasterizer;
		FragmentProcessor fragmentProcessor;

		static const int CACHE_SIZE = 30;
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

		void RenderAll();
	};

}

// 图元汇编后的管线流程为逐图元处理，否则测试时前面的图元深度还没写入
template <typename T, typename>
SR::RendererComponent::State SR::FragmentProcessor::DepthTesting(std::queue<std::shared_ptr<T>>& data) // Early Depth Test
{
	auto& depthBuf = frameBuffer->depthBuf;
	if (depthFunc == SR::DepthFunc::Always || !depthBuf) return SR::RendererComponent::State::Done;
	int size = data.size();
	while (size--)
	{
		auto fragmentPtr = data.front();
		auto& fragment = *fragmentPtr;
		data.pop();
		float z = fragment.gl_FragDepth;
		float bufZ;
		depthBuf->Get(fragment.gl_FragCoord.x, fragment.gl_FragCoord.y, bufZ);
		bool pass = true;
		switch (depthFunc)
		{
		case DepthFunc::Greater: { pass = z > bufZ; break; }
		case DepthFunc::Less: { pass = z < bufZ; break; }
		case DepthFunc::LEqual: { pass = z <= bufZ; break; }
		case DepthFunc::GEqual: { pass = z >= bufZ; break; }
		case DepthFunc::Equal: { pass = z == bufZ; break; }
		}
		if (pass) data.push(fragmentPtr);
	}
	return SR::RendererComponent::State::Done;
}

#include "Renderer.h"
#include "Viewport.h"

void SR::RenderTask::Submit()
{
	if (!viewport) viewport = std::make_shared<SR::Viewport>(SR::Viewport::main);
	if (!mesh || !vert || !frag || !frameBuffer || !viewport)
	{
		XLogWarning("Not bind to a valid mesh, vertex shader, fragment shader, viewport or frame buffer.");
		return;
	}
	Renderer::GetInstance().Push(*this);
}

//std::vector<std::unique_ptr< // cache hit要求gl_VertexID和gl_InstanceID 都与cache相等，这里不考虑实例化渲染，只考虑gl_VertexID
SR::RendererComponent::State SR::VertexProcessor::Processing(std::shared_ptr<SR::Mesh>& mesh)
{
	if (!mesh || (begin >= mesh->indices.size())) return SR::RendererComponent::State::Done;
	// If cached, skip this vertex
	int idx = mesh->indices[begin++];
	do
	{
		VertexShaderOutput out;
		if (bCacheEnabled)
		{
			if (cache.Get(idx, out))
			{
				outputs.push(std::make_shared<VertexShaderOutput>(out));
				break;
			}
		}

		VertexShaderInput in;
		if (mesh->IsValidAttribute(VertexAttribute::Positions)) in.position = mesh->vertices[idx];
		if (mesh->IsValidAttribute(VertexAttribute::UVs)) in.uv = mesh->uvs[idx];
		if (mesh->IsValidAttribute(VertexAttribute::Normals)) in.normal = mesh->normals[idx];
		if (mesh->IsValidAttribute(VertexAttribute::Tangents)) in.tangent = mesh->tangents[idx];
		if (mesh->IsValidAttribute(VertexAttribute::Colors)) in.color = mesh->colors[idx];
		out = Dispatch(idx, in);
		outputs.push(std::make_shared<VertexShaderOutput>(out));
		if (bCacheEnabled) cache.Cache(idx, out);
		break;
	} while (false);
	
	return SR::RendererComponent::State::Running;
}

// 前一个drawcall的图元的处理一定先于后一个drawcall的所有图元 https://www.khronos.org/opengl/wiki/Primitive_Assembly
// Assembly, Face Culling
SR::RendererComponent::State SR::PrimitiveAssembler::Assembly(std::queue<std::shared_ptr<VertexShaderOutput>>& data)
{
	if (data.empty()) return SR::RendererComponent::State::Done;
	int vnum = PrimitiveRequiredVertices(mode);
	if (vnum <= 0) SR::RendererComponent::State::Done;

	int size = data.size();
	if (size < vnum) return SR::RendererComponent::State::Waiting;

	auto out = new PrimitiveAssemblyOutput;
	for (int i = vnum; i--;)
	{
		auto v = data.front();
		out->vertices.emplace_back(v);
		data.pop();
	}
	size -= vnum;

	// TODO: Culling

	// Calculate
	bool frontFacing = 1 > 0 ? true : false;
	if (face == FrontFace::CW) frontFacing = !frontFacing;
	bool cull = !frontFacing;
	if (cullFace == Culling::Front) cull = !cull;

	if (!(cullFace != Culling::Off && cull))
	{
		out->gl_FrontFacing = frontFacing;
		out->gl_PrimitiveID = id++;
		outputs.emplace(out);
	}

	return SR::RendererComponent::State::Done;
}

// Clipping, Perspective Divide, Viewport Transform
SR::RendererComponent::State SR::VertexProcessor::PostProcessing(std::queue<std::shared_ptr<PrimitiveAssemblyOutput>>& data)
{
	// Clipping
	int size = data.size();
	while (size--)
	{
		auto primitve = data.front();
		data.pop();
		for (auto& v : primitve->vertices)
		{
			v->gl_Position.w = 1.f / v->gl_Position.w;
			v->gl_Position.x *= v->gl_Position.w;
			v->gl_Position.y *= v->gl_Position.w;
			v->gl_Position.z *= v->gl_Position.w;
			// TODO: Clipping


			viewport->ApplyViewportTransform(v->gl_Position);
			int x = 1;
		}
		data.push(primitve);
	}
	return SR::RendererComponent::State::Done;
}

// Interpolation, Perspective Correction
SR::RendererComponent::State SR::Rasterizer::Rasterizing(std::queue<std::shared_ptr<PrimitiveAssemblyOutput>>& data)
{
	if (primitiveAssemblyMode != PrimitiveAssemblyMode::Triangles) return SR::RendererComponent::State::Done;
	while (!data.empty())
	{
		PrimitiveAssemblyOutput primitve = *data.front();
		data.pop();
		VertexShaderOutput& v0 = *primitve.vertices[0]; // x,y,z in window space, w = linear depth
		VertexShaderOutput& v1 = *primitve.vertices[1];
		VertexShaderOutput& v2 = *primitve.vertices[2];
		Vec2 p0 = v0.gl_Position.Truncated<2>();
		Vec2 p1 = v1.gl_Position.Truncated<2>();
		Vec2 p2 = v2.gl_Position.Truncated<2>();
		Vec3 zv = Vec3(v0.gl_Position.z, v1.gl_Position.z, v2.gl_Position.z);
		Vec3 wv = Vec3(v0.gl_Position.w, v1.gl_Position.w, v2.gl_Position.w);
		float fltMax = sbm::Math::FloatMax;
		Vec2 bboxMin(fltMax, fltMax), bboxMax(-fltMax, -fltMax), clamp(viewport->width, viewport->height);
		for (int i = 0; i < 2; ++i) { // 取ceil保证浮点转整型时样本点在三角形内
			bboxMax[i] = (sbm::min(clamp[i], sbm::max({ p0[i], p1[i],  p2[i] })));
			bboxMin[i] = (sbm::max(0.f, sbm::min({ bboxMax[i], p0[i], p1[i], p2[i] })));
		}

		bool hasCustomedVarying = bool(v0.varying);
		assert(hasCustomedVarying ? (v0.varying->size() == v1.varying->size()) && (v0.varying->size() == v2.varying->size()) : true);
		int size = hasCustomedVarying ? v0.varying->size() : 0;
		for (int i = bboxMin.y; i <= bboxMax.y; ++i)
		{
			for (int j = bboxMin.x; j <= bboxMax.x; ++j)
			{
				Vec2 p(j, i);
				Vec3 lambda3 = barycentric(p0, p1, p2, p);
				if (lambda3[0] < .0f || lambda3[1] < .0f || lambda3[2] < .0f) {
					continue;
				}

				RasterOutput* out = new RasterOutput;

				float w;
				if (interpolationMode == InterpolationMode::Smooth)
				{
					lambda3 *= wv;
					w = lambda3.x + lambda3.y + lambda3.z;
					lambda3 *= 1.f / w;
				}
				else
				{
					w = lambda3.x * wv.x + lambda3.y * wv.y + lambda3.z * wv.z;
				}

				out->gl_FragCoord = Vec4(
					j, i,
					lambda3.x * zv.x + lambda3.y * zv.y + lambda3.z * zv.z,
					w
				);
				out->gl_FragDepth = out->gl_FragCoord.z;
				out->gl_FrontFacing = primitve.gl_FrontFacing;
				out->gl_PrimitiveID = primitve.gl_PrimitiveID;
				if (hasCustomedVarying)
				{
					out->varying.reset(v0.varying->Clone());
					int i = 0;
					while (i < size)
					{
						(*out->varying)[i] = lambda3.x * (*v0.varying)[i] + lambda3.y * (*v1.varying)[i] + lambda3.z * (*v2.varying)[i],
							++i;
					}
				}

				outputs.emplace(out);
			}
		}
	}
	return SR::RendererComponent::State::Done;
}

SR::RendererComponent::State SR::FragmentProcessor::Shading(std::queue<std::shared_ptr<RasterOutput>>& data)
{
	while (!data.empty())
	{
		auto fragPtr = data.front();
		auto& fragment = *fragPtr;
		data.pop();

		std::shared_ptr<FragmentShaderOutput> out;
		out = std::make_shared<FragmentShaderOutput>(Dispatch(fragment));
		outputs.push(out);
	}
	return SR::RendererComponent::State::Done;
}

// 同一个drawcall内需要按primitive id的顺序做blending
SR::RendererComponent::State SR::FragmentProcessor::Blending(std::queue<std::shared_ptr<FragmentShaderOutput>>& data)
{
	auto& colorBuf = frameBuffer->colorBuf;
	auto& depthBuf = frameBuffer->depthBuf;
	while (!data.empty())
	{
		auto pixelPtr = data.front();
		auto& pixel = *pixelPtr;
		data.pop();

		float srcR = pixel.color.r, srcG = pixel.color.g, srcB = pixel.color.b, srcA = pixel.color.a;
		float dstR, dstG, dstB, dstA;
		colorBuf->Get(pixel.gl_FragCoord.x, pixel.gl_FragCoord.y, dstR, dstG, dstB, dstA);
		switch (srcOp)
		{
		case SR::BlendOp::One:
			break;
		case SR::BlendOp::Zero:
			srcR = 0, srcG = 0, srcB = 0, srcA = 0;
			break;
		}
		switch (dstOp)
		{
		case SR::BlendOp::One:
			break;
		case SR::BlendOp::Zero:
			dstR = 0, dstG = 0, dstB = 0, dstA = 0;
			break;
		}
		float r = srcR + dstR, g = srcG + dstG, b = srcB + dstB, a = srcA + dstA;
		colorBuf->Set(pixel.gl_FragCoord.x, pixel.gl_FragCoord.y, r, g, b, a);
		if (depthBuf)
		{
			depthBuf->Set(pixel.gl_FragCoord.x, pixel.gl_FragCoord.y, pixel.gl_FragDepth, pixel.gl_FragDepth, pixel.gl_FragDepth, pixel.gl_FragDepth);
		}
	}
	return SR::RendererComponent::State::Done;
}

void SR::Renderer::RenderAll()
{
	while (!tasks.empty())
	{
		RenderTask task = tasks.front();
		RenderStatus& status = task.status;
		tasks.pop();

		assert(bool(task.vert));

		vertexProcessor.Reset();
		vertexProcessor.viewport = task.viewport;
		vertexProcessor.vert = task.vert;
		vertexProcessor.varying = task.varying;
		vertexProcessor.uniform = std::const_pointer_cast<const Uniform>(task.uniform);
		vertexProcessor.bCacheEnabled = task.status.bPostTransformCache;

		primitiveAssembler.Reset();
		primitiveAssembler.mode = status.primitiveAssemblyMode;
		primitiveAssembler.cullFace = status.cullFace;
		primitiveAssembler.face = status.face;

		rasterizer.Reset();
		rasterizer.primitiveAssemblyMode = status.primitiveAssemblyMode;
		rasterizer.interpolationMode = status.interpolationMode;
		rasterizer.viewport = task.viewport;

		fragmentProcessor.Reset();
		fragmentProcessor.frag = task.frag;
		fragmentProcessor.frameBuffer = task.frameBuffer;
		fragmentProcessor.uniform = vertexProcessor.uniform;
		fragmentProcessor.depthFunc = status.depthFunc;
		fragmentProcessor.srcOp = status.srcOp;
		fragmentProcessor.dstOp = status.dstOp;
		fragmentProcessor.bEarlyDepthTest = status.bEarlyDepthTest;

		//// Vertex Shader Invoked
		//vsDispatcher->shader = task.vert;
		//vsDispatcher->varying = task.varying;
		//vsDispatcher->uniform = std::const_pointer_cast<const Uniform>(task.uniform);
		//assert(bool(task.frag));
		//fsDispatcher->shader = task.frag;
		//fsDispatcher->uniform = vsDispatcher->uniform;
		//fsDispatcher->bEarlyDepthTest = status.bEarlyDepthTest;

		// 只要有一个running就继续，如果全都是done就跳出
		// 如果存在waiting但前面的流程没有一个是running的，证明是饥饿（starving），需要强制跳出
		while (true)
		{
			bool done = true;
			RendererComponent::State state;
			state = vertexProcessor.Processing(task.mesh);
			done = done && (RendererComponent::State::Done == state);

			state = primitiveAssembler.Assembly(vertexProcessor.outputs);
			if (RendererComponent::State::Waiting == state && done)
			{
				XLogWarning("Primitive Assembler starving, force to exit.")
				break;
			}
			done = done && (RendererComponent::State::Done == state);

			state = vertexProcessor.PostProcessing(primitiveAssembler.outputs);
			done = done && (RendererComponent::State::Done == state);

			state = rasterizer.Rasterizing(primitiveAssembler.outputs);
			done = done && (RendererComponent::State::Done == state);

			if (fragmentProcessor.bEarlyDepthTest)
			{
				state = fragmentProcessor.DepthTesting(rasterizer.outputs);
				done = done && (RendererComponent::State::Done == state);
			}
			state = fragmentProcessor.Shading(rasterizer.outputs);
			done = done && (RendererComponent::State::Done == state);

			state = fragmentProcessor.DepthTesting(fragmentProcessor.outputs);
			done = done && (RendererComponent::State::Done == state);

			state = fragmentProcessor.Blending(fragmentProcessor.outputs);
			done = done && (RendererComponent::State::Done == state);

			if (done) break; // All done
		}

		//VertexProcessing(task.mesh, status.bPostTransformCache);

		//// Primitive Assembly, Cull
		//PrimitiveAssembly(status.primitiveAssemblyMode, status.cullFace, status.face);

		//// Clip, Viewport Transform
		//VertexPostProcessing(*(task.viewport));

		//// Rasterize
		//Rasterizing(status.primitiveAssemblyMode, status.interpolationMode, *task.viewport);

		//// Early Test: Depth Test
		//if (status.bEarlyDepthTest)
		//	DepthTesting(status.depthFunc, task.frameBuffer, rasterOutputs);

		//// Fragment Shader Invoked
		//
		//Shading();

		//// Per Fragment Ops: Depth Test
		//DepthTesting(status.depthFunc, task.frameBuffer, fsOutputs);

		//// Per Fragment Ops: Blend
		//Blending(status.srcOp, status.dstOp, task.frameBuffer);
	}
}

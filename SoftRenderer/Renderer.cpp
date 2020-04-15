#include "Renderer.h"
//
//void SR::RenderTask::Submit()
//{
//	if (!mesh || !vert || !frag || !varying || !frameBuffer)
//	{
//		XLogWarning("Not bind to a valid mesh, varying, vertex shader, fragment shader or frame buffer.");
//		return;
//	}
//	if (!viewport) viewport = std::make_shared<SR::Viewport>(SR::Viewport::main);
//	Renderer::GetInstance().Push(*this);
//}
//
////std::vector<std::unique_ptr< // cache hit要求gl_VertexID和gl_InstanceID 都与cache相等，这里不考虑实例化渲染，只考虑gl_VertexID
//void SR::Renderer::VertexProcessing(const std::shared_ptr<SR::Mesh>& mesh, bool cacheEnabled)
//{
//	if (!mesh) return;
//	auto& meshRef = *mesh;
//	auto& indices = meshRef.indices;
//	for (int i = 0, n = indices.size(); i < n; ++i)
//	{
//		// If cached, skip this vertex
//		int idx = indices[i];
//		std::shared_ptr<VertexShaderOutput> out;
//		if (cacheEnabled)
//		{
//			if (cache->Get(idx, out))
//			{
//				vsOutputs.push(out);
//				continue;
//			}
//		}
//
//		VertexShaderInput in;
//		in.gl_VertexID = idx;
//		if (mesh->Attribute(VertexAttribute::Positions)) in.position = mesh->vertices[idx];
//		if (mesh->Attribute(VertexAttribute::UVs)) in.uv = mesh->uvs[idx];
//		if (mesh->Attribute(VertexAttribute::Normals)) in.normal = mesh->normals[idx];
//		if (mesh->Attribute(VertexAttribute::Tangents)) in.tangent = mesh->tangents[idx];
//		if (mesh->Attribute(VertexAttribute::Colors)) in.color = mesh->colors[idx];
//		out = std::make_shared<VertexShaderOutput>(vsDispatcher->Dispatch(i, idx, in));
//		vsOutputs.push(out);
//		if (cacheEnabled) cache->Cache(idx, out);
//	}
//}
//
//// 前一个drawcall的图元的处理一定先于后一个drawcall的所有图元 https://www.khronos.org/opengl/wiki/Primitive_Assembly
//// Assembly, Face Culling
//void SR::Renderer::PrimitiveAssembly(PrimitiveAssemblyMode paMode, Culling cullFace, FrontFace face)
//{
//	if (vsOutputs.empty()) return;
//	int vnum = 0;
//	switch (paMode)
//	{
//	case PrimitiveAssemblyMode::Triangles: { vnum = 3; break; }
//	}
//	if (vnum <= 0) return;
//	int size = vsOutputs.size();
//	int id = 0;
//	while (size >= vnum)
//	{
//		auto out = new PrimitiveAssemblyOutput;
//		for (int i = vnum; i--;)
//		{
//			out->vertices.emplace_back(vsOutputs.front());
//			vsOutputs.pop();
//		}
//
//		// TODO: Culling
//
//		// Calculate
//		bool frontFacing = 1 > 0 ? true : false;
//		if (face == FrontFace::CW) frontFacing = !frontFacing;
//		bool cull = !frontFacing;
//		if (cullFace == Culling::Front) cull = !cull;
//
//		if (cullFace != Culling::Off && cull) continue;
//		out->gl_FrontFacing = frontFacing;
//		out->gl_PrimitiveID = id++;
//		paOutputs.emplace(out);
//	}
//}
//
//// Clipping, Perspective Divide, Viewport Transform
//void SR::Renderer::VertexPostProcessing(const Viewport& viewport)
//{
//	// Clipping
//	int size = paOutputs.size();
//	while (size--)
//	{
//		auto primitve = paOutputs.front();
//		paOutputs.pop();
//		for (auto& v : primitve->vertices)
//		{
//			v->gl_Position.w = 1.f / v->gl_Position.w;
//			v->gl_Position.x *= v->gl_Position.w;
//			v->gl_Position.y *= v->gl_Position.w;
//			v->gl_Position.z *= v->gl_Position.w;
//			// TODO: Clipping
//
//
//			viewport.ApplyViewportTransform(v->gl_Position);
//		}
//		paOutputs.push(primitve);
//	}
//}
//
//// Interpolation, Perspective Correction
//void SR::Renderer::Rasterizing(PrimitiveAssemblyMode primitiveAssemblyMode, InterpolationMode interpolationMode, const SR::Viewport& viewport)
//{
//	if (primitiveAssemblyMode != PrimitiveAssemblyMode::Triangles) return;
//	while (!paOutputs.empty())
//	{
//		PrimitiveAssemblyOutput primitve = *paOutputs.front();
//		paOutputs.pop();
//		VertexShaderOutput& v0 = *primitve.vertices[0]; // x,y,z in window space, w = linear depth
//		VertexShaderOutput& v1 = *primitve.vertices[1];
//		VertexShaderOutput& v2 = *primitve.vertices[2];
//		Vec2 p0 = v0.gl_Position.Truncated<2>();
//		Vec2 p1 = v1.gl_Position.Truncated<2>();
//		Vec2 p2 = v2.gl_Position.Truncated<2>();
//		Vec3 zv = Vec3(v0.gl_Position.z, v1.gl_Position.z, v2.gl_Position.z);
//		Vec3 wv = Vec3(v0.gl_Position.w, v1.gl_Position.w, v2.gl_Position.w);
//		float fltMax = sbm::Math::FloatMax;
//		Vec2 bboxMin(fltMax, fltMax), bboxMax(-fltMax, -fltMax), clamp(viewport.width, viewport.height);
//		for (int i = 0; i < 2; ++i) { // 取ceil保证浮点转整型时样本点在三角形内
//			bboxMax[i] = sbm::ceil(sbm::min(clamp[i], sbm::max({ p0[i], p1[i],  p2[i] }))) - 1;
//			bboxMin[i] = sbm::ceil(sbm::max(0.f, sbm::min({ bboxMax[i], p0[i], p1[i], p2[i] })));
//		}
//
//		assert((v0.varying->size() == v1.varying->size()) && (v0.varying->size() == v2.varying->size()));
//		int size = v0.varying->size();
//		bool b = size > 0;
//		float* begin0 = b ? reinterpret_cast<float*>(v0.varying.get()) : nullptr;
//		float* begin1 = b ? reinterpret_cast<float*>(v1.varying.get()) : nullptr;
//		float* begin2 = b ? reinterpret_cast<float*>(v2.varying.get()) : nullptr;
//		for (int i = bboxMin.y; i <= bboxMax.y; ++i)
//		{
//			for (int j = bboxMin.x; j <= bboxMax.x; ++j)
//			{
//				Vec2 p(j, i);
//				Vec3 lambda3 = barycentric(p0, p1, p2, p);
//				if (lambda3[0] < .0f || lambda3[1] < .0f || lambda3[2] < .0f) {
//					continue;
//				}
//
//				RasterOutput* out = new RasterOutput;
//
//				float w;
//				if (interpolationMode == InterpolationMode::Smooth)
//				{
//					lambda3 *= wv;
//					w = lambda3.x + lambda3.y + lambda3.z;
//					lambda3 *= 1.f / w;
//				}
//				else
//				{
//					w = lambda3.x * wv.x + lambda3.x * wv.y + lambda3.x * wv.z;
//				}
//				out->gl_FragCoord = Vec4(
//					lambda3.x * p0.x + lambda3.x * p1.x + lambda3.x * p2.x,
//					lambda3.x * p0.y + lambda3.x * p1.y + lambda3.x * p2.y,
//					lambda3.x * zv.x + lambda3.x * zv.y + lambda3.x * zv.z,
//					w
//				);
//				out->gl_FragDepth = out->gl_FragCoord.z;
//				out->gl_FrontFacing = primitve.gl_FrontFacing;
//				out->gl_PrimitiveID = primitve.gl_PrimitiveID;
//				out->varying.reset(v0.varying->Clone());
//				float* begin = b ? reinterpret_cast<float*>(out->varying.get()) : nullptr;
//				int i = 0;
//				while (i < size)
//				{
//					*(begin + i) = lambda3.x * (*(begin0 + i)) + lambda3.x * (*(begin0 + i)) + lambda3.x * (*(begin0 + i)),
//						++i;
//				}
//				rasterOutputs.emplace(out);
//			}
//		}
//	}
//}
//
//void SR::Renderer::Shading()
//{
//	while (!rasterOutputs.empty())
//	{
//		auto fragPtr = rasterOutputs.front();
//		auto& fragment = *fragPtr;
//		rasterOutputs.pop();
//
//		std::shared_ptr<FragmentShaderOutput> output;
//		output = std::make_shared<FragmentShaderOutput>(fsDispatcher->Dispatch(fragment));
//		fsOutputs.push(output);
//	}
//}
//
//void SR::Renderer::Blending(BlendOp srcOp, BlendOp dstOp, const std::shared_ptr<FrameBuffer>& fb)
//{
//	while (!fsOutputs.empty())
//	{
//		auto pixelPtr = fsOutputs.front();
//		auto& pixel = *pixelPtr;
//		fsOutputs.pop();
//		float srcR = pixel.color.r, srcG = pixel.color.g, srcB = pixel.color.b, srcA = pixel.color.a;
//		float dstR, dstG, dstB, dstA;
//		fb->colorBuf->Get(pixel.gl_FragCoord.x, pixel.gl_FragCoord.y, dstR, dstG, dstB, dstA);
//		switch (srcOp)
//		{
//		case SR::BlendOp::One:
//			break;
//		case SR::BlendOp::Zero:
//			srcR = 0, srcG = 0, srcB = 0, srcA = 0;
//			break;
//		}
//		switch (dstOp)
//		{
//		case SR::BlendOp::One:
//			break;
//		case SR::BlendOp::Zero:
//			dstR = 0, dstG = 0, dstB = 0, dstA = 0;
//			break;
//		}
//		float r = srcR + dstR, g = srcG + dstG, b = srcB + dstB, a = srcA + dstA;
//		fb->colorBuf->Set(pixel.gl_FragCoord.x, pixel.gl_FragCoord.y, r, g, b, a);
//		if (!fb->depthBuf) return;
//		fb->depthBuf->Set(pixel.gl_FragCoord.x, pixel.gl_FragCoord.y, pixel.gl_FragDepth, pixel.gl_FragDepth, pixel.gl_FragDepth, pixel.gl_FragDepth);
//	}
//}
//
//void SR::Renderer::RenderAll()
//{
//	while (!tasks.empty())
//	{
//		RenderTask task = tasks.front();
//		RenderState& state = task.state;
//		tasks.pop();
//
//		// Vertex Shader Invoked
//		assert(bool(task.vert));
//		vsDispatcher->shader = task.vert;
//		vsDispatcher->varying = task.varying;
//		vsDispatcher->uniform = std::const_pointer_cast<const Uniform>(task.uniform);
//		VertexProcessing(task.mesh, state.bPostTransformCache);
//
//		// Primitive Assembly
//		PrimitiveAssembly(state.primitiveAssemblyMode, state.cullFace, state.face);
//
//		// Clip, Viewport Transform, Cull
//		VertexPostProcessing(*(task.viewport));
//
//		// Rasterize
//		Rasterizing(state.primitiveAssemblyMode, state.interpolationMode, *task.viewport);
//
//		// Early Test: Depth Test
//		if (state.bEarlyDepthTest)
//			DepthTesting(state.depthFunc, task.frameBuffer, rasterOutputs);
//
//		// Fragment Shader Invoked
//		assert(bool(task.frag));
//		fsDispatcher->shader = task.frag;
//		fsDispatcher->uniform = vsDispatcher->uniform;
//		fsDispatcher->bEarlyDepthTest = state.bEarlyDepthTest;
//		Shading();
//
//		// Per Fragment Ops: Depth Test
//		DepthTesting(state.depthFunc, task.frameBuffer, fsOutputs);
//
//		// Per Fragment Ops: Blend
//		Blending(state.srcOp, state.dstOp, task.frameBuffer);
//	}
//}

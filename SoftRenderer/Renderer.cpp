#include "Renderer.h"
#include "Viewport.h"
#include <vector>
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

float SR::PrimitiveAssembler::CalIntersectRatio(const Vec4& ps, const Vec4& pe, Axis axis)
{
	float t = -1;
	if (axis == Axis::W)
	{
		t = (W_CLIPPED_PLANE - ps.w) / (pe.w - ps.w);
	}
	else
	{
		int idx = (int)axis;
		int sign = idx > 0 ? 1 : -1;
		idx = idx * sign - 1;
		float i0 = ps[idx], w0 = ps.w, di = pe[idx] - ps[idx], dw = pe.w - ps.w;
		t = (w0 - sign * i0) / (sign * di - dw);
	}
	return t;
}

std::vector<std::shared_ptr<SR::VertexShaderOutput>> SR::PrimitiveAssembler::ClippingAgainstPlane(const std::vector<std::shared_ptr<SR::VertexShaderOutput>>& polygon, Axis axis)
{
	std::vector<std::shared_ptr<SR::VertexShaderOutput>> out;
	for (int i = 0, n = polygon.size(); i < n; ++i)
	{
		auto& ps = *polygon[i%n];
		auto& pe = *polygon[(i+1)%n];
		bool inside0 = IsInside(ps.gl_Position, axis);
		bool insidet = IsInside(pe.gl_Position, axis);
		if (insidet != inside0)
		{
			float t = CalIntersectRatio(ps.gl_Position, pe.gl_Position, axis);

			SR::VertexShaderOutput p;
			p.gl_VertexID = insidet ? ps.gl_VertexID : pe.gl_VertexID;
			p.gl_Position = ps.gl_Position + (pe.gl_Position - ps.gl_Position) * t;
			int size = bool(ps.varying) ? ps.varying->size() : 0;
			int i = 0;
			if(size > 0) p.varying.reset(ps.varying->Clone());
			while (i < size)
			{
				(*p.varying)[i] = (*ps.varying)[i] + ((*pe.varying)[i] - (*ps.varying)[i]) * t;
				++i;
			}
			out.emplace_back(std::make_shared<SR::VertexShaderOutput>(p));
		}
		if (insidet)
		{
			out.emplace_back(std::make_shared<SR::VertexShaderOutput>(pe));
		}
	}
	return out;
}


std::vector<std::shared_ptr<SR::VertexShaderOutput>> SR::PrimitiveAssembler::HomogeneousClipping(const std::vector<std::shared_ptr<VertexShaderOutput>>& polygon)
{
	return polygon;
	bool bNeedClipped = false;
	for (auto& v : polygon)
	{
		bNeedClipped |= !IsVisible(v->gl_Position);
		if (bNeedClipped) break;
	}

	if (!bNeedClipped) return polygon;
	int i = 0;
	std::vector<std::shared_ptr<VertexShaderOutput>> pingpong[2] = { polygon };
	const Axis plane[] = { W,POSITIVE_X,NEGATIVE_X,POSITIVE_Y,NEGATIVE_Y,POSITIVE_Z,NEGATIVE_Z }; // w,x,-x,y,-y,z,-z
	for (auto k : plane)
	{
		pingpong[(i + 1) % 2] = std::move(ClippingAgainstPlane(pingpong[i], k));
		i = (i + 1) % 2;
	}
	return pingpong[i];
}

// 前一个drawcall的图元的处理一定先于后一个drawcall的所有图元 https://www.khronos.org/opengl/wiki/Primitive_Assembly
// Perspective Divide, Assembly, Clipping, Viewport Transform, Culling
// 1.获得3个裁剪空间的顶点的信息，2.裁剪获得新的多边形并生成新顶点的信息，3.三角化，生成新图元，4.透视除法，5.cvv剔除，6.视口变换，7.背面剔除
SR::RendererComponent::State SR::PrimitiveAssembler::Assembly(std::queue<std::shared_ptr<VertexShaderOutput>>& data)
{
	assert(mode == PrimitiveAssemblyMode::Triangles);
	if (data.empty()) return SR::RendererComponent::State::Done;
	int vnum = PrimitiveRequiredVertices(mode);
	if (vnum <= 0) SR::RendererComponent::State::Done;

	int size = data.size();
	if (size < vnum) return SR::RendererComponent::State::Waiting;

	std::vector<std::shared_ptr<VertexShaderOutput>> vertices;
	vertices.reserve(3);
	for (int i = vnum; i--;)
	{
		auto v = data.front();
		vertices.emplace_back(v);
		data.pop();
	}
	size -= vnum;

	// TODO: Clipping
	auto clippedVertices = HomogeneousClipping(vertices);

	// Assembly
	for (int i = 0, n = clippedVertices.size(); i < n - 2; ++i)
	{
		auto out = new PrimitiveAssemblyOutput;
		out->vertices.reserve(3);
		out->vertices.push_back(clippedVertices[0]);
		out->vertices.push_back(clippedVertices[i + 1]);
		out->vertices.push_back(clippedVertices[i + 2]);
		
		// Perspective Division
		for (auto& v : out->vertices)
		{
			v->gl_Position.w = 1.f / v->gl_Position.w;
			v->gl_Position.x *= v->gl_Position.w;
			v->gl_Position.y *= v->gl_Position.w;
			v->gl_Position.z *= v->gl_Position.w;
			viewport->ApplyViewportTransform(v->gl_Position);
		}
		
		// Culling
		float a = 0;
		for (int k = 0, m = out->vertices.size(); k < m; ++k)
		{
			a = out->vertices[k]->gl_Position.x * out->vertices[(k+1)%m]->gl_Position.y - out->vertices[(k + 1) % m]->gl_Position.x * out->vertices[k]->gl_Position.y;
		}
		bool frontFacing = a > 0;
		if (viewport->bTopDown) frontFacing = !frontFacing; // window屏幕坐标y轴反了，需要翻转一次
		if (face == FrontFace::CW) frontFacing = !frontFacing;

		bool cull = !frontFacing;
		if (cullFace == Culling::Front) cull = !cull;
		if (!(cullFace != Culling::Off && cull))
		{
			out->gl_FrontFacing = frontFacing;
			out->gl_PrimitiveID = id++;
			outputs.emplace(out);
		}
	}

	return SR::RendererComponent::State::Done;
}

// Perspective Divide, Clipping, Viewport Transform, Culling
//SR::RendererComponent::State SR::VertexProcessor::PostProcessing(std::queue<std::shared_ptr<PrimitiveAssemblyOutput>>& data)
//{
//	// Clipping
//	int size = data.size();
//	while (size--)
//	{
//		auto primitve = data.front();
//		data.pop();
//		for (auto& v : primitve->vertices)
//		{
//			// Perspective Divide
//			v->gl_Position.w = 1.f / v->gl_Position.w;
//			v->gl_Position.x *= v->gl_Position.w;
//			v->gl_Position.y *= v->gl_Position.w;
//			v->gl_Position.z *= v->gl_Position.w;
//			// TODO: Clipping
//
//
//			// Viewport Transform
//			viewport->ApplyViewportTransform(v->gl_Position);
//
//			int x = 1;
//		}
//		data.push(primitve);
//	}
//	return SR::RendererComponent::State::Done;
//}

void SR::Rasterizer::RasterizeFilledTriangle(const SR::PrimitiveAssemblyOutput& primitive)
{

	VertexShaderOutput& v0 = *primitive.vertices[0]; // x,y,z in window space, w = linear depth
	VertexShaderOutput& v1 = *primitive.vertices[1];
	VertexShaderOutput& v2 = *primitive.vertices[2];
	Vec2 p0 = v0.gl_Position.Truncated<2>();
	Vec2 p1 = v1.gl_Position.Truncated<2>();
	Vec2 p2 = v2.gl_Position.Truncated<2>();
	Vec3 zv = Vec3(v0.gl_Position.z, v1.gl_Position.z, v2.gl_Position.z);
	Vec3 wv = Vec3(v0.gl_Position.w, v1.gl_Position.w, v2.gl_Position.w);
	Vec2 edge[3] = { p2 - p1 ,p0 - p2,p1 - p0 };
	float fltMax = sbm::Math::FloatMax;
	Vec2 bboxMin(fltMax, fltMax), bboxMax(-fltMax, -fltMax), clamp(viewport->width, viewport->height);
	for (int i = 0; i < 2; ++i) { // 取ceil保证浮点转整型时样本点在三角形内
		bboxMax[i] = sbm::ceil(sbm::min(clamp[i], sbm::max({ p0[i], p1[i],  p2[i] })) - 1);
		bboxMin[i] = sbm::ceil(sbm::max(0.f, sbm::min({ bboxMax[i], p0[i], p1[i], p2[i] })));
	}
	bool hasCustomedVarying = bool(v0.varying);
	assert(hasCustomedVarying ? (v0.varying->size() == v1.varying->size()) && (v0.varying->size() == v2.varying->size()) : true);
	int size = hasCustomedVarying ? v0.varying->size() : 0;
	for (int i = bboxMin.y; i <= bboxMax.y; ++i)
	{
		for (int j = bboxMin.x; j <= bboxMax.x; ++j)
		{
			Vec2 p(j + 0.5f, i + 0.5f);
			Vec3 lambda3 = barycentric(p0, p1, p2, p);
			bool overlap = true;
			for (int i = 0; i < 3; ++i)
			{
				// Top-Left Rule
				overlap &= (lambda3[i] == 0) ? ((edge[i].x < 0 && edge[i].y == 0) || edge[i].y < 0) : (lambda3[i] > 0);
			}
			if (!overlap) continue;
			/*if (rasterizationMode == RasterizationMode::Line)
			{
				if (lambda3[0] > .01f && lambda3[1] > .01f && lambda3[2] > .01f)
				{
					continue;
				}
			}*/
			

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
			out->gl_FrontFacing = primitive.gl_FrontFacing;
			out->gl_PrimitiveID = primitive.gl_PrimitiveID;
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

void SR::Rasterizer::RasterizeWireTriangle(const SR::PrimitiveAssemblyOutput& primitive)
{
	VertexShaderOutput& v0 = *primitive.vertices[0]; // x,y,z in window space, w = linear depth
	VertexShaderOutput& v1 = *primitive.vertices[1];
	VertexShaderOutput& v2 = *primitive.vertices[2];

	RasterizeLine(v0.gl_Position, v1.gl_Position, primitive.gl_FrontFacing, primitive.gl_PrimitiveID);
	RasterizeLine(v1.gl_Position, v2.gl_Position, primitive.gl_FrontFacing, primitive.gl_PrimitiveID);
	RasterizeLine(v2.gl_Position, v0.gl_Position, primitive.gl_FrontFacing, primitive.gl_PrimitiveID);
}

void SR::Rasterizer::RasterizeLine(const Vec4& v0, const Vec4& v1, bool frontFacing, int primitiveID)
{
	int x0 = v0.x;
	int x1 = v1.x;
	int y0 = v0.y;
	int y1 = v1.y;
	float z0 = v0.z;
	float z1 = v1.z;
	float w0 = v0.w;
	float w1 = v1.w;
	bool steep = false;
	// 交换XY轴，保证只讨论X Major
	if (sbm::abs(x1 - x0) < sbm::abs(y1 - y0))
	{
		// 转置
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}

	// 保证只需处理x0递增至x1
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
		std::swap(z0, z1);
		std::swap(w0, w1);
	}

	std::vector<SR::RasterOutput*> outs(x1 - x0);

	// (1) e>=0.5 => f(e)=dx*(2*e-1)>=0
	// (2) e=0 => f(0)=dx*(2*0-1)=dx
	// (3) e-=1 => f(e-1)=dx*[2*(e-1)-1]=dx*[2*e-1]-2*dx=f(e)-2*dx
	// (4) e+=k => e+=dy/dx => f(e+k)=dx*[2*(e+k)-1]=dx*[2*e-1]+2*dx*(dy/dx)=f(e)+2*dy
	int dx = x1 - x0, dy = y1 - y0;
	int err = -dx;
	int derr = sbm::abs(2 * dy); // 这个绝对值保证将讨论限定于第一象限
	int inc = y1 > y0 ? 1 : -1;
	float s, t;
	if (steep)
	{
		for (int x = x0, y = y0; x <= x1; ++x)
		{
			// 转置
			RasterOutput* out = new RasterOutput;
			out->gl_FrontFacing = frontFacing;
			out->gl_PrimitiveID = primitiveID;
			t = dx == 0 ? 0 : (x - x0) / dx; // t == 1, x == x0
			s = w0 * (1 - t);
			t *= w1;
			float w = (s + t);
			float z = 1.f / w;
			s *= z;
			t *= z;
			out->gl_FragCoord.x = y;
			out->gl_FragCoord.y = x;
			out->gl_FragDepth = out->gl_FragCoord.z = z0 * s + z1 * t;
			out->gl_FragCoord.w = w;
			outputs.emplace(out);
			//image.set(y, x, color);
			err += derr;
			//if (err >= 0)
			if (err > 0)
			{
				y += inc;
				err -= 2 * dx;
			}
		}
	}
	else
	{
		for (int x = x0, y = y0; x <= x1; ++x)
		{
			RasterOutput* out = new RasterOutput;
			out->gl_FrontFacing = frontFacing;
			out->gl_PrimitiveID = primitiveID;
			t = dx == 0 ? 0 : (x - x0) / dx; // t == 1, x == x0
			s = w0 * (1 - t);
			t *= w1;
			float w = (s + t);
			float z = 1.f / w;
			s *= z;
			t *= z;
			out->gl_FragCoord.x = x;
			out->gl_FragCoord.y = y;
			out->gl_FragDepth = out->gl_FragCoord.z = z0 * s + z1 * t;
			out->gl_FragCoord.w = w;
			outputs.emplace(out);
			//image.set(x, y, color);
			err += derr;
			//if (err >= 0)
			if (err > 0)
			{
				y += inc;
				err -= 2 * dx;
			}
		}
	}
}

// Interpolation, Perspective Correction
SR::RendererComponent::State SR::Rasterizer::Rasterizing(std::queue<std::shared_ptr<PrimitiveAssemblyOutput>>& data, SR::RasterizationMode mode)
{
	if (primitiveAssemblyMode != PrimitiveAssemblyMode::Triangles) return SR::RendererComponent::State::Done;
	while (!data.empty())
	{
		PrimitiveAssemblyOutput primitive = *data.front();
		data.pop();

		if (SR::RasterizationMode::Line == mode)
		{
			RasterizeWireTriangle(primitive);
		}
		else
		{
			RasterizeFilledTriangle(primitive);
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
			depthBuf->Set((pixel.gl_FragCoord.x), (pixel.gl_FragCoord.y), pixel.gl_FragDepth);
		}
	}
	return SR::RendererComponent::State::Done;
}

void SR::Renderer::RenderAll()
{
	//std::queue<SR::RenderTask> tmp;
	//do
	//{
		while (!tasks.empty())
		{
			RenderTask task = tasks.front();
			RenderStatus& status = task.status;
			tasks.pop();
			/*if (status.rasterizationMode == RasterizationMode::ShadedWireframe)
			{
				auto prevDF = status.depthFunc;
				auto lineTask = task;

				status.rasterizationMode = RasterizationMode::Line;
				status.depthFunc = DepthFunc::LEqual;
				lineTask.status = status;
				tmp.push(lineTask);

				status.rasterizationMode = RasterizationMode::Filled;
				status.depthFunc = prevDF;
			}*/

			assert(bool(task.vert));
			bool drawLine = (status.rasterizationMode == RasterizationMode::Line);
			if (drawLine)
			{
				rasterizer.defaultWireframeUniform->mat_ObjectToClip = task.uniform->mat_ObjectToClip;
				vertexProcessor.uniform = (rasterizer.defaultWireframeUniform);
			}
			else
			{
				vertexProcessor.uniform = std::const_pointer_cast<const Uniform>(task.uniform);
			}
			vertexProcessor.Reset();
			//vertexProcessor.viewport = task.viewport;
			vertexProcessor.vert = drawLine ? rasterizer.defaultWireframeVert : task.vert;
			vertexProcessor.varying = drawLine ? std::shared_ptr<Varying>()  : task.varying;
			vertexProcessor.bCacheEnabled = task.status.bPostTransformCache;

			primitiveAssembler.Reset();
			primitiveAssembler.mode = status.primitiveAssemblyMode;
			primitiveAssembler.cullFace = status.cullFace;
			primitiveAssembler.face = status.face;
			primitiveAssembler.viewport = task.viewport;

			rasterizer.Reset();
			rasterizer.primitiveAssemblyMode = status.primitiveAssemblyMode;
			rasterizer.interpolationMode = status.interpolationMode;
			rasterizer.rasterizationMode = status.rasterizationMode;
			rasterizer.viewport = task.viewport;

			fragmentProcessor.Reset();
			fragmentProcessor.frag = drawLine ? rasterizer.defaultWireframeFrag : task.frag; // ;
			fragmentProcessor.frameBuffer = task.frameBuffer;
			fragmentProcessor.uniform = vertexProcessor.uniform;
			fragmentProcessor.depthFunc = status.depthFunc;
			fragmentProcessor.srcOp = status.srcOp;
			fragmentProcessor.dstOp = status.dstOp;
			fragmentProcessor.bEarlyDepthTest = status.bEarlyDepthTest;

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

				//state = vertexProcessor.PostProcessing(primitiveAssembler.outputs);
				//done = done && (RendererComponent::State::Done == state);

				state = rasterizer.Rasterizing(primitiveAssembler.outputs, status.rasterizationMode);
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
		}

		/*tmp.swap(tasks);
		std::queue<SR::RenderTask> tmp2;
		tmp.swap(tmp2);*/
	//}while (!tasks.empty());
}

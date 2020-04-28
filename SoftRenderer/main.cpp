#include <tchar.h>
#include <iostream>
#include <string>
#include <memory>

#include "Math/Vector.h"
#include "Math/Matrix.h"
#include "Math/Matrix4x4.h"
#include "Math/Utility.h"
#include "Time.h"
#include "Mesh.h"
#include "MeshLoader.h"
#include "TextureLoader.h"
#include "Math/Math.h"
#include "Color.h"
#include "Texture.h"
#include "FrameBuffer.h"
#include "Log.h"
#include "Camera.h"
#include "Window.h"
#include "Renderer.h"
#include "CommonShader.h"
#include "BlinnPhongShader.h"
#include "Header.h"

static int gWidth = 800;
static int gHeight = 600;

Mat4 GetViewportMatrix(int x, int y, int width, int height)
{
	Mat4 viewport = Mat4::Identity;
	float halfW = width * 0.5f, halfH = height * .5f, halfDepth = 255 * .5f;
	viewport.M(0, 0) = halfW; viewport.M(0, 3) = halfW + x;
	// 因为windows buffer设置成了top-down，这也符合对buffer起始位置的直观理解，所以要想避免绘制后垂直翻转，就需要对视口进行翻转，所以这里y坐标系数需要乘以-1
	viewport.M(1, 1) = -halfH ; viewport.M(1, 3) = halfH + y; 
	viewport.M(2, 2) = 1; viewport.M(2, 3) = 0;
	return viewport;
}

Mat4 GetLookAtMatrix(const Vec3& eye, const Vec3& center, const Vec3& up)
{
	Mat4 view = Mat4::Identity;
	Vec3 front = center - eye;
	Vec3 right = Cross(front, up).Normalized();
	Vec3 upNorm = Cross(right, front).Normalized();
	front.Normalize();
	view.SetRow(0, right.x, right.y, right.z, -Dot(right, eye));
	view.SetRow(1, upNorm.x, upNorm.y, upNorm.z, -Dot(upNorm, eye));
	view.SetRow(2, front.x, front.y, front.z, -Dot(front, eye));
	view.SetRow(3, 0, 0, 0, 1.f);
	return view;
}

void TestDraw(SR::Texture& image, SR::Texture& zBuf,
	Vec3 v0, Vec3 v1, Vec3 v2,
	Vec2 uv0, Vec2 uv1, Vec2 uv2,
	Vec3 n0, Vec3 n1, Vec3 n2,
	SR::Color color
);

void DrawFilledTrianxgleBarycentricCoordinate_Texture_TestPerspectiveCorrection(SR::Texture& image, SR::Texture& tex, SR::Texture& normalMap, SR::Texture& specMap, SR::Texture& zBuf,
	Vec3 v0, Vec3 v1, Vec3 v2,
	Vec2 uv0, Vec2 uv1, Vec2 uv2,
	Vec3 n0, Vec3 n1, Vec3 n2
);

Mat4 gViewportMat;
Mat4 gViewMat;
Mat4 gProjMat;
Vec3 gCameraPos;

void Test();
int main()
{
	// Camera
	auto& mainCam = *SR::Camera::mainCamera;
	mainCam.nearClip = 0.01f;
	mainCam.farClip = 7;
	mainCam.fov = 75;
	mainCam.viewport = SR::Viewport::main;
	mainCam.RegisterInputListener();

	gViewportMat = mainCam.ViewportTransform();
	mainCam.position = Vec3(2.25f,0,5);
	gCameraPos = mainCam.position;
	mainCam.LookAt(Vec3(2.25f, 0, 0));
	// Resources
	SR::TGALoader tgaLoader;
	std::shared_ptr<SR::Mesh> tri(SR::MeshLoader::GetInstance().Load("Resources/tri.obj"));
	std::shared_ptr<SR::Mesh> box(SR::MeshLoader::GetInstance().Load("Resources/box.obj", true));
	std::shared_ptr<SR::Mesh> sphere(SR::MeshLoader::GetInstance().Load("Resources/sphere.obj", true));
	std::shared_ptr<SR::Mesh> floor(SR::MeshLoader::GetInstance().Load("Resources/floor/floor.obj", true));
	std::shared_ptr<SR::Texture> floorDiffuse(tgaLoader.Load("Resources/floor/floor_diffuse.tga"));// 
	
	std::shared_ptr<SR::Mesh> africanHead(SR::MeshLoader::GetInstance().Load("Resources/african_head/african_head.obj", true));
	std::shared_ptr<SR::Texture> africanHeadDiffuse(tgaLoader.Load("Resources/african_head/african_head_diffuse.tga"));// 
	std::shared_ptr<SR::Texture> africanHeadNormal(tgaLoader.Load("Resources/african_head/african_head_nm_tangent.tga"));// 
	std::shared_ptr<SR::Texture> africanHeadSpec(tgaLoader.Load("Resources/african_head/african_head_spec.tga"));// 
	
	//std::shared_ptr<SR::Mesh> bianka_s(SR::MeshLoader::GetInstance().Load("Resources/bianka_s.obj", true));
	
	//std::shared_ptr<SR::Mesh> africanHeadEyeInner(SR::MeshLoader::GetInstance().Load("Resources/african_head/african_head_eye_inner.obj", true));
	//std::shared_ptr<SR::Texture> africanHeadEyeInnerDiffuse(tgaLoader.Load("Resources/african_head/african_head_eye_inner_diffuse.tga"));// 
	//std::shared_ptr<SR::Texture> africanHeadEyeInnerNormal(tgaLoader.Load("Resources/african_head/african_head_eye_inner_nm_tangent.tga"));// 
	//std::shared_ptr<SR::Texture> africanHeadEyeInnerSpec(tgaLoader.Load("Resources/african_head/african_head_eye_inner_spec.tga"));// 
	//
	//std::shared_ptr<SR::Mesh> africanHeadEyeOuter(SR::MeshLoader::GetInstance().Load("Resources/african_head/african_head_eye_outer.obj", true));
	//std::shared_ptr<SR::Texture> africanHeadEyeOuterDiffuse(tgaLoader.Load("Resources/african_head/african_head_eye_outer_diffuse.tga"));// 
	//std::shared_ptr<SR::Texture> africanHeadEyeOuterNormal(tgaLoader.Load("Resources/african_head/african_head_eye_outer_nm_tangent.tga"));// 
	//std::shared_ptr<SR::Texture> africanHeadEyeOuterSpec(tgaLoader.Load("Resources/african_head/african_head_eye_outer_spec.tga"));// 

	// Window
	auto& wnd = SR::Window::GetInstance();
	wnd.Init(SR::Viewport::main.width, SR::Viewport::main.height, SR::Viewport::main.bTopDown);
	auto& backBuf = wnd.GetBackBuffer();
	if (backBuf)
	{
		if (backBuf->colorBuf) backBuf->colorBuf->Clear(SR::Color32::grey.r, SR::Color32::grey.g, SR::Color32::grey.b, SR::Color32::grey.a);
		if (backBuf->depthBuf) backBuf->depthBuf->Clear(SR::Color::white.r, 0.f,0.f,0.f);
	}
	

	Mat4 modelTRS = Mat4::TRS(Vec3(0,0,0), Vec3(0, 0, 0), Vec3(1.f));
	//Mat4 modelTRS1 = Mat4::TRS(Vec3(0,0,0), Vec3(0, 0, 0), Vec3(.5f,.5f,.5f));
	Mat4 modelTRTInv = modelTRS.GetInversed();
	// Init Render Tasks
	
	SR::CommonVert commonVert;
	SR::CommonFrag commonFrag;
	SR::CommonVarying commonVarying;
	SR::CommonUniform commonUniform;
	commonUniform.worldLightDir = Vec3(-1);
	commonUniform.mat_ObjectToWorld = modelTRS;
	commonUniform.mat_WorldToObject = modelTRTInv;
	//SR::RenderTask drawFloor;
	//uniform.albedo = floorDiffuse;
	//drawFloor.frameBuffer = backBuf;
	//drawFloor.Bind(&vert, &varying);
	//drawFloor.Bind(&frag);
	//drawFloor.Bind(floor.get());
	//drawFloor.status.rasterizationMode = SR::RasterizationMode::Line;

	//SR::BlinnPhongVert blinnVert;
	//SR::BlinnPhongFrag blinnFrag;
	//SR::BlinnPhongVarying blinnVarying;
	//SR::BlinnPhongUniform blinnUniform;
	//blinnUniform.albedo = africanHeadDiffuse;
	//blinnUniform.normal = africanHeadNormal;
	//blinnUniform.spec = africanHeadSpec;
	//blinnUniform.worldLightDir = Vec3(-1);
	//blinnUniform.mat_ObjectToWorld = modelTRS;

	SR::RenderStatus status;
	status.depthFunc = SR::DepthFunc::Less;
	status.bEarlyDepthTest = true;
	status.rasterizationMode = SR::RasterizationMode::Line;
	status.cullFace = SR::Culling::Back;
	SR::RenderTask task;
	task.status = status;
	task.frameBuffer = backBuf;
	SR::RenderTask task1;
	task1.status = status;
	task1.frameBuffer = backBuf;
	/*task.Bind(&blinnVert, &blinnVarying);
	task.Bind(&blinnFrag);
	task.Bind(africanHead.get());*/

	task.Bind(&commonVert, &commonVarying);
	task.Bind(&commonFrag);
	//task.Bind(africanHead.get());

	task1.Bind(&commonVert, &commonVarying);
	task1.Bind(&commonFrag);
	task1.Bind(africanHead.get());

	// Time
	SR::Time::Init();
	float lastPrintTime = SR::Time::TimeSinceStartup();

	while (!wnd.ShouldExit()) {
		SR::Time::Update();		
		float deltaTime = SR::Time::DeltaTime();

		SR::Input::Update(deltaTime);
		
		if (backBuf)
		{
			if (backBuf->colorBuf) backBuf->colorBuf->Clear(SR::Color32::grey.r, SR::Color32::grey.g, SR::Color32::grey.b, SR::Color32::grey.a);
			if (backBuf->depthBuf) backBuf->depthBuf->Clear(SR::Color::white.r, 0.f, 0.f, 0.f);
		}

		if (SR::Input::GetKeyDown(SR::KeyCode::Alpha1))
		{
			task1.status.rasterizationMode = task.status.rasterizationMode = SR::RasterizationMode::Filled;
		}		
		else if (SR::Input::GetKeyDown(SR::KeyCode::Alpha2))
		{
			task1.status.rasterizationMode = task.status.rasterizationMode = SR::RasterizationMode::Line;
		}
		
		//blinnUniform.worldCamPos = mainCam.position;
		//blinnUniform.mat_ObjectToClip = mainCam.ProjectionMatrix() * mainCam.ViewMatrix();
		commonUniform.worldCamPos = mainCam.position;
		commonUniform.mat_ObjectToClip = mainCam.ProjectionMatrix() * mainCam.ViewMatrix() * modelTRS;

		/*task.Bind(&commonUniform);
		task.Submit();*/

		task1.Bind(&commonUniform);
		task1.Submit();

		SR::Renderer::GetInstance().RenderAll();

		wnd.Update(deltaTime);
		float now = SR::Time::TimeSinceStartup();
		float fps = SR::Time::FPS();
		if ((now - lastPrintTime) > 1)
		{
			std::cout << "FPS:" << fps << std::endl;
			lastPrintTime = now;
		}
		wnd.Dispatch();
	}
	wnd.Destroy();

	return 0;
}

void Test()
{
	//TestDraw(*backBuf->colorBuf, *backBuf->depthBuf,
	//	//Vec3(-.5f, -.5f, .5f), Vec3(.5f, -.5f, -.5f), Vec3(0, .5f, 0),
	//	//Vec2(0, 0), Vec2(1, 0), Vec2(.5, 1),
	//	//Vec3(1, 0, 1), Vec3(1, 0, 1), Vec3(1, 0, 1),
	//	Vec3(-.5f, -.5f, .0f), Vec3(.5f, -.5f, .0f), Vec3(-.5f, .5f, 0),
	//	Vec2(0, 0), Vec2(1, 0), Vec2(.5, 1),
	//	Vec3(0, 0, 1), Vec3(0, 0, 1), Vec3(0, 0, 1),
	//	SR::Color::white
	//);
	//{
	//	auto& f = africanHead->indices;
	//	//for (auto f : africanHead->faces)
	//	for (int i = 0, n = africanHead->indices.size(); i < n; i += 3)
	//	{
	//		/*auto v0 = africanHead->vertices[f[0][0]];
	//		auto v1 = africanHead->vertices[f[1][0]];
	//		auto v2 = africanHead->vertices[f[2][0]];

	//		auto uv0 = africanHead->uv[f[0][1]];
	//		auto uv1 = africanHead->uv[f[1][1]];
	//		auto uv2 = africanHead->uv[f[2][1]];

	//		auto n0 = africanHead->normals[f[0][2]];
	//		auto n1 = africanHead->normals[f[1][2]];
	//		auto n2 = africanHead->normals[f[2][2]];*/
	//		auto id0 = f[i + 0], id1 = f[i + 1], id2 = f[i + 2];
	//		auto v0 = africanHead->vertices[id0];
	//		auto v1 = africanHead->vertices[id1];
	//		auto v2 = africanHead->vertices[id2];
	//		
	//		auto uv0 = africanHead->uvs[id0];
	//		auto uv1 = africanHead->uvs[id1];
	//		auto uv2 = africanHead->uvs[id2];

	//		auto n0 = africanHead->normals[id0];
	//		auto n1 = africanHead->normals[id1];
	//		auto n2 = africanHead->normals[id2];
	//		DrawFilledTrianxgleBarycentricCoordinate_Texture_TestPerspectiveCorrection(*(backBuf->colorBuf), *africanHeadDiffuse, *africanHeadNormal, *africanHeadSpec, *(backBuf->depthBuf), v0, v1, v2, uv0, uv1, uv2, n0, n1, n2);
	//	}
	//}
}

void DrawFilledTrianxgleBarycentricCoordinate_Texture_TestPerspectiveCorrection(SR::Texture& image, SR::Texture& tex, SR::Texture& normalMap, SR::Texture& specMap, SR::Texture& zBuf,
	Vec3 v0, Vec3 v1, Vec3 v2,
	Vec2 uv0, Vec2 uv1, Vec2 uv2,
	Vec3 n0, Vec3 n1, Vec3 n2
)
{
	// 光照
	int screenW = gWidth, screenH = gHeight;
	static Vec3 lightDir(-1, -1, -1);
	lightDir.Normalize();
	int width = image.GetWidth() - 1;
	int height = image.GetHeight() - 1;
	int texWidth = tex.GetWidth() - 1;
	int texHeight = tex.GetHeight() - 1;
	int normalWidth = normalMap.GetWidth() - 1;
	int normalHeight = normalMap.GetHeight() - 1;
	int specMapW = specMap.GetWidth() - 1;
	int specMapH = specMap.GetHeight() - 1;
	float fltMax = sbm::Math::FloatMax;
	Vec2 bboxMin(fltMax, fltMax), bboxMax(-fltMax, -fltMax), clamp(width, height);

	static Vec3 camera(1, 1, 3), center(0, 0, 0), up(0, 1, 0), trans(0, 0, 0), rot(0, 0, 0), scale(1, 1, 1);
	/*Mat4 proj = Mat4::Identity;
	proj.M(3, 2) = 1 / camera.z;
	Mat4 view = GetLookAtMatrix(camera, center, up);
	static Mat4 viewport = GetViewportMatrix(0, 0, screenW, screenH);
*/
	Mat4 trs = Mat4::TRS(trans, rot, scale);
	Mat4 view = gViewMat;
	Mat4 proj = gProjMat;
	Mat4 viewport = gViewportMat;
	Mat4 MVP = proj * view * trs;// proj ;

	Vec4 screenV0(v0.x, v0.y, v0.z, 1);
	Vec4 screenV1(v1.x, v1.y, v1.z, 1);
	Vec4 screenV2(v2.x, v2.y, v2.z, 1);
	Vec4 worldPos0(screenV0), worldPos1(screenV1), worldPos2(screenV2);

	worldPos0 = trs * worldPos0;
	worldPos1 = trs * worldPos1;
	worldPos2 = trs * worldPos2;

	screenV0 = MVP * screenV0;
	float screenw0 = screenV0.w;
	screenV0 = viewport * (screenV0 / screenV0.w);

	screenV1 = MVP * screenV1;
	float screenw1 = screenV1.w;
	screenV1 = viewport * (screenV1 / screenV1.w);

	screenV2 = MVP * screenV2;
	float screenw2 = screenV2.w;
	screenV2 = viewport * (screenV2 / screenV2.w);

	sbm::Matrix<2, 2> uvMat({ uv1.x - uv0.x, uv2.x - uv0.x,uv1.y - uv0.y, uv2.y - uv0.y });
	sbm::Matrix<2, 3> worldPosMat({
		worldPos1.x - worldPos0.x, worldPos2.x - worldPos0.x,
		worldPos1.y - worldPos0.y, worldPos2.y - worldPos0.y,
		worldPos1.z - worldPos0.z, worldPos2.z - worldPos0.z,
		});

	Mat4 trsInvT = trs.GetInversed().GetTransposed();
	auto worldNormal0 = (trsInvT * n0.Expanded<4>(0));
	auto worldNormal1 = (trsInvT * n1.Expanded<4>(0));
	auto worldNormal2 = (trsInvT * n2.Expanded<4>(0));

	auto tbMat = uvMat.GetInversed() * worldPosMat;
	auto tangent = tbMat.GetRow(0).Expanded<4>(0);
	auto bitangent = tbMat.GetRow(1).Expanded<4>(0);
	auto normal = Cross(bitangent, tangent);
	float w0 = Dot(normal, worldNormal0) < 0 ? -1 : 1;
	float w1 = Dot(normal, worldNormal1) < 0 ? -1 : 1;
	float w2 = Dot(normal, worldNormal2) < 0 ? -1 : 1;
	tangent.Normalize();
	worldNormal0.Normalize();	worldNormal1.Normalize();	worldNormal2.Normalize();
	auto worldTangent0 = w0 * (tangent - Dot(tangent, worldNormal0) * worldNormal0).Normalized();
	auto worldTangent1 = w1 * (tangent - Dot(tangent, worldNormal1) * worldNormal1).Normalized();
	auto worldTangent2 = w2 * (tangent - Dot(tangent, worldNormal2) * worldNormal2).Normalized();
	auto worldBitangent0 = Cross(worldTangent0, worldNormal0).Normalized();
	auto worldBitangent1 = Cross(worldTangent1, worldNormal1).Normalized();
	auto worldBitangent2 = Cross(worldTangent2, worldNormal2).Normalized();

	// 收窄BBox
	for (int i = 0; i < 2; ++i) {
		bboxMax[i] = sbm::min(clamp[i], sbm::max({ screenV0[i], screenV1[i], screenV2[i] }));
		bboxMin[i] = sbm::max(0.f, sbm::min({ bboxMax[i], screenV0[i], screenV1[i], screenV2[i] }));
	}
	// 扫描并画点
	for (int i = bboxMin.y; i <= bboxMax.y; ++i)
	{
		for (int j = bboxMin.x; j <= bboxMax.x; ++j)
		{
			Vec2 p(j, i);
			Vec3 lambda3 = barycentric(screenV0.Truncated<2>(), screenV1.Truncated<2>(), screenV2.Truncated<2>(), p);
			if (lambda3[0] < .0f || lambda3[1] < .0f || lambda3[2] < .0f) {
				continue;
			}

			lambda3 = lambda3 * Vec3(1.f / (screenw0), 1.f / (screenw1), 1.f / (screenw2));// auto
			float screenw = 1.f/Dot(lambda3, Vec3(1));
			lambda3 *= screenw;
			float screenz = screenw;// (screenz0 * lambda3[0] + screenz1 * lambda3[1] + screenz2 * lambda3[2]);
			//float screenz =  (screenV0.z * lambda3[0] + screenV1.z * lambda3[1] + screenV2.z * lambda3[2]);

			//if (screenz >= zBuf.Get(j,i).r) continue;
			//zBuf.SetColor(j, i, SR::Color(screenz));
			// 插值
			float uvx = (uv0.x * lambda3[0] + uv1.x * lambda3[1] + uv2.x * lambda3[2]);
			float uvy = (1 - (uv0.y * lambda3[0] + uv1.y * lambda3[1] + uv2.y * lambda3[2]));
			float posx = (worldPos0.x * lambda3[0] + worldPos1.x * lambda3[1] + worldPos2.x * lambda3[2]);
			float posy = (worldPos0.y * lambda3[0] + worldPos1.y * lambda3[1] + worldPos2.y * lambda3[2]);
			float posz = (worldPos0.z * lambda3[0] + worldPos1.z * lambda3[1] + worldPos2.z * lambda3[2]);
			float worldNormalX = (worldNormal0.x * lambda3[0] + worldNormal1.x * lambda3[1] + worldNormal2.x * lambda3[2]);
			float worldNormalY = (worldNormal0.y * lambda3[0] + worldNormal1.y * lambda3[1] + worldNormal2.y * lambda3[2]);
			float worldNormalZ = (worldNormal0.z * lambda3[0] + worldNormal1.z * lambda3[1] + worldNormal2.z * lambda3[2]);
			float worldTangentX = (worldTangent0.x * lambda3[0] + worldTangent1.x * lambda3[1] + worldTangent2.x * lambda3[2]);
			float worldTangentY = (worldTangent0.y * lambda3[0] + worldTangent1.y * lambda3[1] + worldTangent2.y * lambda3[2]);
			float worldTangentZ = (worldTangent0.z * lambda3[0] + worldTangent1.z * lambda3[1] + worldTangent2.z * lambda3[2]);
			float worldBitangentX = (worldBitangent0.x * lambda3[0] + worldBitangent1.x * lambda3[1] + worldBitangent2.x * lambda3[2]);
			float worldBitangentY = (worldBitangent0.y * lambda3[0] + worldBitangent1.y * lambda3[1] + worldBitangent2.y * lambda3[2]);
			float worldBitangentZ = (worldBitangent0.z * lambda3[0] + worldBitangent1.z * lambda3[1] + worldBitangent2.z * lambda3[2]);

			// 采样
			SR::Color32 albedo;
			tex.Get(int(0.5f + texWidth * uvx), int(0.5f + texHeight * uvy), albedo.r, albedo.g, albedo.b, albedo.a);
			SR::Color32 ncolor;
			normalMap.Get(int(0.5f + normalWidth * uvx), int(0.5f + normalHeight * uvy), ncolor.r, ncolor.g, ncolor.b, ncolor.a);
			SR::Color32 glosscolor;
			specMap.Get(int(0.5f + specMapW * uvx), int(0.5f + specMapH * uvy), glosscolor.r, glosscolor.g, glosscolor.b, glosscolor.a);

			Vec3 pos(posx, posy, posz); // 引入法线变换后nz就不用反了
			Vec4 norm = Vec4((ncolor.r / 255.f) * 2 - 1, (ncolor.g / 255.f) * 2 - 1, (ncolor.b / 255.f) * 2 - 1, 0);
			Mat4 tan2World;
			tan2World.SetColumn(0, worldTangentX, worldTangentY, worldTangentZ, 0);
			tan2World.SetColumn(1, worldBitangentX, worldBitangentY, worldBitangentZ, 0);
			tan2World.SetColumn(2, worldNormalX, worldNormalY, worldNormalZ, 0);
			tan2World.SetColumn(3, 0, 0, 0, 1);
			norm = (tan2World * norm).Normalized();
			float diffuse = Dot(Vec3(norm), -lightDir);// *0.5f + 0.5f;

			Vec3 e = reflect(lightDir, Vec3(norm)).Normalized();
			Vec3 viewDir = (camera - pos).Normalized();
			float spec = 0.6f * sbm::pow(sbm::max(Dot(viewDir, e), 0.f), sbm::max<decltype(glosscolor.b)>(5, glosscolor.b));
			//assert(glosscolor.b >= 0);
			auto c = SR::Color32(
				/*sbm::clamp<int>(255 * (norm.x * 2 -1), 0, 255),
				sbm::clamp<int>(255 * (norm.y * 2 - 1), 0, 255),
				sbm::clamp<int>(255 * (norm.z * 2 - 1), 0, 255)*/
				sbm::clamp<int>(5 + albedo.r * (diffuse + spec), 0, 255),
				sbm::clamp<int>(5 + albedo.g * (diffuse + spec), 0, 255),
				sbm::clamp<int>(5 + albedo.b * (diffuse + spec), 0, 255)
			);

			image.Set(j, i, c.r, c.g, c.b, c.a);
		}
	}
}

void TestDraw(SR::Texture& image, SR::Texture& zBuf,
	Vec3 v0, Vec3 v1, Vec3 v2,
	Vec2 uv0, Vec2 uv1, Vec2 uv2,
	Vec3 n0, Vec3 n1, Vec3 n2,
	SR::Color color
)
{
	// 光照
	int screenW = gWidth, screenH = gHeight;
	static Vec3 lightDir(-1, -1, -1);
	lightDir.Normalize();
	int width = image.GetWidth() - 1;
	int height = image.GetHeight() - 1;
	float fltMax = sbm::Math::FloatMax;
	Vec2 bboxMin(fltMax, fltMax), bboxMax(-fltMax, -fltMax), clamp(width, height);

	static Vec3 center(0, 0, 0), up(0, 1, 0), trans(0, 0, 0), rot(0,0,0), scale(1, 1, 1);
	/*
	Vec3 camera(0, 0, 3),
	Mat4 proj = Mat4::Identity;
	proj.M(3, 2) = 1 / camera.z;*/
	//Mat4 viewport = GetViewportMatrix(0, 0, screenW, screenH);
	Mat4 trs = Mat4::TRS(trans, rot, scale);
	Mat4 view = gViewMat;
	Mat4 proj = gProjMat;
	Mat4 viewport = gViewportMat;
	Mat4 MVP = proj * view * trs;// proj ;

	Vec4 screenV0(v0.x, v0.y, v0.z, 1);
	Vec4 screenV1(v1.x, v1.y, v1.z, 1);
	Vec4 screenV2(v2.x, v2.y, v2.z, 1);
	Vec4 worldPos0(screenV0), worldPos1(screenV1), worldPos2(screenV2);

	screenV0 = trs * screenV0;
	worldPos0 = screenV0;
	screenV0 = view * screenV0;
	screenV0 = proj * screenV0;
	float screenw0 = screenV0.w;
	screenV0 = viewport * (screenV0 / screenV0.w);

	screenV1 = trs * screenV1;
	worldPos1 = screenV1;
	screenV1 = view * screenV1;
	screenV1 = proj * screenV1;
	float screenw1 = screenV1.w;
	screenV1 = viewport * (screenV1 / screenV1.w);

	screenV2 = trs * screenV2;
	worldPos2 = screenV2;
	screenV2 = view * screenV2;
	screenV2 = proj * screenV2;
	float screenw2 = screenV2.w;
	screenV2 = viewport * (screenV2 / screenV2.w);

	sbm::Matrix<2, 2> uvMat({ uv1.x - uv0.x, uv2.x - uv0.x,uv1.y - uv0.y, uv2.y - uv0.y });
	sbm::Matrix<2, 3> worldPosMat({
		worldPos1.x - worldPos0.x, worldPos2.x - worldPos0.x,
		worldPos1.y - worldPos0.y, worldPos2.y - worldPos0.y,
		worldPos1.z - worldPos0.z, worldPos2.z - worldPos0.z,
		});

	Mat4 trsInvT = trs.GetInversed().GetTransposed();
	auto worldNormal0 = (trsInvT * n0.Expanded<4>(0));
	auto worldNormal1 = (trsInvT * n1.Expanded<4>(0));
	auto worldNormal2 = (trsInvT * n2.Expanded<4>(0));

	auto tbMat = uvMat.GetInversed() * worldPosMat;
	auto tangent = tbMat.GetRow(0).Expanded<4>(0);
	auto bitangent = tbMat.GetRow(1).Expanded<4>(0);
	auto normal = Cross(bitangent, tangent);
	float w0 = Dot(normal, worldNormal0) < 0 ? -1 : 1;
	float w1 = Dot(normal, worldNormal1) < 0 ? -1 : 1;
	float w2 = Dot(normal, worldNormal2) < 0 ? -1 : 1;
	tangent.Normalize();
	worldNormal0.Normalize();	worldNormal1.Normalize();	worldNormal2.Normalize();
	auto worldTangent0 = w0 * (tangent - Dot(tangent, worldNormal0) * worldNormal0).Normalized();
	auto worldTangent1 = w1 * (tangent - Dot(tangent, worldNormal1) * worldNormal1).Normalized();
	auto worldTangent2 = w2 * (tangent - Dot(tangent, worldNormal2) * worldNormal2).Normalized();
	auto worldBitangent0 = Cross(worldTangent0, worldNormal0).Normalized();
	auto worldBitangent1 = Cross(worldTangent1, worldNormal1).Normalized();
	auto worldBitangent2 = Cross(worldTangent2, worldNormal2).Normalized();

	// 收窄BBox
	for (int i = 0; i < 2; ++i) {
		bboxMax[i] = sbm::min(clamp[i], sbm::max({ screenV0[i], screenV1[i], screenV2[i] }));
		bboxMin[i] = sbm::max(0.f, sbm::min({ bboxMax[i], screenV0[i], screenV1[i], screenV2[i] }));
	}
	// 扫描并画点
	for (int i = bboxMin.y; i <= bboxMax.y; ++i)
	{
		for (int j = bboxMin.x; j <= bboxMax.x; ++j)
		{
			Vec2 p(j, i);
			Vec3 lambda3 = barycentric(screenV0.Truncated<2>(), screenV1.Truncated<2>(), screenV2.Truncated<2>(), p);
			if (lambda3[0] < .0f || lambda3[1] < .0f || lambda3[2] < .0f) {
				continue;
			}

			lambda3 = lambda3 * Vec3(1.f / (screenw0), 1.f / (screenw1), 1.f / (screenw2));// auto
			float screenw = Dot(lambda3, Vec3(1));
			lambda3 /= screenw;
			float screenz = 1 / screenw;// (screenz0 * lambda3[0] + screenz1 * lambda3[1] + screenz2 * lambda3[2]);
			//float screenz =  (screenV0.z * lambda3[0] + screenV1.z * lambda3[1] + screenV2.z * lambda3[2]);

			/*if (screenz >= zBuf.GetColor(j, i).r) continue;
			zBuf.SetColor(j, i, SR::Color(screenz));*/
			// 插值
			float uvx = (uv0.x * lambda3[0] + uv1.x * lambda3[1] + uv2.x * lambda3[2]);
			float uvy = (1 - (uv0.y * lambda3[0] + uv1.y * lambda3[1] + uv2.y * lambda3[2]));
			float posx = (worldPos0.x * lambda3[0] + worldPos1.x * lambda3[1] + worldPos2.x * lambda3[2]);
			float posy = (worldPos0.y * lambda3[0] + worldPos1.y * lambda3[1] + worldPos2.y * lambda3[2]);
			float posz = (worldPos0.z * lambda3[0] + worldPos1.z * lambda3[1] + worldPos2.z * lambda3[2]);
			float worldNormalX = (worldNormal0.x * lambda3[0] + worldNormal1.x * lambda3[1] + worldNormal2.x * lambda3[2]);
			float worldNormalY = (worldNormal0.y * lambda3[0] + worldNormal1.y * lambda3[1] + worldNormal2.y * lambda3[2]);
			float worldNormalZ = (worldNormal0.z * lambda3[0] + worldNormal1.z * lambda3[1] + worldNormal2.z * lambda3[2]);
			float worldTangentX = (worldTangent0.x * lambda3[0] + worldTangent1.x * lambda3[1] + worldTangent2.x * lambda3[2]);
			float worldTangentY = (worldTangent0.y * lambda3[0] + worldTangent1.y * lambda3[1] + worldTangent2.y * lambda3[2]);
			float worldTangentZ = (worldTangent0.z * lambda3[0] + worldTangent1.z * lambda3[1] + worldTangent2.z * lambda3[2]);
			float worldBitangentX = (worldBitangent0.x * lambda3[0] + worldBitangent1.x * lambda3[1] + worldBitangent2.x * lambda3[2]);
			float worldBitangentY = (worldBitangent0.y * lambda3[0] + worldBitangent1.y * lambda3[1] + worldBitangent2.y * lambda3[2]);
			float worldBitangentZ = (worldBitangent0.z * lambda3[0] + worldBitangent1.z * lambda3[1] + worldBitangent2.z * lambda3[2]);

			// 采样

			Vec3 pos(posx, posy, posz); // 引入法线变换后nz就不用反了
			Vec4 norm = n0.Expanded<4>(0);// Vec4((ncolor.r / 255.f) * 2 - 1, (ncolor.g / 255.f) * 2 - 1, (ncolor.b / 255.f) * 2 - 1, 0);
			Mat4 tan2World;
			tan2World.SetColumn(0, worldTangentX, worldTangentY, worldTangentZ, 0);
			tan2World.SetColumn(1, worldBitangentX, worldBitangentY, worldBitangentZ, 0);
			tan2World.SetColumn(2, worldNormalX, worldNormalY, worldNormalZ, 0);
			tan2World.SetColumn(3, 0, 0, 0, 1);
			norm = (tan2World * norm).Normalized();
			float diffuse = Dot(Vec3(norm), -lightDir);// *0.5f + 0.5f;

			Vec3 e = reflect(lightDir, Vec3(norm)).Normalized();
			Vec3 viewDir = (gCameraPos - pos).Normalized();
			float spec = 0;// 0.6f * sbm::pow(sbm::max(Dot(viewDir, e), 0.f), sbm::max<decltype(glosscolor.b)>(5, glosscolor.b));
			//assert(glosscolor.b >= 0);
			auto c = SR::Color32(
				/*sbm::clamp<int>(255 * (norm.x * 2 -1), 0, 255),
				sbm::clamp<int>(255 * (norm.y * 2 - 1), 0, 255),
				sbm::clamp<int>(255 * (norm.z * 2 - 1), 0, 255)*/
				sbm::clamp<int>(diffuse * 255, 0, 255),
				sbm::clamp<int>(diffuse * 255, 0, 255),
				sbm::clamp<int>(diffuse * 255, 0, 255)
			);

			image.Set(j, i, c.r, c.g, c.b, c.a);
		}
	}
}

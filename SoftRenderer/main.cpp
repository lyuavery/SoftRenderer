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

int main()
{
	// Camera
	auto& mainCam = *SR::Camera::mainCamera;
	mainCam.nearClip = 0.01f;
	mainCam.farClip = 7;
	mainCam.fov = 75;
	mainCam.bOrtho = true;
	mainCam.orthoSize = 2;
	mainCam.viewport = SR::Viewport::main;
	mainCam.RegisterInputListener();

	mainCam.position = Vec3(0.f,0,3);
	mainCam.LookAt(Vec3(0., 0, 0));
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
	task1.Bind(tri.get());

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
		else if (SR::Input::GetKeyDown(SR::KeyCode::Alpha3))
		{
			mainCam.bOrtho = true;
		}
		else if (SR::Input::GetKeyDown(SR::KeyCode::Alpha4))
		{
			mainCam.bOrtho = false;
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

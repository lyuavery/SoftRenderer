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
	mainCam.farClip = 50;
	mainCam.fov = 75;
	mainCam.bOrtho = false;
	mainCam.orthoSize = 2;
	mainCam.viewport = SR::Viewport::main;
	mainCam.RegisterInputListener();
	mainCam.position = Vec3(0.f,0,3);
	mainCam.LookAt(Vec3(0., 0, 0));

	// Resources
	SR::TGALoader tgaLoader;
	std::shared_ptr<SR::Mesh> africanHead(SR::MeshLoader::GetInstance().Load("Resources/african_head/african_head.obj", true));
	std::shared_ptr<SR::Texture> africanHeadDiffuse(tgaLoader.Load("Resources/african_head/african_head_diffuse.tga"));// 
	std::shared_ptr<SR::Texture> africanHeadNormal(tgaLoader.Load("Resources/african_head/african_head_nm_tangent.tga"));// 
	std::shared_ptr<SR::Texture> africanHeadSpec(tgaLoader.Load("Resources/african_head/african_head_spec.tga"));// 

	// Window
	auto& wnd = SR::Window::GetInstance();
	wnd.Init(SR::Viewport::main.width, SR::Viewport::main.height, SR::Viewport::main.bTopDown);
	auto& backBuf = wnd.GetBackBuffer();
	if (backBuf)
	{
		if (backBuf->colorBuf) backBuf->colorBuf->Clear(SR::Color32::grey.r, SR::Color32::grey.g, SR::Color32::grey.b, SR::Color32::grey.a);
		if (backBuf->depthBuf) backBuf->depthBuf->Clear(SR::Color::white.r, 0.f,0.f,0.f);
	}
	
	// Transform
	Mat4 modelTRS = Mat4::TRS(Vec3(0,0,0), Vec3(0, 0, 0), Vec3(1.f));
	Mat4 modelTRTInv = modelTRS.GetInversed();

	// Shader && Render State
	SR::BlinnPhongVert blinnVert;
	SR::BlinnPhongFrag blinnFrag;
	SR::BlinnPhongVarying blinnVarying;
	SR::BlinnPhongUniform blinnUniform;
	blinnUniform.albedo = africanHeadDiffuse;
	blinnUniform.normal = africanHeadNormal;
	blinnUniform.spec = africanHeadSpec;
	blinnUniform.worldLightDir = Vec3(-1);
	blinnUniform.mat_ObjectToWorld = modelTRS;

	SR::RenderStatus status;
	status.depthFunc = SR::DepthFunc::Less;
	status.bEarlyDepthTest = true;
	status.rasterizationMode = SR::RasterizationMode::Filled;
	status.cullFace = SR::Culling::Back;

	SR::RenderTask task;
	task.status = status;
	task.frameBuffer = backBuf;
	task.status = status;
	task.frameBuffer = backBuf;
	task.Bind(&blinnVert, &blinnVarying);
	task.Bind(&blinnFrag);
	task.Bind(africanHead.get());

	// Time
	SR::Time::Init();
	float lastPrintTime = SR::Time::TimeSinceStartup();

	// Main Loop
	while (!wnd.ShouldExit()) {
		SR::Time::Update();		
		float deltaTime = SR::Time::DeltaTime();

		// Input Update
		SR::Input::Update(deltaTime);
		
		// Logic Update
		if (SR::Input::GetKeyDown(SR::KeyCode::Alpha1))
		{
			task.status.rasterizationMode = task.status.rasterizationMode = SR::RasterizationMode::Filled;
		}		
		else if (SR::Input::GetKeyDown(SR::KeyCode::Alpha2))
		{
			task.status.rasterizationMode = task.status.rasterizationMode = SR::RasterizationMode::Line;
		}
		else if (SR::Input::GetKeyDown(SR::KeyCode::Alpha3))
		{
			mainCam.bOrtho = true;
		}
		else if (SR::Input::GetKeyDown(SR::KeyCode::Alpha4))
		{
			mainCam.bOrtho = false;
		}

		// Render Loop
		if (backBuf)
		{
			if (backBuf->colorBuf) backBuf->colorBuf->Clear(SR::Color32::grey.r, SR::Color32::grey.g, SR::Color32::grey.b, SR::Color32::grey.a);
			if (backBuf->depthBuf) backBuf->depthBuf->Clear(SR::Color::white.r, 0.f, 0.f, 0.f);
		}
		blinnUniform.worldCamPos = mainCam.position;
		blinnUniform.mat_ObjectToClip = mainCam.ProjectionMatrix() * mainCam.ViewMatrix();
		task.Bind(&blinnUniform);
		task.Submit();
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

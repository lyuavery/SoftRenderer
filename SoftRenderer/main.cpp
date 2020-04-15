#include <tchar.h>
#include <iostream>
#include <string>
#include <memory>

#include "Header.h"
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


static int screen_keys[512];	// 当前键盘按下状态
//#ifdef _MSC_VER
//#pragma comment(lib, "gdi32.lib")
//#pragma comment(lib, "user32.lib")
//#endif

// 先考虑单线程
// 首要目标是将教程的东西实时显示，允许截图（保存到tga）
// 然后实现裁剪,scanline,top-left和模式切换
// 到这一步就是一个完整的东西了
// WindowMgr
		// FrameBuffer
// ResLoader
// Time

// 管线化
// 先考虑最基础组件，而且按自己的理解来设计

// Actor
	// Transform
	// Renderer
// App
	// InputMgr
	// SceneMgr
		// Actor
			// Transform
			// Renderer
				// MeshRenderer
				// SkinnedMeshRenderer
			// Mesh
			// MeshFilter
			// Material
				// Shader
			// Texture
	// RendererMgr
		// Vertex
		// Rasterize
		// Pixel
	// WindowMgr
		// FrameBuffer
// ResLoader
// Time
// Profiler
// Log
// Math

namespace SR
{
	class Logger
	{
		static const int DEFAULT_RESERVE = 512;
	private:
	public:
		void LogInfo(char* fmt, ...)
		{
			auto fd = stdout;
			const char* tag = "INFO";

			std::string extFmt("[");
			extFmt.reserve(DEFAULT_RESERVE);
			extFmt = extFmt + tag + "] " + __FILE__ + ": Line " + std::to_string(__LINE__) + ":\n" + fmt;
			va_list args;
			va_start(args, fmt);
			vfprintf(fd, extFmt.c_str(), args);
			va_end(args);
		}

		void LogWarning(char* fmt, ...)
		{
			auto fd = stdout;
			const char* tag = "WARNING";

			std::string extFmt("[");
			extFmt.reserve(DEFAULT_RESERVE);
			extFmt = extFmt + tag + "] " + __FILE__ + ": Line " + std::to_string(__LINE__) + ":\n" + fmt;
			va_list args;
			va_start(args, fmt);
			vfprintf(fd, extFmt.c_str(), args);
			va_end(args);
		}

		void LogError(char* fmt, ...)
		{
			auto fd = stderr;
			const char* tag = "ERROR";

			std::string extFmt("[");
			extFmt.reserve(DEFAULT_RESERVE);
			extFmt = extFmt + tag + "] " + __FILE__ + ": Line " + std::to_string(__LINE__) + ":\n" + fmt;
			va_list args;
			va_start(args, fmt);
			vfprintf(fd, extFmt.c_str(), args);
			va_end(args);
		}
	};

	/*class RenderTarget
	{
	public:
		static std::shared_ptr<RenderTarget> active;
		std::unique_ptr<FrameBuffer> frameBuffer;
	};
	std::shared_ptr<RenderTarget> RenderTarget::active;
*/
}


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

//
//class MathSymbol {
//	std::string str;
//public:
//	MathSymbol() { str = ""; }
//	MathSymbol(const MathSymbol& v) { str = v.str; }
//	MathSymbol(const char* v) :str(v) { }
//	MathSymbol(const std::string& v) :str(v) { }
//	MathSymbol(const float v):str("("+std::to_string(v)+")") { }
//	MathSymbol operator+(const MathSymbol& symbol) {
//		if (symbol.str == "0" && str == "0")  return MathSymbol("0");
//		if (symbol.str == "0") return *this;
//		if (str == "0") return symbol;
//		return MathSymbol(str+ "+" + symbol.str);
//	}
//	MathSymbol operator*(const MathSymbol& symbol) {
//		if (symbol.str == "0" || str == "0") return MathSymbol("0");
//		if (symbol.str == "1" && str == "1")  return MathSymbol("1");
//		if (symbol.str == "1") return *this;
//		if (str == "1") return symbol;
//		return MathSymbol(str + "*" + symbol.str);
//	}
//	MathSymbol& operator=(const MathSymbol& symbol) {
//		str = symbol.str;
//		return *this;
//	}
//	friend std::ostream& operator<<(std::ostream& out, const MathSymbol& ms);
//};
//std::ostream& operator<<(std::ostream& out, const MathSymbol& ms)
//{
//	out << ms.str;
//	return out;
//}
//MathSymbol rxarr[] = {
//		"1","0","0","0",
//		"0","cosx","-sinx","0",
//		"0","sinx","cosx","0",
//		"0","0","0","1",
//};
//TestMat rx(rxarr);
//
//MathSymbol ryarr[] = {
//	"cosy","0","siny","0",
//	"0","1","0","0",
//	"-siny","0","cosy","0",
//	"0","0","0","1",
//};
//TestMat ry(ryarr);
//
//MathSymbol rzarr[] = {
//	"cosz","-sinz","0","0",
//	"sinz","cosz","0","0",
//	"0","0","1","0",
//	"0","0","0","1",
//};
//TestMat rz(rzarr);
////TestMat ret = ry * rx * rz;
//TestMat ret = rz * ry * rx;
//std::cout << ret;
//getchar();
//return 0;
//typedef sbm::Matrix<4, 4, MathSymbol> TestMat;

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
int main()
{
	std::cout << sizeof(Mat4::size()) << std::endl;
	std::cout << sizeof(Vec4::size()) << std::endl;
	std::cout << sizeof(sbm::Matrix<2,3>::size()) << std::endl;
	getchar();
	return 0;
	SR::Camera::mainCamera = std::unique_ptr<SR::Camera>(new SR::Camera(Vec3(0,0,3), Vec3(0)));
	auto& mainCam = *SR::Camera::mainCamera;
	mainCam.viewport = SR::Viewport(0, 0, gWidth, gHeight, true);

	mainCam.RegisterInputListener();
	
	gViewportMat = mainCam.ViewportTransform();
	gCameraPos = mainCam.position;

	int w = gWidth, h = gHeight;

	auto& wnd = SR::Window::GetInstance();
	wnd.Init(w, h, true);
	auto& backBuf = wnd.GetBackBuffer();
	if (backBuf)
	{
		if (backBuf->colorBuf) backBuf->colorBuf->Clear(SR::Color32::grey.r, SR::Color32::grey.g, SR::Color32::grey.b, SR::Color32::grey.a);
		if (backBuf->depthBuf) backBuf->depthBuf->Clear(SR::Color::black.r, 0.f,0.f,0.f);
	}
	//SR::RenderTarget::active = backBuf;
	//memset(screen_keys, 0, sizeof(int) * 512);
	/*foreach pluginMgr.plugins.Init();
	while (!window->IsExit()) {
		inputsystem->Update();
		render
	}*/

	//std::unique_ptr<SR::Mesh> floor(SR::MeshLoader::GetInstance().Load("Resources/floor/floor.obj"));
	float fltMax = sbm::Math::FloatMax;
	std::shared_ptr<SR::Mesh> africanHead(SR::MeshLoader::GetInstance().Load("Resources/african_head/african_head.obj"));
	SR::TGALoader tgaLoader;
	std::shared_ptr<SR::Texture2D> africanHeadDiffuse(tgaLoader.Load("Resources/african_head/african_head_diffuse.tga"));// 
	std::shared_ptr<SR::Texture2D> africanHeadNormal(tgaLoader.Load("Resources/african_head/african_head_nm_tangent.tga"));// 
	std::shared_ptr<SR::Texture2D> africanHeadSpec(tgaLoader.Load("Resources/african_head/african_head_spec.tga"));// 

	/*for (int j = 0, w = backBuf->colorBuf.GetWidth(), h = backBuf->colorBuf.GetHeight(); j < h; ++j)
	{
		for (int i = 0; i < w; ++i)
		{
			backBuf->colorBuf.Set(i, j, africanHeadSpec->Get(i, j));
		}
	}
*/
	SR::Time::Init();
	float lastPrintTime = SR::Time::TimeSinceStartup();
	while (!wnd.ShouldExit()) {
		SR::Time::Update();		
		float deltaTime = SR::Time::DeltaTime();

		SR::Input::Update(deltaTime);
		gViewMat = mainCam.ViewMatrix();
		gProjMat = mainCam.ProjectionMatrix();

		if (SR::Input::GetKeyDown(SR::KeyCode::Z))
		{
			mainCam.Orbit(45, 45);
		}

		if (backBuf)
		{
			if (backBuf->colorBuf) backBuf->colorBuf->Clear(SR::Color32::grey.r, SR::Color32::grey.g, SR::Color32::grey.b, SR::Color32::grey.a);
			if (backBuf->depthBuf) backBuf->depthBuf->Clear(SR::Color::black.r, 0.f, 0.f, 0.f);
		}
		//TestDraw(backBuf->colorBuf, backBuf->depthBuf,
		//	//Vec3(-.5f, -.5f, .5f), Vec3(.5f, -.5f, -.5f), Vec3(0, .5f, 0),
		//	//Vec2(0, 0), Vec2(1, 0), Vec2(.5, 1),
		//	//Vec3(1, 0, 1), Vec3(1, 0, 1), Vec3(1, 0, 1),
		//	Vec3(-.5f, -.5f, .0f), Vec3(.5f, -.5f, .0f), Vec3(-.5f, .5f, 0),
		//	Vec2(0, 0), Vec2(1, 0), Vec2(.5, 1),
		//	Vec3(0, 0, 1), Vec3(0, 0, 1), Vec3(0, 0, 1),
		//	SR::Color::white
		//	);
		{
			auto& f = africanHead->indices;
			//for (auto f : africanHead->faces)
			for (int i = 0, n = africanHead->indices.size(); i < n; i += 3)
			{
				/*auto v0 = africanHead->vertices[f[0][0]];
				auto v1 = africanHead->vertices[f[1][0]];
				auto v2 = africanHead->vertices[f[2][0]];

				auto uv0 = africanHead->uv[f[0][1]];
				auto uv1 = africanHead->uv[f[1][1]];
				auto uv2 = africanHead->uv[f[2][1]];

				auto n0 = africanHead->normals[f[0][2]];
				auto n1 = africanHead->normals[f[1][2]];
				auto n2 = africanHead->normals[f[2][2]];*/
				auto v0 = africanHead->vertices[f[i + 0]];
				auto v1 = africanHead->vertices[f[i + 1]];
				auto v2 = africanHead->vertices[f[i + 2]];

				auto uv0 = africanHead->uvs[f[i + 1]];
				auto uv1 = africanHead->uvs[f[i + 1]];
				auto uv2 = africanHead->uvs[f[i + 1]];

				auto n0 = africanHead->normals[f[i + 2]];
				auto n1 = africanHead->normals[f[i + 2]];
				auto n2 = africanHead->normals[f[i + 2]];
				DrawFilledTrianxgleBarycentricCoordinate_Texture_TestPerspectiveCorrection(*(backBuf->colorBuf), *africanHeadDiffuse, *africanHeadNormal, *africanHeadSpec, *(backBuf->depthBuf), v0, v1, v2, uv0, uv1, uv2, n0, n1, n2);
			}
		}
		
		wnd.Update(deltaTime);
		float now = SR::Time::TimeSinceStartup();
		if ((now - lastPrintTime) > 1)
		{
			std::cout << "FPS:" << SR::Time::FPS() << std::endl;
			lastPrintTime = now;
		}
		wnd.Dispatch();
		//Sleep(1);
	}
	wnd.Destroy();

	return 0;
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

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
#include "Window.h"
#include "Log.h"

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

	enum class KeyCode
	{
		None = 0,
		Return,
		Escape,
		Space,
		A,
		D,
		E,
		Q,
		S,
		W,
		UpArrow,
		DownArrow,
		RightArrow,
		LeftArrow,
		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		LeftCtrl,
		RightCtrl,
		LeftMouse,
		MiddleMouse,
		RightMouse,
	};

	class InputMgr
	{
		friend class WindowMgr;
	private:
		bool keyDown[512];
		bool keyUp[512];
		bool isDirty;
	public:

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


Vec3 Reflect(const Vec3& i, const Vec3& n)
{
	//Vec3 iNorm = i.Normalized();
	//Vec3 nNorm = n.Normalized();
	return i - 2 * Dot(i, n) * n;
}

Vec3 BarycentricF_Projection(const Vec4& v0, const Vec4& v1, const Vec4& v2, const Vec2& p)
{
	// (u, v, 1) 与（AB.x, AC.x, PA.x) 和（AB.y, AC.y, PA.y) 正交，
	Vec3 lambda = Cross(Vec3(v1.x - v0.x, v2.x - v0.x, v0.x - p.x), Vec3(v1.y - v0.y, v2.y - v0.y, v0.y - p.y));
	// 由于使用整数坐标，lambda.z为两倍三角形面积，等于0时证明退化
	if (sbm::abs(lambda.z) < 1e-4) return Vec3(-1, 1, 1);
	float rcpArea = 1.0f / lambda.z;
	// 除以lambda.z来规范重心坐标和为1
	return Vec3(1.f - (lambda.x + lambda.y) * rcpArea, lambda.x * rcpArea, lambda.y * rcpArea);
}

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

Mat4 GetTRSMatrix(const Vec3& translation, const Vec3& rotation, const Vec3& scale)
{
	const float d2r = sbm::Math::PI / 180.f;
	float radx = rotation.x * d2r, rady = rotation.y * d2r, radz = rotation.z * d2r;
	float cosx = sbm::cos(radx), sinx = sbm::sin(radx), cosy = sbm::cos(rady), siny = sbm::sin(rady), cosz = sbm::cos(radz), sinz = sbm::sin(radz);
	Mat4 rotx = Mat4::Identity, roty = Mat4::Identity, rotz = Mat4::Identity;
	rotx.M(1, 1) = cosx; rotx.M(1, 2) = -sinx; rotx.M(2, 1) = sinx; rotx.M(2, 2) = cosx;
	roty.M(0, 0) = cosy; roty.M(0, 2) = siny; roty.M(2, 0) = -siny; roty.M(2, 2) = cosy;
	rotz.M(0, 0) = cosz; rotz.M(0, 1) = -sinz; rotz.M(1, 0) = sinz; rotz.M(1, 1) = cosz;
	Mat4 trs = roty * rotx * rotz;
	trs.M(0, 0) *= scale.x; trs.M(1, 0) *= scale.x; trs.M(2, 0) *= scale.x;
	trs.M(0, 1) *= scale.y; trs.M(1, 1) *= scale.y; trs.M(2, 1) *= scale.y;
	trs.M(0, 2) *= scale.z; trs.M(1, 2) *= scale.z; trs.M(2, 2) *= scale.z;
	trs.M(0, 3) = translation.x; trs.M(1, 3) = translation.y; trs.M(2, 3) = translation.z;
	return trs;
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

	static Mat4 viewport = GetViewportMatrix(0, 0, screenW, screenH);
	static Vec3 camera(1, 1, 3), center(0, 0, 0), up(0, 1, 0), trans(0, 0, 0), rot(0, 0, 0), scale(1, 1, 1);
	Mat4 proj = Mat4::Identity;
	proj.M(3, 2) = 1 / camera.z;
	Mat4 trs = GetTRSMatrix(trans, rot, scale);
	Mat4 view = GetLookAtMatrix(camera, center, up);
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
			Vec3 lambda3 = BarycentricF_Projection(screenV0, screenV1, screenV2, p);
			if (lambda3[0] < .0f || lambda3[1] < .0f || lambda3[2] < .0f) {
				continue;
			}

			lambda3 = lambda3 * Vec3(1.f / (screenw0), 1.f / (screenw1), 1.f / (screenw2));// auto
			float screenw = Dot(lambda3, Vec3(1));
			lambda3 /= screenw;
			float screenz = 1 / screenw;// (screenz0 * lambda3[0] + screenz1 * lambda3[1] + screenz2 * lambda3[2]);
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
			SR::Color32 albedo = tex.GetColor32(int(0.5f + texWidth * uvx), int(0.5f + texHeight * uvy));
			SR::Color32 ncolor = normalMap.GetColor32(int(0.5f + normalWidth * uvx), int(0.5f + normalHeight * uvy));
			SR::Color32 glosscolor = specMap.GetColor32(int(0.5f + specMapW * uvx), int(0.5f + specMapH * uvy));

			Vec3 pos(posx, posy, posz); // 引入法线变换后nz就不用反了
			Vec4 norm = Vec4((ncolor.r / 255.f) * 2 - 1, (ncolor.g / 255.f) * 2 - 1, (ncolor.b / 255.f) * 2 - 1, 0);
			Mat4 tan2World;
			tan2World.SetColumn(0, worldTangentX, worldTangentY, worldTangentZ, 0);
			tan2World.SetColumn(1, worldBitangentX, worldBitangentY, worldBitangentZ, 0);
			tan2World.SetColumn(2, worldNormalX, worldNormalY, worldNormalZ, 0);
			tan2World.SetColumn(3, 0, 0, 0, 1);
			norm = (tan2World * norm).Normalized();
			float diffuse = Dot(Vec3(norm), -lightDir);// *0.5f + 0.5f;

			Vec3 e = Reflect(lightDir, Vec3(norm)).Normalized();
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

			image.SetColor32(j, i, c);
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

	static Mat4 viewport = GetViewportMatrix(0, 0, screenW, screenH);
	static Vec3 camera(1, 1, 3), center(0, 0, 0), up(0, 1, 0), trans(0, 0, 0), rot(0, 0, 0), scale(1, 1, 1);
	Mat4 proj = Mat4::Identity;
	proj.M(3, 2) = 1 / camera.z;
	Mat4 trs = GetTRSMatrix(trans, rot, scale);
	Mat4 view = GetLookAtMatrix(camera, center, up);
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
			Vec3 lambda3 = BarycentricF_Projection(screenV0, screenV1, screenV2, p);
			if (lambda3[0] < .0f || lambda3[1] < .0f || lambda3[2] < .0f) {
				continue;
			}

			lambda3 = lambda3 * Vec3(1.f / (screenw0), 1.f / (screenw1), 1.f / (screenw2));// auto
			float screenw = Dot(lambda3, Vec3(1));
			lambda3 /= screenw;
			float screenz = 1 / screenw;// (screenz0 * lambda3[0] + screenz1 * lambda3[1] + screenz2 * lambda3[2]);
			//float screenz =  (screenV0.z * lambda3[0] + screenV1.z * lambda3[1] + screenV2.z * lambda3[2]);

			//if (screenz >= zBuf.Get(j, i).r) continue;
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
			
			Vec3 pos(posx, posy, posz); // 引入法线变换后nz就不用反了
			Vec4 norm = n0.Expanded<4>(0);// Vec4((ncolor.r / 255.f) * 2 - 1, (ncolor.g / 255.f) * 2 - 1, (ncolor.b / 255.f) * 2 - 1, 0);
			Mat4 tan2World;
			tan2World.SetColumn(0, worldTangentX, worldTangentY, worldTangentZ, 0);
			tan2World.SetColumn(1, worldBitangentX, worldBitangentY, worldBitangentZ, 0);
			tan2World.SetColumn(2, worldNormalX, worldNormalY, worldNormalZ, 0);
			tan2World.SetColumn(3, 0, 0, 0, 1);
			norm = (tan2World * norm).Normalized();
			float diffuse = Dot(Vec3(norm), -lightDir);// *0.5f + 0.5f;

			Vec3 e = Reflect(lightDir, Vec3(norm)).Normalized();
			Vec3 viewDir = (camera - pos).Normalized();
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

			image.SetColor32(j, i, c);
		}
	}
}


int main()
{
	int w = gWidth, h = gHeight;
	auto& wnd = SR::Window::GetInstance();
	wnd.Init(w, h);
	auto backBuf = wnd.GetBackBuffer();
	if (backBuf)
	{
		backBuf->Clear(SR::Color32::grey);
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
	std::unique_ptr<SR::Mesh> africanHead(SR::MeshLoader::GetInstance().Load("Resources/african_head/african_head.obj"));
	SR::TGALoader tgaLoader;
	std::unique_ptr<SR::Texture2D> africanHeadDiffuse(tgaLoader.Load("Resources/african_head/african_head_diffuse.tga"));// 
	std::unique_ptr<SR::Texture2D> africanHeadNormal(tgaLoader.Load("Resources/african_head/african_head_nm_tangent.tga"));// 
	std::unique_ptr<SR::Texture2D> africanHeadSpec(tgaLoader.Load("Resources/african_head/african_head_spec.tga"));// 

	/*for (int j = 0, w = backBuf->colorBuf.GetWidth(), h = backBuf->colorBuf.GetHeight(); j < h; ++j)
	{
		for (int i = 0; i < w; ++i)
		{
			backBuf->colorBuf.Set(i, j, africanHeadSpec->Get(i, j));
		}
	}
*/
	float prevTimeMark = SR::Time::Now();
	float prevTime = prevTimeMark;
	float elapseTime = 0;
	float elapseFrameCnt = 0;
	float frameCnt = 0;
	while (!wnd.ShouldExit()) {
		float now = SR::Time::Now();
		float deltaTime = now - prevTime;
		prevTime = now;
		elapseTime += deltaTime;
		++elapseFrameCnt;
		++frameCnt;

		TestDraw(backBuf->colorBuf, backBuf->depthBuf,
			Vec3(-.5f, -.5f, .5f), Vec3(.5f, -.5f, -.5f), Vec3(0, .5f, 0),
			Vec2(0, 0), Vec2(1, 0), Vec2(.5, 1),
			Vec3(1, 0, 1), Vec3(1, 0, 1), Vec3(1, 0, 1),
			SR::Color::white
			);
		/*for (auto f : africanHead->faces) {
			auto v0 = africanHead->vertices[f[0][0]];
			auto v1 = africanHead->vertices[f[1][0]];
			auto v2 = africanHead->vertices[f[2][0]];

			auto uv0 = africanHead->uv[f[0][1]];
			auto uv1 = africanHead->uv[f[1][1]];
			auto uv2 = africanHead->uv[f[2][1]];

			auto n0 = africanHead->normals[f[0][2]];
			auto n1 = africanHead->normals[f[1][2]];
			auto n2 = africanHead->normals[f[2][2]];
			
			DrawFilledTrianxgleBarycentricCoordinate_Texture_TestPerspectiveCorrection(backBuf->colorBuf, *africanHeadDiffuse, *africanHeadNormal, *africanHeadSpec, backBuf->depthBuf, v0, v1, v2, uv0, uv1, uv2, n0, n1, n2);
		}*/
		wnd.Update(deltaTime);
		if ((now - prevTimeMark) > 1)
		{
			std::cout << "FPS:" << (elapseFrameCnt / elapseTime) << std::endl;
			prevTimeMark = now;
			elapseTime = 0;
			elapseFrameCnt = 0;
		}
		wnd.Dispatch();
		Sleep(1);
	}
	wnd.Destroy();


	//TGAImage texture, normalMap, specMap;
	//texture.read_tga_file("res/diablo3_pose/diablo3_pose_diffuse.tga"); // 
	//normalMap.read_tga_file("res/diablo3_pose/diablo3_pose_nm_tangent.tga"); // 
	//specMap.read_tga_file("res/diablo3_pose/diablo3_pose_spec.tga");

	//TGAImage floorDiff, floorNormal;
	//floorDiff.read_tga_file("res/floor/floor_diffuse.tga"); // african_head_diffuse
	//floorNormal.read_tga_file("res/floor/floor_nm_tangent.tga"); // african_head_diffuse

	// Line
	{
		//DrawLine(image, 600, 700, 500, 500, white);
		/*
		DrawLine(image, 110, 10, 290, 160, red);
		DrawLine(image, 290, 160, 200, 30, green);
		DrawLine(image, 200, 30, 110, 10, blue);*/
	}

	// Filled Mesh
	//TGAColor clearColor(255, 255, 255);
	//TGAImage defFB(_G_WIDTH, _G_HEIGHT, TGAImage::RGB);
	//defFB.clear();

	//std::unique_ptr<float> zBuf(new float[defFB.get_width() * defFB.get_height()]);
	//for (int i = 0, n = defFB.get_width() * defFB.get_height(); i < n; ++i) zBuf.get()[i] = fltMax;

	//TGAImage shadowMap(_G_WIDTH, _G_HEIGHT, TGAImage::GRAYSCALE);
	//for (int i = 0, w = shadowMap.get_width(); i < w; ++i) {
	//	for (int j = 0, h = shadowMap.get_height(); j < h; ++j)
	//	{
	//		shadowMap.set(i, j, clearColor);
	//	}
	//}

	//std::unique_ptr<float> shadowZBuf(new float[shadowMap.get_width() * shadowMap.get_height()]);
	//for (int i = 0, n = shadowMap.get_width() * shadowMap.get_height(); i < n; ++i) shadowZBuf.get()[i] = fltMax;
	//{
	//	//TestOrtho(shadowMap, shadowZBuf.get(), Vec3(-1.2f, -0.6f, 0.6f), Vec3(-0.6f, 0.6f, 0), Vec3(0, -0.6f, -0.6f), white);
	//	//TestOrtho(shadowMap, shadowZBuf.get(), Vec3(0.6f, 1.2f, 0), Vec3(0.f, .0f, 0.6f), Vec3(1.2f, 0.f, -0.6f), white);
	//	//Test_SelfShadow(defFB, zBuf.get(), shadowZBuf.get(), Vec3(-1.2f, -0.6f, 0.6f), Vec3(-0.6f, 0.6f, 0), Vec3(0, -0.6f, -0.6f), white);
	//	//Test_SelfShadow(defFB, zBuf.get(), shadowZBuf.get(), Vec3(0.6f, 1.2f, 0), Vec3(0.f, .0f, 0.6f), Vec3(1.2f, 0.f, -0.6f), white);
	//	for (auto f : mesh->faces) {
	//		auto v0 = mesh->vertices[f[0][0]];
	//		auto v1 = mesh->vertices[f[1][0]];
	//		auto v2 = mesh->vertices[f[2][0]];

	//		auto uv0 = mesh->uv[f[0][1]];
	//		auto uv1 = mesh->uv[f[1][1]];
	//		auto uv2 = mesh->uv[f[2][1]];

	//		auto n0 = mesh->normals[f[0][2]];
	//		auto n1 = mesh->normals[f[1][2]];
	//		auto n2 = mesh->normals[f[2][2]];
	//		DrawFilledTrianxgleBarycentricCoordinate_Texture_Ortho(shadowMap, shadowZBuf.get(), v0, v1, v2, white);
	//	}

	//	for (auto f : mesh->faces) {
	//		auto v0 = mesh->vertices[f[0][0]];
	//		auto v1 = mesh->vertices[f[1][0]];
	//		auto v2 = mesh->vertices[f[2][0]];

	//		auto uv0 = mesh->uv[f[0][1]];
	//		auto uv1 = mesh->uv[f[1][1]];
	//		auto uv2 = mesh->uv[f[2][1]];

	//		auto n0 = mesh->normals[f[0][2]];
	//		auto n1 = mesh->normals[f[1][2]];
	//		auto n2 = mesh->normals[f[2][2]];
	//		DrawFilledTrianxgleBarycentricCoordinate_Texture_SelfShadow(defFB, texture, normalMap, specMap, zBuf.get(), shadowZBuf.get(), v0, v1, v2, uv0, uv1, uv2, n0, n1, n2, white);
	//	}

	//}
	//
	//defFB.flip_vertically();
	//defFB.write_tga_file("output.tga");
	//shadowMap.flip_vertically();
	//shadowMap.write_tga_file("shadowMap.tga");

	return 0;
}


#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <string>
#include <memory>

#include "Header.h"
#include "Math/Vector.h"
#include "Mesh.h"
#include "MeshLoader.h"

static int screen_keys[512];	// 当前键盘按下状态

#ifdef UNICODE
static const wchar_t *WINDOW_CLASS_NAME = L"MySoftRenderer";
static const wchar_t *WINDOW_ENTRY_NAME = L"Entry";
#else
static const char *WINDOW_CLASS_NAME = "MySoftRenderer";
static const char *WINDOW_ENTRY_NAME = "Entry";
#endif

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
	class Time
	{
	public:
		static float Now()
		{
			static double period = -1;
			LARGE_INTEGER counter;
			if (period < 0) {
				LARGE_INTEGER frequency;
				QueryPerformanceFrequency(&frequency);
				period = 1 / (double)frequency.QuadPart;
			}
			QueryPerformanceCounter(&counter);
			return float(counter.QuadPart * period);
		}
	};

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
	
	struct Color32;
	struct Color : Vec4
	{
		Color(float v) :Vec4(v) {}
		Color(float r, float g, float b, float a) :Vec4(r,g,b,a) {}
		static const Color black, white, zero;
		operator Color32();
	};
	const Color Color::black = Color(0.f, 0.f,0.f,1.0f), Color::white = Color(1.0f), Color::zero = Color(.0f);

	struct Color;
	struct Color32 : Vec4uc
	{
		Color32(Byte v) :Vec4uc(v) {}
		Color32(Byte r, Byte g, Byte b, Byte a) :Vec4uc(r, g, b, a) {}
		static const Color32 black, white, zero;
		operator Color();
		
	};
	const Color32 Color32::black = Color32(0,0,0,1), Color32::white = Color32(1), Color32::zero = Color32(0);

	Color32::operator Color()
	{
		const float denominator = 1.f / 255;
		return Color(r * denominator, g * denominator, b * denominator, a * denominator);
	}

	Color::operator Color32()
	{
		UInt32 _r = int(0.5f + 255 * r); _r = _r > 255 ? 255 : _r;
		UInt32 _g = int(0.5f + 255 * g); _g = _g > 255 ? 255 : _g;
		UInt32 _b = int(0.5f + 255 * b); _b = _b > 255 ? 255 : _b;
		UInt32 _a = int(0.5f + 255 * a); _a = _a > 255 ? 255 : _a;
		return Color32(_r, _g, _b, _a);
	}

	class FrameBufferAttachment
	{
	public:
		enum class Format : Byte
		{
			RGB24,
			BGR24,
			ARGB32,
			BGRA32,
			RGBAHalf,
			Z8,
			ZHalf,
		};
	private:
		std::unique_ptr<Byte> buffer;
	public:
		const Format format;
		const int width;
		const int height;
		const int channels;
		FrameBufferAttachment(Format fmt, int w, int h) : format(fmt), width(w), height(h), channels(GetChannels(fmt)), buffer(new Byte[w*h*channels]) 
		{ }
		FrameBufferAttachment(Format fmt, int w, int h, Byte* raw) :format(fmt), width(w), height(h) , channels(GetChannels(fmt)), buffer(raw) 
		{ }
		void Clear(const Color32& c = Color32::black)
		{
			if (!buffer) return;
			if (width <= 0 || height <= 0) return;
			if (Format::ARGB32 == format || Format::BGRA32 == format)
			{
				int* ptr = reinterpret_cast<int*>(buffer.get());
				int chn = GetChannels(format);
				for (int i = 0, total = (width * height); i < total; ++i)
				{
					memcpy(ptr++, &c, chn);
				}
			}
		}
		
		void Assign(Byte* data){ std::unique_ptr<Byte>(data).swap(buffer); }

		static int GetChannels(Format fmt)
		{
			int n = 0;
			switch (fmt)
			{
			case Format::ARGB32:
			case Format::BGRA32:
			{
				n = 4;
				break;
			}
			}
			return n;
		}
		FrameBufferAttachment() = delete;
	};
	typedef FrameBufferAttachment::Format FBFormat;

	class FrameBuffer
	{
	public:
		FrameBufferAttachment colorBuf;
		FrameBufferAttachment depthBuf;

		FrameBuffer(FBFormat colorBufFmt, int w0, int h0, FBFormat depthBufFmt, int w1, int h1)
			:colorBuf(colorBufFmt, w0, h0), depthBuf(depthBufFmt, w1, h1)
		{}

		FrameBuffer(FBFormat colorBufFmt, int w0, int h0, Byte* colorData, FBFormat depthBufFmt, int w1, int h1, Byte* depthData)
			:colorBuf(colorBufFmt, w0, h0, colorData), depthBuf(depthBufFmt, w1, h1, depthData)
		{}

		void Clear(const Color32& c = Color32::black)
		{
			colorBuf.Clear();
			depthBuf.Clear();
		}
		FrameBuffer() = delete;
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

	class Screen
	{
	public:
		static const int GetWidth() { return GetSystemMetrics(SM_CXSCREEN); }
		static const int GetHeight() { return GetSystemMetrics(SM_CYSCREEN); }
	};

	class RenderTarget
	{
	public:
		static std::shared_ptr<RenderTarget> active;
		std::unique_ptr<FrameBuffer> frameBuffer;
	};
	std::shared_ptr<RenderTarget> RenderTarget::active;


	template<typename T>
	class Singleton
	{};

	class Window:public Singleton<Window>
	{
	private:
		int width;
		int height;
		bool isTopDown;
		HWND handle = NULL;
		HDC wndDC = NULL;
		HGDIOBJ hgdi = NULL;
		WNDCLASS wndClass;
		std::shared_ptr<FrameBuffer> frameBuffer;
		bool bInit;
		bool bExit;
		Window():bInit(false), bExit(false) {}
	public:
#ifdef UNICODE
		static constexpr const wchar_t *DEFAULT_WINDOW_CLASS_NAME = L"MySoftRenderer";
		static constexpr const wchar_t* DEFAULT_WINDOW_TILE = "Hello!";
#else
		static constexpr const char* DEFAULT_WINDOW_CLASS_NAME = "MySoftRenderer";
		static constexpr const char* DEFAULT_WINDOW_TILE = "Hello!";
#endif
		// GetBackBuffer不一定成功，这时候应该返回什么
		// 用智能指针
		// 希望渲染过程不用检查target
		inline std::shared_ptr<FrameBuffer> GetBackBuffer() { return frameBuffer; }

		static Window& GetInstance()
		{
			static Window* inst = new Window;
			return *inst;
		}

		void Update(float delta) {
			// Blit
			if (!bInit) return;
			HDC hDC = GetDC(handle);
			BitBlt(hDC, 0, 0, width, height, wndDC, 0, 0, SRCCOPY);
			ReleaseDC(handle, hDC);
		}

		void Dispatch()
		{
			// Dispatch messages
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		void Destroy() { 
			if(handle) ShowWindow(handle, SW_HIDE);
			if (wndDC) DeleteDC(wndDC);
			if (hgdi) DeleteObject(hgdi);
			if (handle) DestroyWindow(handle);
		}

		//	WNDCLASS-> CreateWindow ->handle
		//		handle-> GetDC ->hDC
		//			hDC-> CreateCompatibleDC ->compatibleHDC
		//				compatibleHDC,BITMAPINFO-> CreateDIBSection ->hBmp, screenBuf
		//					hBmp,compatibleHDC-> SelectObject ->hGDIObj
		//	hDC,compatibleHDC-> BitBlt
		int Init(int w, int h, bool topDown = true)
		{
			if (bInit) return 0;
			width = w;
			height = h;
			isTopDown = topDown;
			wndClass.style = CS_BYTEALIGNCLIENT;
			wndClass.lpfnWndProc = MessageLoop;
			wndClass.cbClsExtra = 0;
			wndClass.cbWndExtra = 0;
			wndClass.hInstance = GetModuleHandle(NULL);
			wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
			wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
			wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
			wndClass.lpszMenuName = NULL;
			wndClass.lpszClassName = DEFAULT_WINDOW_CLASS_NAME;

			if (!RegisterClass(&wndClass))
			{
				XLogWarning("RegisterClass failed.");
				return -1;
			}

			const TCHAR *title = DEFAULT_WINDOW_TILE;
			auto wndHandle = CreateWindow(
				wndClass.lpszClassName, 
				title,
				WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
				0, 0, 0, 0, NULL, NULL, 
				wndClass.hInstance, 
				NULL);
			if (wndHandle == NULL)
			{
				XLogWarning("CreateWindow failed.");
				return -1;
			}
			handle = wndHandle;

			auto hDC = GetDC(wndHandle);
			auto compatibleHDC = CreateCompatibleDC(hDC);
			ReleaseDC(wndHandle, hDC);
			wndDC = compatibleHDC;
			
			frameBuffer = std::unique_ptr<FrameBuffer>(new FrameBuffer(FBFormat::ARGB32, w, h, nullptr, FBFormat::Z8, w, h, nullptr));
			int ch = frameBuffer->colorBuf.channels;
			BITMAPINFO bi = {
				{
					sizeof(BITMAPINFOHEADER),
					w, (isTopDown ? -1 : 1) * h,
					1,
					ch * sizeof(Byte) * 8,
					BI_RGB,
					w * h * ch,
					0, 0, 0, 0
				}
			};

			LPVOID ptr;
			auto hBmp = CreateDIBSection(compatibleHDC, &bi, DIB_RGB_COLORS, &ptr, 0, 0);
			if (hBmp == NULL)
			{
				XLogWarning("CreateDIBSection failed.");
				return -1;
			}
			auto hGDIObj = (HBITMAP)SelectObject(compatibleHDC, hBmp);
			hgdi = hGDIObj;

			auto screenBuf = (Byte*)ptr;
			if (screenBuf == NULL)
			{
				XLogWarning("Failed to get buffer.");
				return -1;
			}

			RECT rect = { 0, 0, w, h };
			AdjustWindowRect(&rect, GetWindowLong(wndHandle, GWL_STYLE), 0);
			int wndW = rect.right - rect.left;
			int wndH = rect.bottom - rect.top;
			int posX = (Screen::GetWidth() - wndW) / 2;
			int posY = (Screen::GetHeight() - wndH) / 2;
			if (posY < 0) posY = 0;

			SetWindowPos(wndHandle, NULL, posX, posY, wndW, wndH, (SWP_NOCOPYBITS | SWP_NOZORDER | SWP_SHOWWINDOW));
			SetForegroundWindow(wndHandle);
			ShowWindow(wndHandle, SW_NORMAL);
			Dispatch();

			memset(screenBuf, 0, w * h );
			frameBuffer->colorBuf.Assign(screenBuf);
			bInit = true;
			return 0;
		}

		bool ShouldExit() { return bExit; }

		static LRESULT __stdcall MessageLoop(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			//WM_LBUTTONDOWN
			//WM_RBUTTONDOWN
			//WM_LBUTTONUP
			//WM_RBUTTONUP
			//WM_MOUSEWHEEL
			//float offset = GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
			switch (msg) {
			case WM_CLOSE: { GetInstance().bExit = true; break; }
			case WM_KEYDOWN:/* screen_keys[wParam & 511] = 1;*/ break;
			case WM_KEYUP: /*screen_keys[wParam & 511] = 0;*/ break;
			default: return DefWindowProc(hWnd, msg, wParam, lParam);
			}
			return 0;
		}
	};

}


static int gWidth = 800;
static int gHeight = 600;
int main()
{
	int w = gWidth, h = gHeight;
	auto& wnd = SR::Window::GetInstance();
	wnd.Init(w, h);
	auto backBuf = wnd.GetBackBuffer();
	if (backBuf)
	{
		backBuf->Clear();
	}
	//SR::RenderTarget::active = backBuf;
	//memset(screen_keys, 0, sizeof(int) * 512);

	/*foreach pluginMgr.plugins.Init();
	while (!window->IsExit()) {
		inputsystem->Update();
		render
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

		wnd.Update(deltaTime);
		if ((now - prevTimeMark) > 1)
		{
			std::cout << "FPS:" << (elapseFrameCnt / elapseTime) << std::endl;
			prevTimeMark = now;
			elapseTime = 0;
			elapseFrameCnt = 0;
		}
		wnd.Dispatch();
		//Sleep(1);
	}
	wnd.Destroy();

	//std::shared_ptr<Mesh> floor(MeshLoader::Instance().Load("res/floor/floor.obj"));
	//float fltMax = std::numeric_limits<float>::max();

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
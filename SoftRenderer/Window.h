#pragma once
#include <Windows.h>
#include <memory>

#include "Header.h"
#include "Singleton.h"
#include "FrameBuffer.h"
#include "Screen.h"
namespace SR
{
	class Window :public Singleton<Window>
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
		Window() :bInit(false), bExit(false) {}
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
			if (handle) ShowWindow(handle, SW_HIDE);
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
		int Init(int w, int h, bool topDown = true);


		bool ShouldExit() { return bExit; }

		static LRESULT __stdcall MessageLoop(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	};

}

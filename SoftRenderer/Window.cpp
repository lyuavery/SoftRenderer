#include "Window.h"
#include "Log.h"

LRESULT __stdcall SR::Window::MessageLoop(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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

int SR::Window::Init(int w, int h, bool topDown)
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

	frameBuffer = std::unique_ptr<FrameBuffer>(new FrameBuffer(SR::FrameBufferAttachmentFormat::BGRA32, w, h, nullptr, SR::FrameBufferAttachmentFormat::Depth8, w, h, nullptr));
	int bpp = frameBuffer->colorBuf.GetBytesPerPixel();
	BITMAPINFO bi = {
		{
			sizeof(BITMAPINFOHEADER),
			w, (isTopDown ? -1 : 1) * h,
			1,
			bpp * sizeof(Byte) * 8,
			BI_RGB,
			w * h * bpp,
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
	
	memset(screenBuf, 0, w * h * bpp);
	frameBuffer->colorBuf.Assign(screenBuf);
	frameBuffer->depthBuf.Assign(new Byte[w * h]);
	bInit = true;
	return 0;
}
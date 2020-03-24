

#include <windows.h>
#include <tchar.h>
#include <iostream>
static LRESULT WndProc(HWND hWnd, UINT msg,
	WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CLOSE:  break;
	case WM_KEYDOWN:/* screen_keys[wParam & 511] = 1;*/ break;
	case WM_KEYUP: /*screen_keys[wParam & 511] = 0;*/ break;
	default: return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}
static int gScreenWidth = 800;
static int gScreenHeight = 600;
static int screen_keys[512];	// 当前键盘按下状态

#ifdef UNICODE
static const wchar_t *WINDOW_CLASS_NAME = L"MySoftRenderer";
static const wchar_t *WINDOW_ENTRY_NAME = L"Entry";
#else
static const char *WINDOW_CLASS_NAME = "MySoftRenderer";
static const char *WINDOW_ENTRY_NAME = "Entry";
#endif

#ifdef _MSC_VER
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#endif

int main()
{
	int w = gScreenWidth, h = gScreenHeight;
	auto screen_events = WndProc;

	WNDCLASS wc = { CS_BYTEALIGNCLIENT, (WNDPROC)screen_events, 0, 0, 0,
		NULL, NULL, NULL, NULL, WINDOW_CLASS_NAME };
	BITMAPINFO bi = { { sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB,
		w * h * 4, 0, 0, 0, 0 } };
	RECT rect = { 0, 0, w, h };
	int wx, wy, sx, sy;
	LPVOID ptr;
	HDC hDC;
	
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.hInstance = GetModuleHandle(NULL);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	if (!RegisterClass(&wc)) return -1;

	const TCHAR *title = _T("Hello!");
	
	auto screen_handle = CreateWindow(WINDOW_CLASS_NAME, title,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);
	if (screen_handle == NULL) return -2;

	hDC = GetDC(screen_handle);
	auto screen_dc = CreateCompatibleDC(hDC);
	ReleaseDC(screen_handle, hDC);

	auto screen_hb = CreateDIBSection(screen_dc, &bi, DIB_RGB_COLORS, &ptr, 0, 0);
	if (screen_hb == NULL) return -3;

	auto screen_ob = (HBITMAP)SelectObject(screen_dc, screen_hb);
	auto screen_fb = (unsigned char*)ptr;
	auto screen_w = w;
	auto screen_h = h;
	auto screen_pitch = w * 4;

	AdjustWindowRect(&rect, GetWindowLong(screen_handle, GWL_STYLE), 0);
	wx = rect.right - rect.left;
	wy = rect.bottom - rect.top;
	sx = (GetSystemMetrics(SM_CXSCREEN) - wx) / 2;
	sy = (GetSystemMetrics(SM_CYSCREEN) - wy) / 2;
	if (sy < 0) sy = 0;
	SetWindowPos(screen_handle, NULL, sx, sy, wx, wy, (SWP_NOCOPYBITS | SWP_NOZORDER | SWP_SHOWWINDOW));
	SetForegroundWindow(screen_handle);

	ShowWindow(screen_handle, SW_NORMAL);

	//screen_dispatch();
	MSG msg;
	while (1) {
		if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) break;
		if (!GetMessage(&msg, NULL, 0, 0)) break;
		DispatchMessage(&msg);
	}

	memset(screen_keys, 0, sizeof(int) * 512);
	memset(screen_fb, 0, w * h * 4);
	unsigned int* dst = reinterpret_cast<unsigned int*>(screen_fb) ;
	for (int i = 0, n = gScreenHeight * gScreenWidth; i < n; ++i)
	{
		dst[i] = 0x0000FF00;
	}
	/*screen_fb[(gScreenHeight / 2) * (gScreenWidth / 2) + (gScreenHeight / 2)] = 0xFFFFFFFF;
	screen_fb[(gScreenHeight / 2) * (gScreenWidth / 2) + (gScreenHeight / 2) + 4] = 0xFFFFFFFF;
	screen_fb[(gScreenHeight / 2 + 1) * (gScreenWidth / 2) + (gScreenHeight / 2)] = 0xFFFFFFFF;
	screen_fb[(gScreenHeight / 2 + 1) * (gScreenWidth / 2) + (gScreenHeight / 2) + 4] = 0xFFFFFFFF;*/
	//HDC hDC = GetDC(screen_handle);
	hDC = GetDC(screen_handle);
	BitBlt(hDC, 0, 0, screen_w, screen_h, screen_dc, 0, 0, SRCCOPY);
	ReleaseDC(screen_handle, hDC);
	getchar();
	return 0;
}
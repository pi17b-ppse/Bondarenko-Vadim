
#endif
static int put_form(HINSTANCE hinst, LPCWSTR cname, LPCWSTR cap, int width, int height, UINT id);
static tetris*    g_app = NULL;
static HINSTANCE g_inst = NULL;
static void (ARGS2_FASTCALL tetris::*g_create)(HINSTANCE, HWND);
static void (ARGS2_FASTCALL tetris::*g_kdown)(HWND, WORD);
static void (ARGS2_FASTCALL tetris::*g_destroy)(HWND);
static void (tetris::*g_mdown)(HWND, int, int);
static void (tetris::*g_mmove)(HWND, int, int);
static void (ARGS2_FASTCALL tetris::*g_timer)(HWND, UINT);
static void (ARGS2_FASTCALL tetris::*g_paint)(HWND, HDC);


//точка входа
int WINAPI WinMain(HINSTANCE hinst, HINSTANCE, LPSTR, int){
	const WCHAR app[] = {0x422, 0x435, 0x442, 0x440, 0x438, 0x441, 0x0 };

	g_inst    = hinst;
	g_app     = new tetris();
	g_create  = &tetris::onCreate;
	g_kdown   = &tetris::onKeyDown;
	g_destroy = &tetris::onDestroy;
	g_timer   = &tetris::onTimer;
	g_paint   = &tetris::onPaint;
	g_mdown   = &tetris::onMouseDown;
	g_mmove   = &tetris::onMouseMove;
	int   ret = put_form(hinst, L"TETRIS", app, 681, 600, IDI_ICON_APP);
	delete g_app;
	return ret;
}


//------------------------------------------------------------------------------------------------------------


//oбработчик
static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	switch(msg){
	case WM_CREATE:
		(g_app->*g_create)(g_inst, hwnd);
		break;
	case WM_KEYDOWN:
		(g_app->*g_kdown)(hwnd, static_cast<WORD>(wParam));
		break;
	case WM_LBUTTONDOWN:
		(g_app->*g_mdown)(hwnd, static_cast<int>(LOWORD(lParam)), static_cast<int>(HIWORD(lParam)));
		break;
	case WM_MOUSEMOVE:
		(g_app->*g_mmove)(hwnd, static_cast<int>(LOWORD(lParam)), static_cast<int>(HIWORD(lParam)));
		break;
	case WM_TIMER:
		(g_app->*g_timer)(hwnd, static_cast<UINT>(LOWORD(wParam)));
		break;
	case WM_ERASEBKGND:
		(g_app->*g_paint)(hwnd, (HDC)wParam);
		return 1;
	case WM_DESTROY:
		(g_app->*g_destroy)(hwnd);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProcW(hwnd, msg, wParam, lParam);
	}
	return 0;
}


//создание окна
static int put_form(HINSTANCE hinst, LPCWSTR cname, LPCWSTR cap, int width, int height, UINT id){
	WNDCLASSEXW cls = {0};
	cls.cbSize        = sizeof(cls);
	cls.lpfnWndProc   = reinterpret_cast<WNDPROC>(wnd_proc);
	cls.hInstance     = hinst;
	cls.hCursor       = LoadCursor(NULL, IDC_ARROW);
	cls.lpszClassName = cname;
	cls.hIcon         = LoadIconW(hinst, MAKEINTRESOURCEW(id));
	cls.hIconSm       = cls.hIcon;

	if(!RegisterClassExW(&cls))
		return 1;

	DWORD sty = WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_SIZEBOX);
	RECT  rc  = { 0, 0, width, height };
	AdjustWindowRectEx(&rc, sty, FALSE, 0);
	int cx = rc.right  - rc.left;
	int cy = rc.bottom - rc.top;

	HWND hwnd = CreateWindowExW(0, cname, cap, sty, (GetSystemMetrics(SM_CXSCREEN) - cx)/2, 
	                            (GetSystemMetrics(SM_CYSCREEN) - cy)/2, cx, cy, NULL, NULL, hinst, NULL);
	if(hwnd == NULL){
		UnregisterClassW(cname, hinst);
		return 1;
	}
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	MSG msg;
	while(GetMessageW(&msg, NULL, 0, 0)){
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	UnregisterClassW(cname, hinst);
	return 0;
}


#include "Window.h"
#include "IncludeWin.h"
#include "App.h"


//User created procedure
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	try{
		return App{}.Go();
	} catch(const UrielException& e){
		MessageBox(nullptr, e.what(),e.GetType(), MB_OK | MB_ICONEXCLAMATION);
	} catch(const exception& e) {
		MessageBox(nullptr, e.what(), "Standard Exception", MB_OK | MB_ICONEXCLAMATION);
	} catch(...) {
		MessageBox(nullptr, "No details available", "Unknown Exception", MB_OK | MB_ICONEXCLAMATION);
	}
	return -1;

	/*switch (msg) {
		case WM_CLOSE:
			PostQuitMessage(69);
			break;
		case WM_KEYDOWN:
			if (wParam == 'F') {
				SetWindowText(hWnd, "Respects");
			}
			break;
		case WM_KEYUP:
			if (wParam == 'F') {
				SetWindowText(hWnd, "No Respects");
			}
			break;
		case WM_CHAR:
			{
			static std::string title;
			title.push_back((char)wParam);
			SetWindowText(hWnd, title.c_str());
			}
			break;
		case WM_LBUTTONDOWN:
			{
			const POINTS pt = MAKEPOINTS(lParam);
			std::ostringstream oss;
			oss << "(" << pt.x << ", " << pt.y << ")";
			SetWindowText(hWnd, oss.str().c_str());
			}
			break;

	}
	return DefWindowProc(hWnd, msg, wParam, lParam);   */
}


/*
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	
	const auto pClassName = "hw3d";

	//Register window class
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WndProc; // this Default procedure will not really work for games, so we should use our own
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = nullptr;
	wc.hCursor = nullptr;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = pClassName;
	wc.hIconSm = nullptr;
	RegisterClassEx(&wc);

	//Create Window instance
	HWND hWnd = CreateWindowEx(
		0, 
		pClassName,
		"First Window", 
		(WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX), 
		200, 200,
		640, 480, 
		nullptr, 
		nullptr, 
		hInstance,
		nullptr
	);

	//Show the created window
	ShowWindow(hWnd, SW_SHOW);

	//Message pump
	MSG msg;
	BOOL gResult;
	while ((gResult = GetMessage(&msg, nullptr, 0, 0)) > 0) {
		
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (gResult == -1) {
		return -1;
	}else{
		return msg.wParam;
	}

}*/
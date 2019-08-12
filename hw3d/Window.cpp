#include "Window.h"
#include "resource1.h"
#include <sstream>


//WindowClass class functions *******************************************
Window::WindowClass Window::WindowClass::wndClass; //Singleton

Window::WindowClass::WindowClass() noexcept : hInst(GetModuleHandle(nullptr)) { //constructor
	WNDCLASSEX wc = {0};
		wc.cbSize = sizeof(wc);
		wc.style = CS_OWNDC;
		wc.lpfnWndProc = HandleMsgSetup;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hIcon = static_cast<HICON>(LoadImage(GetInstance(), MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 32,32, 0)); //MUST be a .ico file
		wc.hCursor = nullptr;
		wc.hbrBackground = nullptr;
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = GetName();;
		wc.hInstance = GetInstance();
		wc.hIconSm = static_cast<HICON>(LoadImage(GetInstance(), MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 16, 16, 0)); //MUST be a .ico file

	RegisterClassEx(&wc);
}

Window::WindowClass::~WindowClass() {
	UnregisterClass(wndClassName, GetInstance());
}

const char* Window::WindowClass::GetName() noexcept {
	return wndClassName;
}

HINSTANCE Window::WindowClass::GetInstance() noexcept {
	return wndClass.hInst;
}

//Window Exception class functions *******************************************
Window::Exception::Exception(int line, const char* file, HRESULT hr) noexcept
	: UrielException(line, file), hr(hr){
}

const char* Window::Exception::what() const noexcept {
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] " << GetErrorCode() << std::endl
		<< "[Description] " << GetErrorString() << std::endl
		<< GetOriginalString();
	whatBuffer = oss.str(); //passes string output to buffer because string stream is deleted automatically after this function
	return whatBuffer.c_str();
}

const char* Window::Exception::GetType() const noexcept {
	return "Uriel Window Exception";
}

std::string Window::Exception::TranslateErrorCode(HRESULT hr) noexcept {
	char* pMsgBuf = nullptr;
	DWORD nMsgLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |  //lenght of error string
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
		reinterpret_cast<LPSTR>(&pMsgBuf),0,nullptr);
	if(nMsgLen == 0) {
		return "Unidentified error code";
	}
	std::string errorString = pMsgBuf;
	LocalFree(pMsgBuf);
	return errorString;
}

HRESULT Window::Exception::GetErrorCode() const noexcept {
	return hr;
}

std::string Window::Exception::GetErrorString() const noexcept {
	return TranslateErrorCode(hr);
}

//Window class functions *******************************************
Window::Window(int width, int height, const char* name) /*noexcept*/ {
	//calculate window size based on desired client region(CR) size
	RECT wr;
	wr.left = 100;
	wr.top = 100;
	wr.right = width + wr.left;
	wr.bottom = height + wr.top;
	if(AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE) == 0) {  //RECT pLocation, window style, menu
		throw CHWND_LAST_EXCEPT();
	} 

	//throw CHWND_EXCEPT(ERROR_ARENA_TRASHED);

	//Create window and hWnd
	hWnd = CreateWindow(
		WindowClass::GetName(), name, //class name, window name
		WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, //window style
		CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, //starting window position x, y, width, height
		nullptr, nullptr, WindowClass::GetInstance(), this //parent, menu, instance, long pointer parameter(pointer to 'this' Window class)
	);
	//error check
	if(hWnd == nullptr) {
		throw CHWND_LAST_EXCEPT();
	}

	//Newly created windows start off as hidden
	ShowWindow(hWnd, SW_SHOWDEFAULT);

	//Create graphics object
	pGfx = std::make_unique<Graphics>(hWnd);
}

Window::~Window() {
	DestroyWindow(hWnd);
}

void Window::SetTitle(const std::string& title) {
	if(SetWindowText(hWnd, title.c_str()) == 0) {
		throw CHWND_LAST_EXCEPT();
	}
}

std::optional<int> Window::ProcessMessages() {
	//Message pump
	MSG msg;

	while(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		//check for quit because PeekMessage does not signal this via return
		if(msg.message == WM_QUIT) {
			return msg.wParam; //wParam is the passed value to PostQuitMessage
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return {}; //return empty optional when not quitting app
}

Graphics& Window::Gfx()
{
	return *pGfx;
}

//Setup procedure LONG POINTER
LRESULT CALLBACK Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
	//use create parameter passed in from CreateWindow() to store window class pointer
	if(msg == WM_NCCREATE) {
		//extract ptr to window class from creation data
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
		Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);
		//set WINAPI-managed user data to store ptr to window class
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));

		//set message proc to normal (non-setup) handler now that setup is finished
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk));
		//forward message to window class handler
		return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
	}
	//if we get a message before the WM_CREATE message, handle using the default handler
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

//Thunk procedure LONG POINTER
LRESULT CALLBACK Window::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
	//retrieve ptr to window class
	Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	//forward message to window class handler
	return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
}

//Message output procedure LONG POINTER
LRESULT Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
	switch(msg) {
		case WM_CLOSE:
			PostQuitMessage(0);
			return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}
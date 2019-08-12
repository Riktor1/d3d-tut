#pragma once
#include "IncludeWin.h"
#include "UrielException.h"
#include "Graphics.h"
#include <optional>
#include <memory>

//Creates and destroys window and handles messages
class Window {
public:
	//Exception handler
	class Exception : public UrielException{
	public:
		Exception(int line, const char* file, HRESULT hr) noexcept;
		const char* what() const noexcept override;
		virtual const char* GetType() const noexcept;
		static std::string TranslateErrorCode(HRESULT hr) noexcept;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorString() const noexcept;
	private:
		HRESULT hr;
	};
private:
	//Singleton method
	class WindowClass {
	public:
		static const  char* GetName() noexcept;
		static HINSTANCE GetInstance() noexcept;
		//Copy constuctor and assingment operator can remain public, but must be deleted if public to prevent cloning***********
		WindowClass(const WindowClass&) = delete; //copy constructor
		WindowClass& operator=(const WindowClass&) = delete; //copy assignment operator

	private: //make all constructors private for singleton method so only 1 instance is ever available
		WindowClass() noexcept; //constructor
		~WindowClass() noexcept; //destructor
		static constexpr const char* wndClassName = "Uriel's Direct3D Engine Window";
		static WindowClass wndClass;
		HINSTANCE hInst;
	};

public:
	Window(int width, int length, const char* name) /*noexcept*/;
	~Window();
	//Copy constuctor and assingment operator can remain public, but must be deleted if public to prevent cloning***********
	Window(const Window&) = delete; //copy constructor
	Window& operator=(const Window&) = delete; //copy assingment operator
	void SetTitle(const std::string& title);
	static std::optional<int> ProcessMessages();
	Graphics& Gfx(); //graphics accessor

public:
	//Graphics gfx;
private:
	//3 procedures to pass info using a single function HandleMsg
	static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

	int width;
	int height;
	HWND hWnd;
	std::unique_ptr<Graphics> pGfx;
};

//error exception helper macro
#define CHWND_EXCEPT(hr) Window::Exception(__LINE__, __FILE__, hr)
#define CHWND_LAST_EXCEPT() Window::Exception(__LINE__, __FILE__, GetLastError());
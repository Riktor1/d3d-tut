#pragma once
#include "IncludeWin.h"
#include "UrielException.h"
#include "Graphics.h"
//#include "dxerr.h"
//#include "GraphicsThrowMacros.h"
#include "WindowsThrowMacors.h"
#include <optional>
#include <memory>

//Creates and destroys window and handles messages
class Window {
public:
	//Exception handler BASE class
	class Exception : public UrielException {
		using UrielException::UrielException;
	public:
		static std::string TranslateErrorCode(HRESULT hr) noexcept;
	};
	//HrException handler DERIVED class
	class HrException : public Exception {
	public:
		HrException(int line, const char* file, HRESULT hr) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorDescription() const noexcept;
	private:
		HRESULT hr;
	};
	//NoGfxException handler DERIVED class
	class NoGfxException : public Exception {
	public:
		using Exception::Exception;
		const char* GetType() const noexcept override;
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


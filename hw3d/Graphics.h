#pragma once
#include "IncludeWin.h"
#include "UrielException.h"
#include "GraphicsThrowMacros.h"
#include "DxgiInfoManager.h"
#include <sstream>
#include <wrl.h>
#include <vector>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <memory>



class Graphics {
public:
	class Exception : public UrielException {
		using UrielException::UrielException;
	};
	//Debug exceptions
	class HrException : public Exception {
	public:
		HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs = {}) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorString() const noexcept;
		std::string GetErrorDescription() const noexcept;
		std::string GetErrorInfo() const noexcept;
	private:
		HRESULT hr;
		std::string info;
	};
	//Info Exception
	class InfoException : public Exception{
	public:
		InfoException(int line, const char* file, std::vector<std::string> infoMsgs = {}) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		std::string GetErrorInfo() const noexcept;
	private:
		std::string info;
	};
	//Device removed exception
	class DeviceRemovedException : public HrException {
		using HrException::HrException;
	public:
		const char* GetType() const noexcept override;
	private:
		std::string reason;
	};

	Graphics(HWND hWnd);
	Graphics(const Graphics&) = delete; //delete copy constructor and assignment
	Graphics& operator=(const Graphics&) = delete;
	~Graphics() = default;
	void EndFrame();
	void ClearBuffer(float red, float green, float blue) noexcept;
	void DrawTestTriangle();
private:
	//ID3D11Device* pDevice = nullptr;
	//IDXGISwapChain* pSwap = nullptr;
	//ID3D11DeviceContext* pContext = nullptr;
	//ID3D11RenderTargetView* pTarget = nullptr;
#ifndef NDEBUG
	DxgiInfoManager infoManager;
#endif
	Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
	Microsoft::WRL::ComPtr<IDXGISwapChain> pSwap;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTarget;
};
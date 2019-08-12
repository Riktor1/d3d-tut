#pragma once
#include "IncludeWin.h"
#include "UrielException.h"
#include "GraphicsThrowMacros.h"
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
		HrException(int line, const char* file, HRESULT hr) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorString() const noexcept;
		std::string GetErrorDescription() const noexcept;
	private:
		HRESULT hr;
	};
	//Device removed exception
	class DeviceRemovedException : public HrException {
		using HrException::HrException;
	public:
		const char* GetType() const noexcept override;
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
	Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
	Microsoft::WRL::ComPtr<IDXGISwapChain> pSwap;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTarget;
};
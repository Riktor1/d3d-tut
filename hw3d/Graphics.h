#pragma once
#include "IncludeWin.h"
#include <wrl.h>
#include <vector>
//#include "Window.h"
#include <d3d11.h>

class Graphics {
public:
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
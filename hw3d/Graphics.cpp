#include "Graphics.h"
#include "dxerr.h"
#include <sstream>
#include <cmath>
#include <DirectXMath.h>
#include <d3dcompiler.h>

namespace wrl = Microsoft::WRL; //an alias for the namespace. *DO NOT use global namepaces in header files*

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

#define GFX_THROW_FAILED(hrcall) if(FAILED(hr = (hrcall))) throw Graphics::HrException(__LINE__,__FILE__, hr);
#define GFX_DEVICE_REMOVED_EXCEPT(hr) Graphics::DeviceRemovedException(__LINE__,__FILE__, (hr));

Graphics::Graphics(HWND hWnd){

	//Swap chain descriptor (Desriptors are very common in D3D)
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferDesc.Width = 0; //set to 0 means look at hte window and figure it out
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 0; //set refresh rate to whatever is already being used
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; //width and height not specified, then dont specify scaling
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.SampleDesc.Count = 1; //These bits set anti-aliasing but we dont want it right now
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //we want the buffer to be where the stuff is rendered so set it as the render target
	sd.BufferCount = 1; //amount of buffers
	sd.OutputWindow = hWnd;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	HRESULT hr; //used for debug error checking

	//create device and front/back buffers, and swap chain and rendering context
	GFX_THROW_FAILED(D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D11_CREATE_DEVICE_DEBUG,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&sd,       //These AddressOf operators (&) have been overload by the smart pointer, ComPtr, 
		&pSwap,    //to Release() and GetAddressOf(), so be careful to not manually release again
		&pDevice,
		nullptr,
		&pContext
	));

	//gain access to texture subresource in swap chain(back buffer)
	//ID3D11Resource* pBackBuffer = nullptr;
	wrl::ComPtr<ID3D11Resource> pBackBuffer;
	GFX_THROW_FAILED(pSwap->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer));
	GFX_THROW_FAILED(pDevice->CreateRenderTargetView(
		pBackBuffer.Get(),
		nullptr,
		&pTarget
	));
	pBackBuffer->Release();
}

//Graphics::~Graphics() {
//	//Try and release Children before Parent, but doesn't matter though
//	if(pTarget != nullptr) {
//		pTarget->Release();
//	}
//	if(pContext != nullptr) {
//		pContext->Release();
//	}
//	if(pSwap != nullptr) {
//		pSwap->Release();
//	}
//	if(pDevice != nullptr) {
//		pDevice->Release();
//	}
//}

void Graphics::EndFrame() {
	HRESULT hr;
	if(FAILED(hr = pSwap->Present(1u, 0u))) { //present frame to buffer
		if(hr == DXGI_ERROR_DEVICE_REMOVED) {
			throw GFX_DEVICE_REMOVED_EXCEPT(pDevice->GetDeviceRemovedReason());
		}else {
			GFX_THROW_FAILED(hr);
		}
	}
}

void Graphics::ClearBuffer(float red, float green, float blue) noexcept {
	const float color[] = { red, green, blue, 1.0f };
	pContext->ClearRenderTargetView(pTarget.Get(), color);
}

//Graphics Exceptions
Graphics::HrException::HrException(int line, const char* file, HRESULT hr) noexcept
	: Exception(line, file), hr(hr)
{
}

const char* Graphics::HrException::what() const noexcept {
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Error String]" << GetErrorString() << std::endl
		<< "[Description]" << GetErrorDescription() << std::endl 
		<< GetOriginalString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::HrException::GetType() const noexcept {
	return "Uriel Graphics Exception";
}

HRESULT Graphics::HrException::GetErrorCode() const noexcept {
	return hr;
}

std::string Graphics::HrException::GetErrorString() const noexcept {
	return DXGetErrorString(hr);
}

std::string Graphics::HrException::GetErrorDescription() const noexcept {
	char buff[512];
	DXGetErrorDescription(hr, buff, sizeof(buff));
	return buff;
}
const char* Graphics::DeviceRemovedException::GetType() const noexcept {
	return "Uriel Graphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
}

void Graphics::DrawTestTriangle() { //pDevice creates stuff and pContext issues commands
	namespace wrl = Microsoft::WRL;

	struct Vertex {
		float x;
		float y;
	};

	//create vertex buffer (one 2D triangle at center of screen)
	const Vertex vertices[] = {
		{0.0f, 0.5f},
		{0.5f, -0.5f},
		{-0.5f, -0.5f},
	};

	wrl::ComPtr<ID3D11Buffer> pVertextBuffer;

	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = sizeof(vertices);
	bd.StructureByteStride = sizeof(Vertex);
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0u;
	bd.MiscFlags = 0u;

	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = vertices;

	//Create buffer
	pDevice->CreateBuffer(&bd, &sd, &pVertextBuffer);

	//bind vertex buffer to pipeline
	const UINT stride = sizeof(Vertex);
	const UINT offset = 0u;
	pContext->IASetVertexBuffers(0u, 1u, pVertextBuffer.GetAddressOf(), &stride, &offset);

	//create pixel shader
	wrl::ComPtr<ID3D11PixelShader> pPixelShader;
	wrl::ComPtr<ID3DBlob> pBlob;
	D3DReadFileToBlob(L"PixelShader.cso", &pBlob);
	pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader);
	//bind pixel shader
	pContext->PSGetShader(&pPixelShader, nullptr, 0u);

	//create vertex shader
	wrl::ComPtr<ID3D11VertexShader> pVertexShader;
	D3DReadFileToBlob(L"VertexShader.cso", &pBlob); //make sure shader file output path in the compiler is set to the ProjectDirectory instead of OutputDirectory
	pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader);
	//bind vertex shader
	pContext->VSSetShader(pVertexShader.Get(), nullptr, 0u); //remember that to get the single-pointer address of a ComPtr you need to use .Get()

	//input (vertex) layout (2D position only)
	wrl::ComPtr<ID3D11InputLayout> pInputLayout;
	const D3D11_INPUT_ELEMENT_DESC ied[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	//create Input Layout
	pDevice->CreateInputLayout(ied, (UINT)std::size(ied), pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pInputLayout);
	//bind input layout
	pContext->IASetInputLayout(pInputLayout.Get());

	//bind render target
	pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), nullptr);

	//Set triangle topology list
	pContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//configure viewport
	D3D11_VIEWPORT vp;
	vp.Width = 800;
	vp.Height = 600;
	vp.MaxDepth = 1;
	vp.MinDepth = 0;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pContext->RSSetViewports(1u, &vp);

	pContext->Draw((UINT)std::size(vertices), 0u);
}


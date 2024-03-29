#include "Graphics.h"
#include "dxerr.h"
#include <sstream>
#include <cmath>
#include <DirectXMath.h>
#include <d3dcompiler.h>

namespace wrl = Microsoft::WRL; //an alias for the namespace. *DO NOT use global namepaces in header files*
namespace dx = DirectX;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

//#define GFX_THROW_FAILED(hrcall) if(FAILED(hr = (hrcall))) throw Graphics::HrException(__LINE__,__FILE__, hr);
//#define GFX_DEVICE_REMOVED_EXCEPT(hr) Graphics::DeviceRemovedException(__LINE__,__FILE__, (hr));

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

	UINT swapCreateFlags = 0u;
#ifndef NDEBUG
	swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	HRESULT hr; //used for debug error checking

	//create device and front/back buffers, and swap chain and rendering context
	GFX_THROW_INFO(D3D11CreateDeviceAndSwapChain(
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
	GFX_THROW_INFO(pSwap->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer));
	GFX_THROW_INFO(pDevice->CreateRenderTargetView(
		pBackBuffer.Get(),
		nullptr,
		&pTarget
	));

	//create depth stencil state
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	wrl::ComPtr<ID3D11DepthStencilState> pDSState;
	GFX_THROW_INFO(pDevice->CreateDepthStencilState(&dsDesc, &pDSState));
	//bind to pipeline
	pContext->OMSetDepthStencilState(pDSState.Get(), 1u);

	//create depth stencil texture
	wrl::ComPtr<ID3D11Texture2D> pDepthStencil;
	D3D11_TEXTURE2D_DESC depDesc = {};
	depDesc.Width = 800u;
	depDesc.Height = 600u;
	depDesc.MipLevels = 1u;
	depDesc.ArraySize = 1u;
	depDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depDesc.SampleDesc.Count = 1u;
	depDesc.SampleDesc.Quality = 0u;
	depDesc.Usage = D3D11_USAGE_DEFAULT;
	depDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	GFX_THROW_INFO(pDevice->CreateTexture2D(&depDesc, nullptr, &pDepthStencil));

	//create view of depth stencil texture
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0u;
	GFX_THROW_INFO(pDevice->CreateDepthStencilView(pDepthStencil.Get(), &dsvDesc, &pDSV));
	
	//bind depth stencil view to OM
	pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), pDSV.Get());
}

void Graphics::EndFrame() {
	HRESULT hr;

#ifndef NDEBUG
	infoManager.Set();
#endif

	if(FAILED(hr = pSwap->Present(1u, 0u))) { //present frame to buffer
		if(hr == DXGI_ERROR_DEVICE_REMOVED) {
			throw GFX_DEVICE_REMOVED_EXCEPT(pDevice->GetDeviceRemovedReason());
		}else {
			GFX_EXCEPT(hr);
		}
	}
}

void Graphics::ClearBuffer(float red, float green, float blue) noexcept {
	const float color[] = { red, green, blue, 1.0f };
	pContext->ClearRenderTargetView(pTarget.Get(), color);
	pContext->ClearDepthStencilView(pDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
}

//Graphics Exceptions
Graphics::HrException::HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs) noexcept
	: Exception(line, file), hr(hr)
{
	//join all info messages with newlines into single string
	for(const auto& m : infoMsgs) {
		info += m;
		info.push_back('\n');
	}
	//remove final newline if exists
	if(!info.empty()) {
		info.pop_back();
	}
}

const char* Graphics::HrException::what() const noexcept {
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Error String]" << GetErrorString() << std::endl
		<< "[Description]" << GetErrorDescription() << std::endl;
		
		if(!info.empty()) {
			oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
		}

		oss << GetOriginalString();
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
std::string Graphics::HrException::GetErrorInfo() const noexcept
{
	return info;
}
const char* Graphics::DeviceRemovedException::GetType() const noexcept {
	return "Uriel Graphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
}

void Graphics::DrawTestTriangle(float angle, float x, float y, float z) { //pDevice creates stuff and pContext issues commands

	//dx::XMVECTOR v = dx::XMVectorSet(3.0f, 3.0f, 0.0f, 0.0f);
	//auto result = dx::XMVector3Transform(v, dx::XMMatrixScaling(1.5f, 0.0f, 0.0f));
	//auto xx = dx::XMVectorGetX(result);
	HRESULT hr;

	struct Vertex {
		struct {
			float x, y, z;
		} pos;
		//struct{
		//	unsigned char r, g, b, a; 
		//} color;
	};

	//Make a vertex buffer
	Vertex vertices[] = {
		{-1.0f, -1.0f, -1.0f },
		{ 1.0f, -1.0f, -1.0f },
		{-1.0f,  1.0f, -1.0f },
		{ 1.0f,  1.0f, -1.0f },
		{-1.0f, -1.0f,  1.0f },
		{ 1.0f, -1.0f,  1.0f },
		{-1.0f,  1.0f,  1.0f },
		{ 1.0f,  1.0f,  1.0f },
	};
	//vertices[0].color.r = 0;
	//vertices[0].color.g = 0;
	//vertices[0].color.b = 255;

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
	GFX_THROW_INFO(pDevice->CreateBuffer(&bd, &sd, &pVertextBuffer));
	//bind vertex buffer to pipeline
	const UINT stride = sizeof(Vertex);
	const UINT offset = 0u;
	pContext->IASetVertexBuffers(0u, 1u, pVertextBuffer.GetAddressOf(), &stride, &offset);

	//Make an index buffer
	const unsigned short indices[] = {
		0,2,1, 2,3,1,
		1,3,5, 3,7,5,
		2,6,3, 3,6,7,
		4,5,7, 4,7,6,
		0,4,2, 2,4,6,
		0,1,4, 1,5,4
	};
	
	wrl::ComPtr<ID3D11Buffer> pIndexBuffer;
	D3D11_BUFFER_DESC ibd = {};
	ibd.ByteWidth = sizeof(indices);
	ibd.StructureByteStride = sizeof(unsigned short);
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0u;
	ibd.MiscFlags = 0u;
	D3D11_SUBRESOURCE_DATA isd = {};
	isd.pSysMem = indices;
	//Create index buffer
	GFX_THROW_INFO(pDevice->CreateBuffer(&ibd, &isd, &pIndexBuffer));
	//bind index buffer
	pContext->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);

	//Make constant buffer for shape transformation
	struct ConstantBuffer {
		dx::XMMATRIX transform;
	};
	const ConstantBuffer cb = {
		{ //basic Z-rotation matrix
			//(3.0f/4.0f)*std::cos(angle),  std::sin(angle), 0.0f,  0.0f,
			//(3.0f/4.0f)*-std::sin(angle), std::cos(angle), 0.0f,  0.0f,
			//0.0f,						  0.0f,		       1.0f,  0.0f,
			//0.0f,						  0.0f,			   0.0f,  1.0f,
			dx::XMMatrixTranspose(
				dx::XMMatrixRotationZ(angle) *
				dx::XMMatrixRotationX(angle) *
				dx::XMMatrixTranslation(x, y, z) *
				dx::XMMatrixPerspectiveLH(1.0f, 3.0f/4.0f, 0.5f, 10.0f)
			)
		}
	};
	wrl::ComPtr<ID3D11Buffer> pConstantBuffer;
	D3D11_BUFFER_DESC cbd = {};
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.ByteWidth = sizeof(cb);
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.MiscFlags = 0u;
	cbd.StructureByteStride = 0u;
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	D3D11_SUBRESOURCE_DATA csd = {};
	csd.pSysMem = &cb;
	GFX_THROW_INFO(pDevice->CreateBuffer(&cbd, &csd, &pConstantBuffer));
	//bind constant buffer
	pContext->VSSetConstantBuffers(0u, 1u, pConstantBuffer.GetAddressOf());

	struct ConstantBuffer2 {
		struct {
			float r, g, b, a;
		} face_colors[6];
	};

	const ConstantBuffer2 cb2 = {
		{
			{1.0f, 0.0f, 1.0f},
			{1.0f, 0.0f, 0.0f},
			{0.0f, 1.0f, 0.0f},
			{0.0f, 0.0f, 1.0f},
			{1.0f, 1.0f, 0.0f},
			{0.0f, 1.0f, 1.0f},
		}
	};

	wrl::ComPtr<ID3D11Buffer> pConstantBuffer2;
	D3D11_BUFFER_DESC cbd2;
	cbd2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd2.ByteWidth = sizeof(cb2);
	cbd2.CPUAccessFlags = 0u;
	cbd2.MiscFlags = 0u;
	cbd2.StructureByteStride = 0u;
	cbd2.Usage = D3D11_USAGE_DEFAULT;
	D3D11_SUBRESOURCE_DATA cbs2 = {};
	cbs2.pSysMem = &cb2;
	GFX_THROW_INFO(pDevice->CreateBuffer(&cbd2, &cbs2, &pConstantBuffer2));
	//bind constant buffer 2
	pContext->PSSetConstantBuffers(0u, 1u, pConstantBuffer2.GetAddressOf());




	//create pixel shader
	wrl::ComPtr<ID3D11PixelShader> pPixelShader;
	wrl::ComPtr<ID3DBlob> pBlob;
	GFX_THROW_INFO(D3DReadFileToBlob(L"PixelShader.cso", &pBlob));
	GFX_THROW_INFO(pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader));
	//bind pixel shader
	pContext->PSSetShader(pPixelShader.Get(), nullptr, 0u);

	//create vertex shader
	wrl::ComPtr<ID3D11VertexShader> pVertexShader;
	D3DReadFileToBlob(L"VertexShader.cso", &pBlob); //make sure shader file output path in the compiler is set to the ProjectDirectory instead of OutputDirectory
	GFX_THROW_INFO(pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader));
	//bind vertex shader
	pContext->VSSetShader(pVertexShader.Get(), nullptr, 0u); //remember that to get the single-pointer address of a ComPtr you need to use .Get()

	//input (vertex) layout (2D position only)
	wrl::ComPtr<ID3D11InputLayout> pInputLayout;
	const D3D11_INPUT_ELEMENT_DESC ied[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		//{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	//create Input Layout
	GFX_THROW_INFO(pDevice->CreateInputLayout(
		ied, 
		(UINT)std::size(ied), 
		pBlob->GetBufferPointer(), 
		pBlob->GetBufferSize(), 
		&pInputLayout
	));
	//bind input layout
	pContext->IASetInputLayout(pInputLayout.Get());

	//bind render target
	//pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), nullptr); //OutputMerger set render target

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

	GFX_THROW_INFO_ONLY(pContext->DrawIndexed((UINT)std::size(indices), 0u, 0u));
}

//Info exception stuff *******************************
Graphics::InfoException::InfoException(int line, const char* file, std::vector<std::string> infoMsgs) noexcept
	: Exception(line, file)
{
	for(const auto& m : infoMsgs) {
		info += m;
		info.push_back('\n');
	}
	//remove final newline if exists
	if(!info.empty()) {
		info.pop_back();
	}
}

const char* Graphics::InfoException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	oss << GetOriginalString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::InfoException::GetType() const noexcept
{
	return "Uriel Graphics Info Exception";
}

std::string Graphics::InfoException::GetErrorInfo() const noexcept
{
	return info;
}

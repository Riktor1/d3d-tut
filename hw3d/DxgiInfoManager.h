#pragma once

#include "IncludeWin.h"
#include <vector>
#include <wrl.h>
#include <dxgidebug.h>

class DxgiInfoManager {
public:
	DxgiInfoManager();
	~DxgiInfoManager() = default;
	DxgiInfoManager(const DxgiInfoManager&) = delete;
	DxgiInfoManager operator= (const DxgiInfoManager&) = delete;
	void Set() noexcept;
	std::vector<std::string> GetMessages() const;
private:
	unsigned long long next = 0u;
	//struct IDXGIInfoQueue* pDxgiInfoQueue = nullptr;
	Microsoft::WRL::ComPtr<IDXGIInfoQueue> pDxgiInfoQueue;
};
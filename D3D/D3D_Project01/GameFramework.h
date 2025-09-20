#pragma once
#include "stdafx.h"

class CGameFramework
{
public:
	CGameFramework();
private:
	// window ����
	HINSTANCE h_instance;
	HWND h_wnd;

	// directX ����
	ComPtr<IDXGIFactory4> dxgi_factory{};
	ComPtr<IDXGISwapChain3> swap_chain{};
	ComPtr<ID3D12Device> d3d_device{};
};


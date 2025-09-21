#pragma once
#include "stdafx.h"

class CGameFramework
{
public:
	CGameFramework();
	void initDirect3D();
private:
	// window 관련
	HINSTANCE h_instance;
	HWND h_wnd;

	// directX 관련
	ComPtr<IDXGIFactory4> dxgi_factory{};
	ComPtr<ID3D12Device> d3d_device{};
	ComPtr<ID3D12Fence> fence{};

	// swap chain
	ComPtr<IDXGISwapChain3> swap_chain{};
	static const UINT swap_chain_buffer_num{ 2 };
	UINT64 fence_value[swap_chain_buffer_num]{};
	HANDLE fence_event{};

	// multi sample
	UINT ms_quality_level{};
	bool ms_enabled{};
};


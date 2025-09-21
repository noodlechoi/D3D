#pragma once
#include "WindowManager.h"

class CGameFramework
{
public:
	CGameFramework();

	void Initialize();
	void CreateDirect3D();
	void CreateCommandObjects();
	void CreateSwapChain();
private:
	// window 관련
	HINSTANCE h_instance;
	HWND h_wnd;

	// direct3D
	ComPtr<IDXGIFactory4> dxgi_factory{};
	ComPtr<ID3D12Device> d3d_device{};

	// command objects
	ComPtr<ID3D12CommandQueue> command_queue{};
	ComPtr<ID3D12CommandAllocator> command_allocator{};
	ComPtr<ID3D12GraphicsCommandList> command_list{};

	// swap chain
	ComPtr<IDXGISwapChain3> swap_chain{};
	static const UINT swap_chain_buffer_num{ 2 };
	UINT swap_chain_buffer_index{};

	// fence - for 동기화
	ComPtr<ID3D12Fence> fence{};
	UINT64 fence_value[swap_chain_buffer_num]{};
	HANDLE fence_event{};

	// multi sample
	UINT ms_quality_level{};
	bool ms_enabled{};
};


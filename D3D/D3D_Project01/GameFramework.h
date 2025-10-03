#pragma once
#include "WindowManager.h"

class CGameFramework
{
public:
	CGameFramework();

	// D3D 관련 객체 생성
	void Initialize();
	void CreateDirect3D();
	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateRtvAndDsvHeaps();
	void CreateRenderTargetViews();
	void CreateDepthStencilView();
	void waitForGpuComplete();

	// 전체화면 swap chain 상태 변화
	void ChangeSwapChainState();

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

	// 렌더 타겟 버퍼, 서술자 힙 인터페이스 포인터, 렌더 타겟 서술자 원소 크기
	ComPtr<ID3D12Resource> render_target_buffers[swap_chain_buffer_num]{};
	ComPtr<ID3D12DescriptorHeap> rtv_descriptor_heap{};
	UINT rtv_increment_size{};

	// 깊이-스텐실 버퍼, 서술자 힙 인터페이스 포인터, 깊이-스텐실 서술자 원소 크기
	ComPtr<ID3D12Resource> depth_stencil_buffer{};
	ComPtr<ID3D12DescriptorHeap> dsv_descriptor_heap{};
	UINT dsv_increment_size{};

	// fence - for 동기화
	ComPtr<ID3D12Fence> fence{};
	UINT64 fence_value[swap_chain_buffer_num]{};
	HANDLE fence_event{};

	// multi sample
	UINT ms_quality_level{};
	bool ms_enabled{};
};


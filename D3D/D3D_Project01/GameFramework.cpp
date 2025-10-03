#include "GameFramework.h"

CGameFramework::CGameFramework()
{
}

void CGameFramework::Initialize()
{
	CreateDirect3D();
	CreateCommandObjects();
}

void CGameFramework::CreateDirect3D()
{
	UINT dxgiFactoryFlags{};
#if defined(_DEBUG) || defined(_DEBUG)
	// d3d12 디버그층 활성화
	ComPtr<ID3D12Debug> debugController;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
	if (debugController.Get()) {
		debugController->EnableDebugLayer();
	}
	dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
	// factory 생성
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgi_factory)));

	// 어댑터(하드웨어)
	IDXGIAdapter1* adapter{};
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != dxgi_factory->EnumAdapters1(i, &adapter); ++i) {
		DXGI_ADAPTER_DESC1 adapterDesc;
		adapter->GetDesc1(&adapterDesc);
		if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;	// 렌더링 드라이버라면?
		if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&d3d_device)))) break;	// direct12 디바이스 생성
	}
	// 12를 지원하는 디바이스를 생성할 수 없으면 WARP 디바이스 생성
	if (!adapter) {
		dxgi_factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
		ThrowIfFailed(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d_device)));
	}

	// 디바이스가 지원하는 다중 샘플 품질 수준 확인
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaAQualityLevels;
	msaaAQualityLevels.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	msaaAQualityLevels.SampleCount = 4;	// Msaa4x 다중 샘플링
	msaaAQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msaaAQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(d3d_device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaAQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS)));
	ms_quality_level = msaaAQualityLevels.NumQualityLevels;

	// 품질 수준이 1보다 크면 다중 샘플링 활성화
	ms_enabled = (ms_quality_level > 1) ? true : false;

	// 동기화를 위한 fence 생성
	ThrowIfFailed(d3d_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
	for (int i = 0; i < swap_chain_buffer_num; ++i)
		fence_value[i] = 0;

	// 펜스와 동기화를 위한 이벤트 객체 생성. signal 시 이벤트 값을 자동적으로 FALSE가 되도록 생성
	/*
	HANDLE CreateEvent(
	LPSECURITY_ATTRIBUTES lpEventAttributes, // 보안 속성
	BOOL bManualReset,                      // 수동/자동 리셋 여부
	BOOL bInitialState,                     // 초기 상태 (신호/비신호)
	LPCSTR lpName                           // 이벤트 객체 이름
	);
	*/
	fence_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	if (adapter) adapter->Release();
}

void CGameFramework::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;	// GPU 계산이 오래 걸릴 때 time out 실행 후 다음 명형 실행
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;	// GPU가 모든 명령 직접 실행

	// 직접 명령 큐 생성
	ThrowIfFailed(d3d_device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&command_queue)));

	// 할당자 생성
	ThrowIfFailed(d3d_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator)));

	//리스트 생성
	ThrowIfFailed(d3d_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator.Get(), nullptr, IID_PPV_ARGS(command_list.GetAddressOf())));

	// 리스트가 생성되면 Open상태가 됨
	ThrowIfFailed(command_list->Close());
}

void CGameFramework::CreateSwapChain()
{
//	RECT rc;
//	GetClientRect(h_wnd, &rc);
//	client_width = rc.right - rc.left;
//	client_height = rc.bottom - rc.top;
//
//	// 따라하기05
//	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
//	swapChainDesc.BufferCount = swap_chain_buffer_num;
//	swapChainDesc.BufferDesc.Width = client_width;
//	swapChainDesc.BufferDesc.Height = client_height;
//	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
//	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
//	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
//	swapChainDesc.OutputWindow = h_wnd;
//	swapChainDesc.SampleDesc.Count = (ms_enabled) ? 4 : 1;
//	swapChainDesc.SampleDesc.Quality = (ms_enabled) ? (ms_quality_level - 1) : 0;
//	swapChainDesc.Windowed = TRUE;
//	// 전체 화면 모드에서 바탕화면의 해상도를 후면 버퍼의 크기에 맞게 변경
//	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
//
//	ThrowIfFailed(dxgi_factory->CreateSwapChain(command_queue.Get(), &swapChainDesc, (IDXGISwapChain**)swap_chain.GetAddressOf()));
//	swap_chain_buffer_index = swap_chain->GetCurrentBackBufferIndex();
//	ThrowIfFailed(dxgi_factory->MakeWindowAssociation(h_wnd, DXGI_MWA_NO_ALT_ENTER));	// alt+enter에 응답하지 않게 설정
//#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE 
//	CreateRenderTargetViews();
//#endif
}

void CGameFramework::CreateRtvAndDsvHeaps()
{
	// 렌더 타겟 서술자 힙(서술자 개수 = 스왑체인 버퍼의 개수) 생성
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.NumDescriptors = swap_chain_buffer_num;
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descriptorHeapDesc.NodeMask = 0;

	ThrowIfFailed(d3d_device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&rtv_descriptor_heap)));

	// 렌더 타겟 서술자 힙 원소의 크기 저장
	rtv_increment_size = d3d_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// 깊이-스탠실 서술자 힙(1개) 생성
	descriptorHeapDesc.NumDescriptors = 1;
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	ThrowIfFailed(d3d_device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&dsv_descriptor_heap)));

	// 원소 크기 저장
	dsv_increment_size = d3d_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void CGameFramework::CreateRenderTargetViews()
{
	// 스왑체인의 각 후면 버퍼에 대한 렌더 타겟 뷰 생성
	D3D12_CPU_DESCRIPTOR_HANDLE rtvCPUDescriptorHandle = rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < swap_chain_buffer_num; ++i) {
		ThrowIfFailed(swap_chain->GetBuffer(i, IID_PPV_ARGS(&render_target_buffers[i])));
		d3d_device->CreateRenderTargetView(render_target_buffers[i].Get(), NULL, rtvCPUDescriptorHandle);
		rtvCPUDescriptorHandle.ptr += rtv_increment_size;
	}
}

void CGameFramework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC resourceDesc;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Alignment = 0;
	//resourceDesc.Width = client_width;
	//resourceDesc.Height = client_height;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	resourceDesc.SampleDesc.Count = (ms_enabled) ? 4 : 1;
	resourceDesc.SampleDesc.Quality = (ms_enabled) ? (ms_quality_level - 1) : 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	// 버퍼 생성
	ThrowIfFailed(d3d_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&depth_stencil_buffer)));

	// 뷰 생성
	// 힙 시작 핸들 값
	D3D12_CPU_DESCRIPTOR_HANDLE dsvCPUDesctiptorHandle = dsv_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
	d3d_device->CreateDepthStencilView(depth_stencil_buffer.Get(), NULL, dsvCPUDesctiptorHandle);
}

void CGameFramework::waitForGpuComplete()
{
	// CPU 펜스 값 증가
	const UINT64 fenceValue = ++fence_value[swap_chain_buffer_index];
	// GPU가 펜스 값을 설정하는 명령을 명령어 큐에 추가
	ThrowIfFailed(command_queue->Signal(fence.Get(), fenceValue));
	// GPU 펜스 값이 CPU 펜스 값보다 작으면 계속 이벤트를 기다림
	if (fence->GetCompletedValue() < fenceValue) {
		ThrowIfFailed(fence->SetEventOnCompletion(fenceValue, fence_event));
		::WaitForSingleObject(fence_event, INFINITE);
	}
}

void CGameFramework::ChangeSwapChainState()
{
	waitForGpuComplete();

	BOOL fullScreenState{};
	swap_chain->GetFullscreenState(&fullScreenState, NULL);
	swap_chain->SetFullscreenState(!fullScreenState, NULL);	// full -> window, window -> full

	DXGI_MODE_DESC targetParameters;
	targetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	/*targetParameters.Width = client_width;
	targetParameters.Height = client_height;*/
	targetParameters.RefreshRate.Numerator = 60;
	targetParameters.RefreshRate.Denominator = 1;
	targetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	targetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swap_chain->ResizeTarget(&targetParameters);

	// ResizeBuffers가 성공하기 위해 모든 직/간접 참조를 해제해야 함
	for (int i = 0; i < swap_chain_buffer_num; ++i) {
		if (render_target_buffers[i]) render_target_buffers[i].Reset();
	}

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swap_chain->GetDesc(&swapChainDesc);
	//swap_chain->ResizeBuffers(swap_chain_buffer_num, client_width, client_height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags);

	swap_chain_buffer_index = swap_chain->GetCurrentBackBufferIndex();

	CreateRenderTargetViews();
}
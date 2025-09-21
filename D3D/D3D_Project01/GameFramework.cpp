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
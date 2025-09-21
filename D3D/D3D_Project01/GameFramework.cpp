#include "GameFramework.h"

CGameFramework::CGameFramework()
{
}

void CGameFramework::initDirect3D()
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

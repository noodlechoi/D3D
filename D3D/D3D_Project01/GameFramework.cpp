#include "GameFramework.h"

CGameFramework::CGameFramework()
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

	//D3D12CreateDevice()
}

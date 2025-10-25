#pragma once
#include "stdafx.h"

// D3D12Book 참고
template<typename T>
class CUploadBuffer {
public:
	CUploadBuffer(ID3D12Device* device, UINT elementCnt, bool isConstBuf)
		: is_const_buf{ isConstBuf }
	{
		element_byte_size = sizeof(T);

		if (isConstBuf)
			element_byte_size = CalcConstBufByteSize(sizeof(T));

		D3D12_HEAP_PROPERTIES heapProperties{};
		heapProperties.Type = heapType;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 1;
		heapProperties.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = elementCnt * element_byte_size;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ThrowIfFailed(device->CreateCommittedResource(heapProperties, resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(upload_buf.GetAddressOf())));

		ThrowIfFailed(upload_buf->Map(0, nullptr, (void**)&mapped_data));
	}

	CUploadBuffer(const CUploadBuffer& rhs) = delete;
	CUploadBuffer& operator=(const CUploadBuffer& rhs) = delete;
	~CUploadBuffer()
	{
		if (!upload_buf)
			upload_buf->Unmap(0, nullptr);
	}

	ID3D12Resource* GetResource()const
	{
		return upload_buf.Get();
	}

	void CopyData(int elementIndex, const T& data)
	{
		memcpy(&mapped_data[elementIndex * mElementByteSize], &data, sizeof(T));
	}
private:
	ComPtr<ID3D12Resource> upload_buf;
	BYTE* mapped_data{};

	UINT element_byte_size{};
	bool is_const_buf{};
};


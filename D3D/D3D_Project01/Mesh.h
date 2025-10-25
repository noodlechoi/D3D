#pragma once
#include "stdafx.h"

class CVertex{
public:
	CVertex() {}
	CVertex(float x, float y, float z) : pos{ XMFLOAT3(x, y, z) } {}
	CVertex(XMFLOAT3 pos) : pos{ pos } {}
protected:
	XMFLOAT3 pos{};
	XMFLOAT4 color{};
};

class CMesh{
public:
	CMesh() {};
	CMesh(CVertex* vertexes);
	void Render(ID3D12GraphicsCommandList* commandList);
protected:
	std::vector<CVertex> vertexes;
};


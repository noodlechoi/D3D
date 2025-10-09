#pragma once
#include "stdafx.h"

class CVertex{
public:
	CVertex() {}
	CVertex(float x, float y, float z) : pos{ XMFLOAT3(x, y, z) } {}
	CVertex(XMFLOAT3 pos) : pos{ pos } {}
protected:
	XMFLOAT3 pos{};
};

class CMesh{
};


#pragma once
#include "stdafx.h"

// window 창 핸들 값 관리하는 클래스 - for gameframework와 윈도우를 분리
class CWindowManager
{
public:
	CWindowManager() {};
	CWindowManager(HWND hwnd);

	void Update();
private:
	HWND h_wnd{};

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};


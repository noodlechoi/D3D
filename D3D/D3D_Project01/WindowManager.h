#pragma once
#include "stdafx.h"

// window 관련 함수 변수를 다루는 클래스 - for gameframework와 윈도우를 분리
class CWindowManager
{
public:
	CWindowManager() {};
	CWindowManager(HWND hwnd);
	void SetHwnd(HWND hwnd) { h_wnd = hwnd; }
private:
	HWND h_wnd{};
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};


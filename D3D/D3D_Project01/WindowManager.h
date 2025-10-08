#pragma once
#include "GameFramework.h"

//class CGameFramework;
class CWindowGameMediator;

// window 창 핸들 값 관리하는 클래스 - for gameframework와 윈도우를 분리
class CWindowManager {
public:
	CWindowManager(HINSTANCE hInstance, CWindowGameMediator* mediator);

	ATOM MyRegisterClass();
	BOOL Init(int nCmdShow);
	bool Create(int nCmdShow);
	HWND GetHWND() const { return h_wnd; }
	SIZE GetClientSize() const { return client; }
private:
	CWindowGameMediator* mediator;
	HWND h_wnd{};
	HINSTANCE h_inst;
	SIZE client{ FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT };

	static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	// 정보 대화 상자의 메시지 처리기
	static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	// 실제 실행 함수
	LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
};

// WindowManager-GameFramework 중재자
class CWindowGameMediator {
public:
	CWindowGameMediator() = delete;	// window manager가 무조건 창 핸들값을 받아야함
	CWindowGameMediator(HINSTANCE hInstance);

	// window manager 관련 함수
	SIZE GetClientSize() const { return window_manager->GetClientSize(); }
	HWND GetHWND() const { return window_manager->GetHWND(); }

	bool Init(int nCmdShow);
	void Update();
	void OnDestroy();
private:
	std::unique_ptr<CWindowManager> window_manager;
	std::unique_ptr<CGameFramework> game_framework;
};
#include "WindowManager.h"
#include "D3D_Project01.h"

CWindowManager::CWindowManager(HINSTANCE hInstance, CWindowGameMediator* mediator)
    : h_inst{hInstance}, mediator{mediator}
{
}

ATOM CWindowManager::MyRegisterClass()
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = StaticWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = h_inst;
    wcex.hIcon = LoadIcon(h_inst, MAKEINTRESOURCE(IDI_D3DPROJECT01));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"DirectXWindow";
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL CWindowManager::Init(int nCmdShow)
{
    DWORD dwstyle = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_BORDER | WS_MAXIMIZEBOX; // 창 스타일 정의
    RECT rc = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };
    AdjustWindowRect(&rc, dwstyle, FALSE);

    // client 크기 저장
    client.cx = rc.right - rc.left;
    client.cy = rc.bottom - rc.top;

    //hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.
    CreateWindowEx(0, L"DirectXWindow", L"DX window", dwstyle,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, h_inst, this);


    if (!h_wnd) {
        return FALSE;
    }

    ShowWindow(h_wnd, nCmdShow);
    UpdateWindow(h_wnd);

    return TRUE;
}

bool CWindowManager::Create(int nCmdShow)
{
    MyRegisterClass();
    if (!Init(nCmdShow)) {
        return FALSE;
    }

    return TRUE;
}

INT_PTR CALLBACK CWindowManager::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
LRESULT CALLBACK CWindowManager::StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CWindowManager* window{};
    
    if (WM_NCCREATE == msg) {   // CreateWindow 메시지 시에만 lParam 전달
        CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
        window = (CWindowManager*)cs->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
        window->h_wnd = hwnd;
    }
    else {
        window = (CWindowManager*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    if (window) {
        return window->WndProc(msg, wParam, lParam);
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CWindowManager::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SIZE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MOUSEMOVE:
    case WM_KEYDOWN:
    case WM_KEYUP:
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(h_wnd, msg, wParam, lParam);
    }
    return 0;
}
// =================

CWindowGameMediator::CWindowGameMediator(HINSTANCE hInstance)
    : window_manager{ std::make_unique<CWindowManager>(hInstance, this) }, game_framework{ std::make_unique<CGameFramework>(this) }
{
}

bool CWindowGameMediator::Init(int nCmdShow)
{
    if (!window_manager->Create(nCmdShow)) {
        return false;
    }
    game_framework->OnCreate();

    return true;
}

void CWindowGameMediator::Update()
{
    game_framework->FrameAdvance();
}

void CWindowGameMediator::OnDestroy()
{
    game_framework->OnDestroy();
}

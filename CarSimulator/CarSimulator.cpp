﻿// CarSimulator.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "CarSimulator.h"
#include "GameFramework.h"

CGameFramework gGameFramework;

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


void err_display(char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
    wchar_t wtext[20];
    mbstowcs(wtext, msg, strlen(msg) + 1);
    LPWSTR ptr = wtext;

    MessageBox(NULL, (LPCTSTR)lpMsgBuf, wtext, MB_ICONERROR);

    LocalFree(lpMsgBuf);
}

void err_quit(char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

    wchar_t wtext[20];
    mbstowcs(wtext, msg, strlen(msg) + 1);
    LPWSTR ptr = wtext;

    MessageBox(NULL, (LPCTSTR)lpMsgBuf, wtext, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

int recvn(SOCKET s, char* buf, int len, int flags)
{
    int received;
    char* ptr = buf;
    int left = len;

    // 남은 바이트가 0이면 수신 종료
    while (left > 0)
    {
        received = recv(s, ptr, left, flags);
        if (received == SOCKET_ERROR)
        {
            return SOCKET_ERROR;
        }
        else if (received == 0)
        {
            break;
        }

        left -= received;
        ptr += received;
    }
    return (len - left);
}
void SendPlayerInfo(const SOCKET& sock)
{
    PlayerData pSendData;
    std::shared_ptr<CBullet> bullet = gGameFramework.GetPlayerBullet();
    if (bullet == NULL)
        pSendData = PlayerData{ gGameFramework.GetPlayerPosition(), gGameFramework.GetPlayerRotation(), gGameFramework.GetPlayerLife(), FALSE, {0, 0, 0} };
    else
        pSendData = PlayerData{ gGameFramework.GetPlayerPosition(), gGameFramework.GetPlayerRotation(), gGameFramework.GetPlayerLife(), TRUE, bullet->GetPosition() };

    if(send(sock, (char*)&pSendData, sizeof(PlayerData), 0) == SOCKET_ERROR)
    {
        err_quit("send()");
    }
}

void RecvGameStart(const SOCKET& sock)
{
    XMFLOAT3 StartPos;
    recvn(sock, (char*)&StartPos, sizeof(XMFLOAT3), 0);

    gGameFramework.m_pPlayer->SetRigidBodyPosition(StartPos);
}

void RecvPlayerInfo(const SOCKET& sock)
{
    recvn(sock, (char*)&g_otherPlayersData[0], sizeof(PlayerData), 0);
    recvn(sock, (char*)&g_otherPlayersData[1], sizeof(PlayerData), 0);
}


DWORD WINAPI TransportData(LPVOID arg)
{
    SOCKET clientSock = (SOCKET)arg;

    int msgType = 0;

    // 시작 신호를 기다림
    while (1)
    {
        if (recvn(clientSock, (char*)&msgType, sizeof(int), 0) == SOCKET_ERROR)
        {
            err_display("receive start");
        }

        if (msgType & GAME_START)
        {
            RecvGameStart(clientSock);
            break;
        }
    }

    while (1)
    {
        int sendMsg = PLAYER_UPDATE;
        if (send(clientSock, (char*)&sendMsg, sizeof(int), 0) == SOCKET_ERROR)
        {
            err_quit("send()");
        }

        SendPlayerInfo(clientSock);

        recvn(clientSock, (char*)&msgType, sizeof(int), 0);

        // 분기, 플레이어 조작
        if (msgType & PLAYER_UPDATE)
        {
            //ZeroMemory()??
            //RecvPlayerInfo(0); 로 안 나누는 게 맞는 지 잘 모르겠음. 일단 계획서 살짝 고침.
            RecvPlayerInfo(clientSock);
        }
        if (msgType & PLAYER_HIT)
        {
            gGameFramework.PlayerHIt();
        }
        if (msgType & BULLET_DELETED)
        {
            gGameFramework.m_pPlayer->EraseBullet();
        }
        if (msgType & GAME_OVER)
        {
            //recvn(clientSock, (char*)&/*blabla == 승리여부 변수*/, sizeof(/*blabla*/), 0);
            break;
        }

        Sleep(0.1f);
    }

    closesocket(clientSock);

    DeleteCriticalSection(&g_cs);

    WSACleanup();

    return 0;
}

void InitNetworkSocket()
{
    LPWSTR* szArgList;
    int argCount;

    InitializeCriticalSection(&g_cs);

    szArgList = CommandLineToArgvW(GetCommandLine(), &argCount);

    WSADATA wsa;

    WSAStartup(MAKEWORD(2, 2), &wsa);

    // socket()
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) err_quit("socket()");

    char ctext[20];
    wcstombs(ctext, szArgList[1], wcslen(szArgList[1]) + 1);
    const char* sAddr = ctext;

    // connect()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(sAddr);

    wcstombs(ctext, szArgList[2], wcslen(szArgList[2]) + 1);
    const char* sPort = ctext;
    serveraddr.sin_port = htons(atoi(sPort));
    if (connect(clientSocket, (SOCKADDR*)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR)
    {
        err_display("connect()");
    }
    
    CreateThread(NULL, 0, TransportData, (LPVOID)clientSocket, 0, NULL);
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CARSIMULATOR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CARSIMULATOR));

    MSG msg;

    // 기본 메시지 루프입니다:
    while (1)
    {
        if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) break;
            if (!::TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
            }
        }
        else
        {
            gGameFramework.FrameAdvance();
            Sleep(0.1f);
        }
    }
    gGameFramework.OnDestroy();

    return (int)msg.wParam;
}


//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CARSIMULATOR));
    wcex.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    //주 윈도우의 메뉴가 나타나지 않도록 한다.
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    return ::RegisterClassEx(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    RECT rc = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };
    DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_BORDER;
    AdjustWindowRect(&rc, dwStyle, FALSE);
    HWND hMainWnd = CreateWindow(szWindowClass, szTitle, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);
    if (!hMainWnd) return(FALSE);
    gGameFramework.OnCreate(hInstance, hMainWnd);

    ::ShowWindow(hMainWnd, nCmdShow);
    ::UpdateWindow(hMainWnd);

#ifdef _WITH_SWAPCHAIN_FULLSCREEN_STATE
    gGameFramework.ChangeSwapChainState();
#endif

    InitNetworkSocket();

    return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SIZE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MOUSEMOVE:
    case WM_KEYDOWN:
    case WM_KEYUP:
        gGameFramework.OnProcessingWindowMessage(hWnd, message, wParam, lParam);
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        break;
    default:
        return(::DefWindowProc(hWnd, message, wParam, lParam));
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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

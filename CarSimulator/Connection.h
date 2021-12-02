#pragma once
#include "stdafx.h"
#include "GameFramework.h"
using namespace std;

// 전역 변수 선언
extern CGameFramework gGameFramework;	// 게임프레임워크
extern array<HANDLE, 2> g_events;		// 렌더, 송신 쓰레드 동기화 이벤트
extern bool g_bGameStarted;				// 게임이 시작되면 true로 바뀜
extern bool g_bGameOver;				// 게임이 종료되면 true로 바뀜

// 쓰레드 함수
DWORD WINAPI TransportData(LPVOID arg);

// 송신 함수
void SendPlayerInfo(const SOCKET& sock);

// 수신 함수
int RecvN(const SOCKET& socket, char* buffer, int length, int flags);
void RecvGameStart(const SOCKET& sock);
void RecvPlayerInfo(const SOCKET& sock);
void RecvGameOver(const SOCKET& sock);

// 기타
void InitConnection();
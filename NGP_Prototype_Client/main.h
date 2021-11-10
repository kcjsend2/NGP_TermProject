#pragma once

// Network
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// C++
#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
using namespace std;

// DirectX
#include <DirectXMath.h>
#include <DirectXCollision.h>
using namespace DirectX;

// 패킷
#define GAME_START      0x000001
#define GAME_OVER       0x000010
#define PLAYER_MOVE     0x000100
#define PLAYER_HIT      0x001000
#define BULLET_UPDATE   0x010000
#define BULLET_DELETE   0x100000

// 다른 클라이언트에 대한 정보 구조체
#pragma pack(1)
struct PlayerData
{
    int         m_id;               // 고유 번호
    XMFLOAT3    m_position; 	    // 위치
    XMFLOAT3    m_rotate;		    // 회전 정보(roll, pitch, yaw)
    bool        m_bulletIsValid;    // 총알이 유효한지
    XMFLOAT3    m_bulletPosition;	// 총알 위치

    PlayerData() : m_id{}, m_position {}, m_rotate{}, m_bulletIsValid{}, m_bulletPosition{}
    {

    }
};

// 함수 선언
void err_quit(char* msg);
void err_display(char* msg);
int recvn(const SOCKET& socket, char* buffer, int length, int flags);

void RecvType(const SOCKET& sock);
void RecvGameStart() { }
void RecvGameOver() { }
void RecvPlayerMove() { }
void RecvPlayerHit() { }
void RecvPlayerBullet() { }
#pragma once

// Network
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// C++
#include <array>
#include <fstream>
#include <iostream>
using namespace std;

// DirectX
#include <DirectXMath.h>
#include <DirectXCollision.h>
using namespace DirectX;

// 패킷
#define GAME_START		0b00001
#define GAME_OVER		0b00010
#define PLAYER_UPDATE	0b00100
#define PLAYER_HIT		0b01000
#define BULLET_DELETED	0b10000

// 서버 IP, 포트
#define SERVER_PORT 9000

// 구조체 선언
#pragma pack(1)
struct PlayerData
{
    int         m_id;               // 고유 번호
    XMFLOAT3    m_position; 	    // 위치
    XMFLOAT3    m_rotate;		    // 회전 정보(roll, pitch, yaw)
    bool        m_bulletIsValid;    // 총알이 유효한지
    XMFLOAT3    m_bulletPosition;	// 총알 위치

    PlayerData() : m_id{}, m_position{}, m_rotate{}, m_bulletIsValid{}, m_bulletPosition{}
    {

    }
};

struct ThreadFuncParam
{
    int     id;
    SOCKET  sock;
};

// 송신 함수
void SendGameStart(ThreadFuncParam* param);
void SendGameOver(ThreadFuncParam* param) { }
void SendPlayerInfo(ThreadFuncParam* param);
void SendBulletDeleted(ThreadFuncParam* param) { }

// 그 외 함수
int RecvN(const SOCKET& socket, char* buffer, int length, int flags);
BoundingOrientedBox GetBoundingBox(INT id) { }
bool BulletCollisionCheck(XMFLOAT3 playerPosition, XMFLOAT3 playerRotate, XMFLOAT3 BulletPosition);
bool isGameOver() { }
#pragma once
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <array>
#include <iostream>

using namespace std;
using namespace DirectX;

enum dataType
{
    GAME_START, // no use?
    GAME_OVER,
    PLAYER_STATUS,
    PLAYER_DEAD
};
void err_quit(char* msg);
void err_display(char* msg);
int recvn(SOCKET s, char* buf, int len, int flags);
#pragma pack(1)
struct PlayerData
{
    int m_dType;
    XMFLOAT3 m_position; 	// 플레이어 위치
    XMFLOAT3 m_rotate;		// 플레이어 회전 정보(roll, pitch, yaw)
    int m_life;			// 플레이어의 목숨 수
    bool m_bHasBullet;		// 총알 유무
    XMFLOAT3 m_bulletPosition;	// 총알 위치
    bool m_bIntersected;

    PlayerData()
    {
        m_dType = NULL;
        m_position = { 0.0f, 0.0f, 0.0f };
        m_rotate = { 0.0f, 0.0f, 0.0f };
        m_life = 0;
        m_bHasBullet = false;
        m_bulletPosition = { 0.0f, 0.0f, 0.0f };
        m_bIntersected = false;
    }

    PlayerData(int Type, XMFLOAT3 Position, XMFLOAT3 rotate, int life, bool hasBullet, XMFLOAT3 bulletPosition, bool bIntersected)
    {
        m_dType = Type;
        m_position = Position;
        m_rotate = rotate;
        m_life = life;
        m_bHasBullet = hasBullet;
        m_bulletPosition = bulletPosition;
        m_bIntersected = bIntersected;
    }

    PlayerData(int Type)
    {
        m_dType = Type;
        m_position = { 0.0f, 0.0f, 0.0f };
        m_rotate = { 0.0f, 0.0f, 0.0f };
        m_life = 0;
        m_bHasBullet = false;
        m_bulletPosition = { 0.0f, 0.0f, 0.0f };
        m_bIntersected = false;
    }
};
array<PlayerData, 3> aPlayerData;
CRITICAL_SECTION cs;
struct ProcessClientData_Parameter
{
    SOCKET sock;
    int id;
};

//Sender
void SendGameStart() { }
void SendGameOver() { }
void SendPlayerMove(INT id) { }
void SendBulletPosition(INT id) { }
void SendBulletDeleted(INT id) { }
void SendPlayerHit(INT id) { }

//Receiver
void RecvType(const SOCKET& sock);
void RecvPlayerMove(INT id) { }
void RecvBulletPosition(INT id) { }
void RecvBulletDeleted(INT id) { }
void RecvPlayerDead(INT id) { }

//setter, getter, collision_check, etc.
void SetPlayerPosition(INT id, const XMFLOAT3& position) { }
void SetPlayerRotation(INT id, const XMFLOAT3& rotation) { }
void SetBulletPosition(INT id, const XMFLOAT3& bulletPosition) { }
XMFLOAT3 GetPlayerPosition(INT id) { }
XMFLOAT3 GetPlayerRotation(INT id) { }
BOOL GetHasBullet(INT id) { }
XMFLOAT3 GetBulletPosition(INT id) { }
BoundingOrientedBox GetBoundingBox(INT id) { }
void BulletCollisionCheck(INT id) { }
void GameOverCheck() { }
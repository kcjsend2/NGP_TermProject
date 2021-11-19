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

// ��Ŷ
#define GAME_START		0b00001
#define GAME_OVER		0b00010
#define PLAYER_UPDATE	0b00100
#define PLAYER_HIT		0b01000
#define BULLET_DELETED	0b10000

// ���� IP, ��Ʈ
#define SERVER_PORT 9000

// ����ü ����
#pragma pack(1)
struct PlayerData
{
    int         m_id;               // ���� ��ȣ
    XMFLOAT3    m_position; 	    // ��ġ
    XMFLOAT3    m_rotate;		    // ȸ�� ����(roll, pitch, yaw)
    bool        m_bulletIsValid;    // �Ѿ��� ��ȿ����
    XMFLOAT3    m_bulletPosition;	// �Ѿ� ��ġ

    PlayerData() : m_id{}, m_position{}, m_rotate{}, m_bulletIsValid{}, m_bulletPosition{}
    {

    }
};

struct ThreadFuncParam
{
    int     id;
    SOCKET  sock;
};

// �۽� �Լ�
void SendGameStart(ThreadFuncParam* param);
void SendGameOver(ThreadFuncParam* param) { }
void SendPlayerInfo(ThreadFuncParam* param);
void SendBulletDeleted(ThreadFuncParam* param) { }

// �� �� �Լ�
int RecvN(const SOCKET& socket, char* buffer, int length, int flags);
BoundingOrientedBox GetBoundingBox(INT id) { }
bool BulletCollisionCheck(XMFLOAT3 playerPosition, XMFLOAT3 playerRotate, XMFLOAT3 BulletPosition);
bool isGameOver() { }
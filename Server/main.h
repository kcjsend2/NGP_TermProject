#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS

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
	XMFLOAT3 m_position; 	// �÷��̾� ��ġ
	XMFLOAT3 m_rotate;		// �÷��̾� ȸ�� ����(roll, pitch, yaw)
	int m_life;			// �÷��̾��� ��� ��
	bool m_bHasBullet;		// �Ѿ� ����
	XMFLOAT3 m_bulletPosition;	// �Ѿ� ��ġ

	PlayerData()
	{
		m_position = { 0.0f, 0.0f, 0.0f };
		m_rotate = { 0.0f, 0.0f, 0.0f };
		m_life = 0;
		m_bHasBullet = false;
		m_bulletPosition = { 0.0f, 0.0f, 0.0f };
	}

	PlayerData(XMFLOAT3 Position, XMFLOAT3 rotate, int life, bool hasBullet, XMFLOAT3 bulletPosition)
	{
		m_position = Position;
		m_rotate = rotate;
		m_life = life;
		m_bHasBullet = hasBullet;
		m_bulletPosition = bulletPosition;
	}
};

struct ThreadFuncParam
{
    int     id;
    SOCKET  sock;
};

// ���� �Լ�
void RecvPlayerInfo(ThreadFuncParam* param);

// �۽� �Լ�
void SendGameStart(ThreadFuncParam* param);
void SendGameOver(ThreadFuncParam* param) { }
void SendBulletDeleted(ThreadFuncParam* param) { }
void SendPlayerInfo(ThreadFuncParam* param);

// ������ �Լ�
DWORD WINAPI ProcessClientData(LPVOID arg);

// �� �� �Լ�
int RecvN(const SOCKET& socket, char* buffer, int length, int flags);
BoundingOrientedBox GetBoundingBox(INT id) { return {}; }
bool BulletCollisionCheck(XMFLOAT3 playerPosition, XMFLOAT3 playerRotate, XMFLOAT3 BulletPosition);
bool isGameOver() { return false; }
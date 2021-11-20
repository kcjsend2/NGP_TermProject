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

// ��Ŷ
#define GAME_START      0b000001
#define GAME_OVER       0b000010
#define PLAYER_MOVE     0b000100
#define PLAYER_HIT      0b001000
#define BULLET_UPDATE   0b010000
#define BULLET_DELETE   0b100000

// �ٸ� Ŭ���̾�Ʈ�� ���� ���� ����ü
#pragma pack(1)
struct PlayerData
{
    int         m_id;               // ���� ��ȣ
    XMFLOAT3    m_position; 	    // ��ġ
    XMFLOAT3    m_rotate;		    // ȸ�� ����(roll, pitch, yaw)
    bool        m_bulletIsValid;    // �Ѿ��� ��ȿ����
    XMFLOAT3    m_bulletPosition;	// �Ѿ� ��ġ

    PlayerData() : m_id{}, m_position {}, m_rotate{}, m_bulletIsValid{}, m_bulletPosition{}
    {

    }
};

// �Լ� ����
void err_quit(char* msg);
void err_display(char* msg);
int recvn(const SOCKET& socket, char* buffer, int length, int flags);

void RecvType(const SOCKET& sock);
void RecvGameStart() { }
void RecvGameOver() { }
void RecvPlayerMove() { }
void RecvPlayerHit() { }
void RecvPlayerBullet() { }
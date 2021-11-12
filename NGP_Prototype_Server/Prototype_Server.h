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

    PlayerData() : m_id{}, m_position{}, m_rotate{}, m_bulletIsValid{}, m_bulletPosition{}
    {

    }
};

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

// 패킷
#define GAME_START      0b000001
#define GAME_OVER       0b000010
#define PLAYER_MOVE     0b000100
#define PLAYER_HIT      0b001000
#define BULLET_UPDATE   0b010000
#define BULLET_DELETE   0b100000

// 다른 클라이언트에 대한 정보 구조체
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

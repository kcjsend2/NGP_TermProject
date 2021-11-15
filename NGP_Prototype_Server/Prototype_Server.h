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

//패킷
#define GAME_START 0b00001
#define GAME_OVER 0b00010
#define PLAYER_UPDATE 0b00100
#define PLAYER_HIT 0b01000
#define BULLET_DELETED 0b10000

#pragma pack(1)
struct PlayerData
{
	XMFLOAT3 m_position; 	// 플레이어 위치
	XMFLOAT3 m_rotate;		// 플레이어 회전 정보(roll, pitch, yaw)
	int m_life;			// 플레이어의 목숨 수
	bool m_bHasBullet;		// 총알 유무
	XMFLOAT3 m_bulletPosition;	// 총알 위치

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

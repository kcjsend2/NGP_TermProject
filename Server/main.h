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

// 패킷
#define GAME_START		0b00001
#define GAME_OVER		0b00010
#define PLAYER_UPDATE	0b00100
#define PLAYER_HIT		0b01000
#define BULLET_DELETED	0b10000

// 서버 포트
#define SERVER_PORT		9000

// 구조체 선언
#pragma pack(1)
struct PlayerData
{
	XMFLOAT3 position; 			// 플레이어 위치
	XMFLOAT3 rotate;			// 플레이어 회전 정보(roll, pitch, yaw)
	int		 life;				// 플레이어의 목숨 수
	bool	 hasBullet;			// 총알 유무
	XMFLOAT3 bulletPosition;	// 총알 위치

	PlayerData() : position{}, rotate{}, life{}, hasBullet{}, bulletPosition{}
	{

	}

	PlayerData(XMFLOAT3 _position, XMFLOAT3 _rotate, int _life, bool _hasBullet, XMFLOAT3 _bulletPosition)
		: position{ _position }, rotate{ _rotate }, life{ _life }, hasBullet{ _hasBullet }, bulletPosition{ _bulletPosition }
	{

	}
};

struct ThreadFuncParam
{
	int     id;
	SOCKET  sock;
};

// 수신 함수
int RecvN(const SOCKET& socket, char* buffer, int length, int flags);
void RecvPlayerInfo(ThreadFuncParam* param);

// 송신 함수
void SendGameStart(ThreadFuncParam* param);
void SendPlayerInfo(ThreadFuncParam* param, int msg);

// 쓰레드 함수
DWORD WINAPI ProcessClientData(LPVOID arg);
DWORD WINAPI CheckGameOver(LPVOID arg);

// 그 외 함수
bool BulletCollisionCheck(XMFLOAT3 playerPosition, XMFLOAT3 playerRotate, XMFLOAT3 bulletPosition);
void CheckBulletDeleted(ThreadFuncParam* param, int& msg);
void CheckPlayerHit(ThreadFuncParam* param, int& msg);
bool isGameOver();
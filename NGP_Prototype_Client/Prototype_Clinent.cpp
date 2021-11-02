#define _WINSOCK_DEPRECATED_NO_WARNINGS // �ֽ� VC++ ������ �� ��� ����
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <cassert>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <array>
#include <iostream>

using namespace DirectX;

using namespace std;

enum dataType
{
    GAME_START,
    GAME_OVER,
    PLAYER_STATUS
};

// �׻� PlayerData �������� �����Ͱ� �ۼ��ŵȴ�.
#pragma pack(1)
struct PlayerData
{
    int m_dType;
    XMFLOAT3 m_position; 	// �÷��̾� ��ġ
    XMFLOAT3 m_rotate;		// �÷��̾� ȸ�� ����(roll, pitch, yaw)
    int m_life;			// �÷��̾��� ��� ��
    bool m_bHasBullet;		// �Ѿ� ����
    XMFLOAT3 m_bulletPosition;	// �Ѿ� ��ġ
    bool m_bIntersected;

    PlayerData()
    {
        m_dType = 999;
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

// ���� �Լ� ���� ��� �� ����
void err_quit(char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

// ���� �Լ� ���� ���
void err_display(char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
    printf("[%s] %s", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

int recvn(SOCKET s, char* buf, int len, int flags)
{
    int received;
    char* ptr = buf;
    int left = len;

    // ���� ����Ʈ�� 0�̸� ���� ����
    while (left > 0)
    {
        received = recv(s, ptr, left, flags);
        if (received == SOCKET_ERROR)
        {
            return SOCKET_ERROR;
        }
        else if (received == 0)
        {
            break;
        }

        left -= received;
        ptr += received;
    }

    // �ޱ�� �� ����Ʈ �� - ���� ���� ����Ʈ �� == ���� ����Ʈ ��
    // ���ݱ��� ���� �� ����Ʈ �� ����
    // ���������� �޾Ҵٸ� left == 0�̹Ƿ� len�� ������, ���� �ʾҴٸ� left == len�̹Ƿ� 0�� ������. ���� ���� ���°� �����ΰ��� �ƴ� (Ŭ���̾�Ʈ���� send���� �ʾ��� �� �����Ƿ�)
    //printf("�ޱ�� �� ����Ʈ : %d\n", len);
    //printf("���� ����Ʈ : %d\n", left);
    return (len - left);
}

int main(int argc, char* argv[])
{
    // 0 : ���� ���� �̸�, 1 : ������ �ּ�, 2 : ��Ʈ ��ȣ

    WSADATA wsa;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // socket()
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");

    // connect()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
    serveraddr.sin_port = htons(atoi(argv[2]));
    if (connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR)
    {
        err_display("connect()");
    }

    array<PlayerData, 2> aOtherPlayerData;
    PlayerData pRecvData;

    // ���� ��ȣ�� ��ٸ�
    while (1)
    {
        recvn(sock, (char*)&pRecvData, sizeof(PlayerData), 0);

        if (pRecvData.m_dType == GAME_START)
        {
            cout << "���� ����" << endl;
            break;
        }
    }

    while (1)
    {
        PlayerData pSendData{ PLAYER_STATUS, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 3, false, {0.0f, 0.0f, 0.0f}, false };
        if (send(sock, (char*)&pSendData, sizeof(PlayerData), 0) == SOCKET_ERROR)
        {
            err_quit("send()");
        }
        
        for (int i = 0; i < 2; ++i)
        {
            cout << endl;
            ZeroMemory(&pRecvData, sizeof(PlayerData));
            recvn(sock, (char*)&pRecvData, sizeof(PlayerData), 0);
            aOtherPlayerData[i] = pRecvData;

            if (pRecvData.m_dType == PLAYER_STATUS)
            {
                aOtherPlayerData[i] = pRecvData;
                cout << "�ٸ� �÷��̾� ��ġ "  << i << ": "<< aOtherPlayerData[i].m_position.x << ", " << aOtherPlayerData[i].m_position.y << ", " << aOtherPlayerData[i].m_position.z << endl;
            }
            else if (pRecvData.m_dType == GAME_OVER)
            {
                cout << "���� ����" << endl;
                break;
            }
        }
    }

    closesocket(sock);

    WSACleanup();

    return 0;
}
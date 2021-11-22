#include "main.h"

// 전역 변수 선언
array<HANDLE, 3> g_events;
array<PlayerData, 3> g_players;

int main()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        MessageBox(NULL, TEXT("윈속 초기화 실패"), TEXT("오류"), 0);
        exit(0);
    }

    SOCKET sock{ socket(AF_INET, SOCK_STREAM, 0) };
    if (sock == INVALID_SOCKET)
    {
        MessageBox(NULL, TEXT("소켓 생성 실패"), TEXT("오류"), 0);
        exit(0);
    }

    SOCKADDR_IN serveraddr{};
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVER_PORT);
    if (bind(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR)
    {
        MessageBox(NULL, TEXT("bind() 실패"), TEXT("오류"), 0);
        exit(0);
    }

    if (listen(sock, SOMAXCONN) == SOCKET_ERROR)
    {
        MessageBox(NULL, TEXT("listen() 실패"), TEXT("오류"), 0);
        exit(0);
    }

    int addrlen;
    SOCKADDR_IN clientAddr;
    array<SOCKET, 3> clientSock;
    array<HANDLE, 3> hThread;

    // 3명의 플레이어 접속을 대기한다.
    for (int i = 0; i < 3; ++i)
    {
        addrlen = sizeof(SOCKADDR);
        clientSock[i] = accept(sock, (SOCKADDR*)&clientAddr, &addrlen);
        if (clientSock[i] == INVALID_SOCKET) 
        {
            --i;
            continue;
        }
        cout << "[접속]" << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << endl;
    }

    // 쓰레드 동기화를 위한 이벤트 생성
    g_events[0] = CreateEvent(NULL, TRUE, TRUE, TEXT("PLAYER0"));
    g_events[1] = CreateEvent(NULL, TRUE, FALSE, TEXT("PLAYER1"));
    g_events[2] = CreateEvent(NULL, TRUE, FALSE, TEXT("PLAYER2"));

    // 쓰레드 생성
    for (int i = 0; i < 3; ++i)
    {
        ThreadFuncParam* param{ new ThreadFuncParam };
        param->id = i;
        param->sock = clientSock[i];
        hThread[i] = CreateThread(NULL, 0, ProcessClientData, param, 0, NULL);
    }

    // 쓰레드 하나가 종료되었다는 것은 종료 조건을 만족했다는 것이다. 연결되어있는 모든 클라이언트들에게 종료 메시지를 보낸다.
    WaitForSingleObject(hThread[0], INFINITE);

    // 메시지 송신
    int msgType{ GAME_OVER };
    for (int i = 0; i < 3; ++i)
    {
        send(clientSock[i], (char*)&msgType, sizeof(int), 0);
        closesocket(clientSock[i]);
    }
    closesocket(sock);
    WSACleanup();
}

void RecvPlayerInfo(ThreadFuncParam* param)
{
    RecvN(param->sock, (char*)&g_players[param->id], sizeof(PlayerData), 0);

    XMFLOAT3 playerPos = g_players[param->id].m_position;
    cout << "PLAYER" << param->id << " : " << playerPos.x << ", " << playerPos.y << ", " << playerPos.z << endl;

    ///////////////////////
    // 충돌 검사를 한다. //
    ///////////////////////

    ///////////////////////////
    // 게임 종료를 체크한다. //
    ///////////////////////////

    // 다른 플레이어에게 이 플레이어의 정보를 송신한다.
    SendPlayerInfo(param);
}

DWORD WINAPI ProcessClientData(LPVOID arg)
{
    ThreadFuncParam* param{ reinterpret_cast<ThreadFuncParam*>(arg) };

    // 게임 시작 메시지를 송신한다.
    SendGameStart(param);

    while (TRUE)
    {
        // 자신의 이벤트가 신호 상태가 될 때까지 대기
        WaitForSingleObject(g_events[param->id], INFINITE);

        // 메시지 타입을 수신한다.
        int msg;
        RecvN(param->sock, (char*)&msg, sizeof(int), 0);

        // 메시지 타입이 PLAYER_UPDATE라면 플레이어 정보 구조체를 수신한다.
        if (msg & PLAYER_UPDATE)
        {
            RecvPlayerInfo(param);
        }

        // 자신의 이벤트 비신호 상태로 변경
        ResetEvent(g_events[param->id]);

        // 다음 이벤트 신호 상태로 변경
        SetEvent(g_events[(param->id + 1) % g_events.size()]);
    }
    return 0;
}

int RecvN(const SOCKET& socket, char* buffer, int length, int flags)
{
    char* ptr{ buffer };
    int received{ 0 }, left{ length };

    while (left > 0)
    {
        received = recv(socket, ptr, left, flags);
        if (received == SOCKET_ERROR)
            return SOCKET_ERROR;
        if (received == 0)
            break;
        left -= received;
        ptr += received;
    }
    return length - left;
}

void SendPlayerInfo(ThreadFuncParam* param)
{
    // 메시지 송신
    int msg{ PLAYER_UPDATE };
    send(param->sock, (char*)&msg, sizeof(int), 0);

    // 플레이어 정보 구조체 송신
    for (int i = 0; i < 3; ++i)
    {
        if (i == param->id)
            continue;
        send(param->sock, (char*)&g_players[i], sizeof(PlayerData), 0);
    }
}

void SendGameStart(ThreadFuncParam* param)
{
    // 메시지 송신
    int msg{ GAME_START };
    send(param->sock, (char*)&msg, sizeof(int), 0);
   
    // 스폰 좌표 송신
    XMFLOAT3 spawnPosition[]{
        { 400.0f, 5.0f, 200.0f },
        { 400.0f, 5.0f, 300.0f },
        { 400.0f, 5.0f, 400.0f }
    };
    send(param->sock, (char*)&spawnPosition[param->id], sizeof(XMFLOAT3), 0);
}

bool BulletCollisionCheck(XMFLOAT3 playerPosition, XMFLOAT3 playerRotate, XMFLOAT3 BulletPosition)
{
    BoundingOrientedBox BBPlayer{ playerPosition, XMFLOAT3{ 4.5f, 1.1f, 4.5f }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
    BoundingOrientedBox BBBullet{ BulletPosition, XMFLOAT3{ 1.1f, 1.1f, 1.1f }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };

    BBPlayer.Transform(BBPlayer, 1.0f, XMLoadFloat3(&playerRotate), {});
    BBBullet.Transform(BBBullet, 1.0f, {}, {});

    return BBPlayer.Intersects(BBBullet);
}
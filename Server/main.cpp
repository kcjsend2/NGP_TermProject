#include "main.h"

// ���� ���� ����
array<HANDLE, 3> g_events;
array<PlayerData, 3> g_players;

int main()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        MessageBox(NULL, TEXT("���� �ʱ�ȭ ����"), TEXT("����"), 0);
        exit(0);
    }

    SOCKET sock{ socket(AF_INET, SOCK_STREAM, 0) };
    if (sock == INVALID_SOCKET)
    {
        MessageBox(NULL, TEXT("���� ���� ����"), TEXT("����"), 0);
        exit(0);
    }

    SOCKADDR_IN serveraddr{};
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVER_PORT);
    if (bind(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR)
    {
        MessageBox(NULL, TEXT("bind() ����"), TEXT("����"), 0);
        exit(0);
    }

    if (listen(sock, SOMAXCONN) == SOCKET_ERROR)
    {
        MessageBox(NULL, TEXT("listen() ����"), TEXT("����"), 0);
        exit(0);
    }

    int addrlen;
    SOCKADDR_IN clientAddr;
    array<SOCKET, 3> clientSock;
    array<HANDLE, 3> hThread;

    // 3���� �÷��̾� ������ ����Ѵ�.
    for (int i = 0; i < 3; ++i)
    {
        addrlen = sizeof(SOCKADDR);
        clientSock[i] = accept(sock, (SOCKADDR*)&clientAddr, &addrlen);
        if (clientSock[i] == INVALID_SOCKET) 
        {
            --i;
            continue;
        }
        cout << "[����]" << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << endl;
    }

    // ������ ����ȭ�� ���� �̺�Ʈ ����
    g_events[0] = CreateEvent(NULL, TRUE, TRUE, TEXT("PLAYER0"));
    g_events[1] = CreateEvent(NULL, TRUE, FALSE, TEXT("PLAYER1"));
    g_events[2] = CreateEvent(NULL, TRUE, FALSE, TEXT("PLAYER2"));

    // ������ ����
    for (int i = 0; i < 3; ++i)
    {
        ThreadFuncParam* param{ new ThreadFuncParam };
        param->id = i;
        param->sock = clientSock[i];
        hThread[i] = CreateThread(NULL, 0, ProcessClientData, param, 0, NULL);
    }

    // ������ �ϳ��� ����Ǿ��ٴ� ���� ���� ������ �����ߴٴ� ���̴�. ����Ǿ��ִ� ��� Ŭ���̾�Ʈ�鿡�� ���� �޽����� ������.
    WaitForSingleObject(hThread[0], INFINITE);

    // �޽��� �۽�
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
    // �浹 �˻縦 �Ѵ�. //
    ///////////////////////

    ///////////////////////////
    // ���� ���Ḧ üũ�Ѵ�. //
    ///////////////////////////

    // �ٸ� �÷��̾�� �� �÷��̾��� ������ �۽��Ѵ�.
    SendPlayerInfo(param);
}

DWORD WINAPI ProcessClientData(LPVOID arg)
{
    ThreadFuncParam* param{ reinterpret_cast<ThreadFuncParam*>(arg) };

    // ���� ���� �޽����� �۽��Ѵ�.
    SendGameStart(param);

    while (TRUE)
    {
        // �ڽ��� �̺�Ʈ�� ��ȣ ���°� �� ������ ���
        WaitForSingleObject(g_events[param->id], INFINITE);

        // �޽��� Ÿ���� �����Ѵ�.
        int msg;
        RecvN(param->sock, (char*)&msg, sizeof(int), 0);

        // �޽��� Ÿ���� PLAYER_UPDATE��� �÷��̾� ���� ����ü�� �����Ѵ�.
        if (msg & PLAYER_UPDATE)
        {
            RecvPlayerInfo(param);
        }

        // �ڽ��� �̺�Ʈ ���ȣ ���·� ����
        ResetEvent(g_events[param->id]);

        // ���� �̺�Ʈ ��ȣ ���·� ����
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
    // �޽��� �۽�
    int msg{ PLAYER_UPDATE };
    send(param->sock, (char*)&msg, sizeof(int), 0);

    // �÷��̾� ���� ����ü �۽�
    for (int i = 0; i < 3; ++i)
    {
        if (i == param->id)
            continue;
        send(param->sock, (char*)&g_players[i], sizeof(PlayerData), 0);
    }
}

void SendGameStart(ThreadFuncParam* param)
{
    // �޽��� �۽�
    int msg{ GAME_START };
    send(param->sock, (char*)&msg, sizeof(int), 0);
   
    // ���� ��ǥ �۽�
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
#include "main.h"

// ���� ���� ����
array<HANDLE, 4> g_events;      // ������ ����ȭ�� ���� �̺�Ʈ ��ü
array<HANDLE, 3> g_threads;     // �÷��̾� ���� �ۼ��� ������ �ڵ�
array<PlayerData, 3> g_players; // �÷��̾� ����

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

    //bool flag = TRUE;
    //setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));

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
    g_events[3] = CreateEvent(NULL, TRUE, FALSE, TEXT("GAMEOVER"));

    // �÷��̾� ���� �ۼ��� ������ ����
    for (int i = 0; i < 3; ++i)
    {
        ThreadFuncParam* param{ new ThreadFuncParam };
        param->id = i;
        param->sock = clientSock[i];
        g_threads[i] = CreateThread(NULL, 0, ProcessClientData, param, 0, NULL);
    }

    // ���� ���� Ȯ�� ������
    CreateThread(NULL, 0, CheckGameOver, NULL, 0, NULL);

    // �÷��̾� ���� �ۼ��� �����尡 ����� ������ ����Ѵ� : �� ��쿣 ������ ���� ���̴�.
    WaitForMultipleObjects(g_threads.size(), g_threads.data(), TRUE, INFINITE);

    // �޽��� �۽�
    int msg{ GAME_OVER };
    for (const SOCKET& s : clientSock)
    {
        send(s, (char*)&msg, sizeof(int), 0);
        closesocket(s);
    }
    closesocket(sock);
    WSACleanup();
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

void RecvPlayerInfo(ThreadFuncParam* param)
{
    RecvN(param->sock, (char*)&g_players[param->id], sizeof(PlayerData), 0);

    //XMFLOAT3 playerPos = g_players[param->id].position;
    //cout << "PLAYER" << param->id << " : " << playerPos.x << ", " << playerPos.y << ", " << playerPos.z << endl;

    int msg{ 0 };

    // �� �Ѿ��� ���� �ǰݽ��״� �� �˻�
    CheckBulletDeleted(param, msg);

    // ���� ���� �Ѿ˿� �ǰݴ��ߴ� �� �˻�
    CheckPlayerHit(param, msg);

    // �ٸ� �÷��̾�� �� �÷��̾��� ������ �۽��Ѵ�.
    SendPlayerInfo(param, msg);
}

void SendPlayerInfo(ThreadFuncParam* param, int msg)
{
    // �޽��� �۽� : �Ķ���ͷ� ���� msg�� PLAYER_UPDATE ��Ŷ�� �߰��Ѵ�.
    msg |= PLAYER_UPDATE;
    send(param->sock, (char*)&msg, sizeof(int), 0);

    // �÷��̾� ���� ����ü �۽�
    for (int i = 0; i < 3; ++i)
    {
        if (i == param->id) continue;
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
        { 405.0f, 5.0f, 200.0f },
        { 395.0f, 5.0f, 200.0f }
    };
    send(param->sock, (char*)&spawnPosition[param->id], sizeof(XMFLOAT3), 0);
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

DWORD WINAPI CheckGameOver(LPVOID arg)
{
    while (TRUE)
    {
        // ���� ���� üũ �̺�Ʈ�� Ȱ��ȭ �� ������ ���
        WaitForSingleObject(g_events[3], INFINITE);

        // ������ �����ٸ� �÷��̾� ���� �ۼ��� ������ ���� ����
        if (isGameOver())
        {
            for (const HANDLE& h : g_threads)
                TerminateThread(h, 0);
            ExitThread(0);
            return 0;
        }

        // �ڽ��� �̺�Ʈ ���ȣ ���·� ����
        ResetEvent(g_events[3]);

        // PLAYER0 �̺�Ʈ ��ȣ ���·� ����
        SetEvent(g_events[0]);
    }
    return 0;
}

void CheckBulletDeleted(ThreadFuncParam* param, int& msg)
{
    for (int i = 0; i < g_players.size(); ++i)
    {
        if (param->id == i) continue;
        if (BulletCollisionCheck(g_players[i].position, g_players[i].rotate, g_players[param->id].bulletPosition))
        {
            msg |= BULLET_DELETED;
            break;
        }
    }
}

void CheckPlayerHit(ThreadFuncParam* param, int& msg)
{
    for (int i = 0; i < g_players.size(); ++i)
    {
        if (param->id == i) continue;
        if (BulletCollisionCheck(g_players[param->id].position, g_players[param->id].rotate, g_players[i].bulletPosition))
        {
            msg |= PLAYER_HIT;
            break;
        }
    }
}

bool isGameOver()
{
    int cnt = 0;
    for (int i = 0; i < 3; ++i)
        if (g_players[i].life != 0)
            ++cnt;
    if (cnt <= 1)
        return true;
    return false;
}

bool BulletCollisionCheck(XMFLOAT3 playerPosition, XMFLOAT4 playerRotate, XMFLOAT3 bulletPosition)
{
    BoundingOrientedBox BBPlayer{ XMFLOAT3{}, XMFLOAT3{ 4.5f, 1.1f, 4.5f }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
    BoundingOrientedBox BBBullet{ XMFLOAT3{}, XMFLOAT3{ 1.1f, 1.1f, 1.1f }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };

    XMMATRIX rotate{ XMMatrixRotationQuaternion(XMLoadFloat4(&playerRotate)) };
    XMMATRIX trans{ XMMatrixTranslation(playerPosition.x, playerPosition.y, playerPosition.z) };
    BBPlayer.Transform(BBPlayer, rotate * trans);

    trans = XMMatrixTranslation(bulletPosition.x, bulletPosition.y, bulletPosition.z);
    BBBullet.Transform(BBBullet, trans);

    return BBPlayer.Intersects(BBBullet);
}
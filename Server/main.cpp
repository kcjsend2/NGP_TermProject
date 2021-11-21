#include "main.h"

// ���� ���� ����
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

    int msgType{ GAME_OVER };
    for (int i = 0; i < 3; ++i)
    {
        send(clientSock[i], (char*)&msgType, sizeof(PlayerData), 0);
        closesocket(clientSock[i]);
    }
    closesocket(sock);
    WSACleanup();
}

void RecvPlayerInfo(ThreadFuncParam* param)
{
    PlayerData playerData;
    RecvN(param->sock, (char*)&playerData, sizeof(PlayerData), 0);
    g_players[param->id] = playerData;

    XMFLOAT3 playerPos = g_players[param->id].m_position;

    printf("%d�� �÷��̾� ��ġ : %f, %f, %f\n", param->id, playerPos.x, playerPos.y, playerPos.z);
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
        ///////////////////////////////////////////////
        // 0�� �̺�Ʈ�� ��ȣ ���°� �Ǳ⸦ ����Ѵ�. //
        ///////////////////////////////////////////////
        
        // �޽��� Ÿ���� �����Ѵ�.
        int msg = 0;
        RecvN(param->sock, (char*)&msg, sizeof(int), 0);
        printf("%d�� ������ �޽��� ����, �޽��� ID: %d\n", param->id, msg);


        // �޽��� Ÿ���� PLAYER_UPDATE��� �÷��̾� ���� ����ü�� �����Ѵ�.
        if (msg & PLAYER_UPDATE)
        {
            RecvPlayerInfo(param);
        }

        ////////////////////////////////////////
        // 0�� �̺�Ʈ�� ���ȣ ���·� �ٲ۴�. //
        // 1�� �̺�Ʈ�� ��ȣ ���·� �ٲ۴�.   //
        ////////////////////////////////////////
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
   
    XMFLOAT3 SpawnPosition[3]{ {100.0f, 5.0f, 100.0f}, {200.0f, 5.0f, 200.0f}, {300.0f, 5.0f, 300.0f} };  // �� ó�� ���� ��ǥ ����. ���� �� �� ������� �� ��. �׳� ������ ���� ���

    //���� ��ǥ �۽�
    send(param->sock, (char*)&SpawnPosition[param->id], sizeof(XMFLOAT3), 0);

    printf("%d�� ������ ���� ���� �۽�\n", param->id);
}

bool BulletCollisionCheck(XMFLOAT3 playerPosition, XMFLOAT3 playerRotate, XMFLOAT3 BulletPosition)
{
    BoundingOrientedBox BBPlayer{ playerPosition, XMFLOAT3{ 4.5f, 1.1f, 4.5f }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
    BoundingOrientedBox BBBullet{ BulletPosition, XMFLOAT3{ 1.1f, 1.1f, 1.1f }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };

    BBPlayer.Transform(BBPlayer, 1.0f, XMLoadFloat3(&playerRotate), {});
    BBBullet.Transform(BBBullet, 1.0f, {}, {});

    return BBPlayer.Intersects(BBBullet);
}
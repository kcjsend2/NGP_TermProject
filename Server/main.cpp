#include "main.h"

// 전역 변수 선언
array<HANDLE, 4> g_events; // 쓰레드 동기화를 위한 이벤트 객체
array<HANDLE, 3> g_threads; // 플레이어 정보 송수신 쓰레드 핸들
array<pair<SOCKET, PlayerData>, 3> g_players; // 플레이어 소켓과 정보

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

    // 3명의 플레이어 접속을 대기한다.
    for (int i = 0; i < 3; ++i)
    {
        addrlen = sizeof(SOCKADDR);
        g_players[i] = make_pair(accept(sock, (SOCKADDR*)&clientAddr, &addrlen), PlayerData{});
        if (g_players[i].first == INVALID_SOCKET)
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
    g_events[3] = CreateEvent(NULL, TRUE, FALSE, TEXT("GAMEOVER"));

    // 플레이어 정보 송수신 쓰레드 생성
    for (int i = 0; i < 3; ++i)
    {
        int* index{ new int(i) };
        g_threads[i] = CreateThread(NULL, 0, ProcessClientData, index, 0, NULL);
    }

    // 게임 종료 확인 쓰레드
    CreateThread(NULL, 0, CheckGameOver, NULL, 0, NULL);

    // 플레이어 정보 송수신 쓰레드가 종료될 때(게임 종료)까지 대기한다
    WaitForMultipleObjects(g_threads.size(), g_threads.data(), TRUE, INFINITE);

    // 게임 종료 메시지 송신
    int msg{ GAME_OVER };
    for (const auto [s, _] : g_players)
    {
        send(s, (char*)&msg, sizeof(int), 0);
        closesocket(s);
    }
    closesocket(sock);
    WSACleanup();
}

DWORD WINAPI ProcessClientData(LPVOID arg)
{
    int index{ *reinterpret_cast<int*>(arg) };

    // 게임 시작 메시지를 송신한다.
    SendGameStart(index);

    while (TRUE)
    {
        // 자신의 이벤트가 신호 상태가 될 때까지 대기
        WaitForSingleObject(g_events[index], INFINITE);

        // 메시지 타입을 수신한다.
        int msg;
        RecvN(g_players[index].first, (char*)&msg, sizeof(int), 0);

        // 메시지 타입이 PLAYER_UPDATE라면 플레이어 정보 구조체를 수신한다.
        if (msg & PLAYER_UPDATE)
        {
            RecvPlayerInfo(index);
        }

        // 자신의 이벤트 비신호 상태로 변경
        ResetEvent(g_events[index]);

        // 다음 이벤트 신호 상태로 변경
        SetEvent(g_events[(index + 1) % g_events.size()]);
    }
    return 0;
}

DWORD WINAPI CheckGameOver(LPVOID arg)
{
    while (TRUE)
    {
        // 게임 종료 체크 이벤트가 활성화 될 때까지 대기
        WaitForSingleObject(g_events[3], INFINITE);

        // 게임이 종료됬다면 플레이어 정보 송수신 쓰레드 강제 종료
        if (isGameOver())
        {
            for (const HANDLE& h : g_threads)
                TerminateThread(h, 0);
            ExitThread(0);
            return 0;
        }

        // 자신의 이벤트 비신호 상태로 변경
        ResetEvent(g_events[3]);

        // PLAYER0 이벤트 신호 상태로 변경
        SetEvent(g_events[0]);
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

void RecvPlayerInfo(int index)
{
    /*
    * RecvPlayerInfo 함수에서는 다음과 같은 일을 수행한다.
    * 1. 플레이어 정보 구조체를 수신하고 저장한다.
    * 2. 수신한 플레이어의 총알이 다른 플레이어를 피격시켰는가?
    * 3. 수신한 플레이어가 다른 플레이어의 총알에 피격당했는가?
    * 4. 1, 2번의 결과를 수신한 플레이어에게 송신한다.
    * 5. 수신한 플레이어의 정보를 다른 플레이어들에게 송신한다.
    */

    RecvN(g_players[index].first, (char*)&g_players[index].second, sizeof(PlayerData), 0);

    int msg{ NULL };

    // 남의 총알에 피격당했는 지 검사
    if (isPlayerHit(index))
        msg |= PLAYER_HIT;

    // 총알이 남을 피격시켰는 지 검사
    if (isBulletHit(index))
        msg |= BULLET_DELETED;

    // 메시지 전송
    if (msg) send(g_players[index].first, (char*)&msg, sizeof(int), 0);

    // 다른 플레이어의 정보 송신
    SendPlayerInfo(index);
}

void SendGameStart(int index)
{
    // 스폰 좌표
    XMFLOAT3 spawnPosition[]{
        { 400.0f, 5.0f, 200.0f },
        { 405.0f, 5.0f, 200.0f },
        { 395.0f, 5.0f, 200.0f }
    };

    // 메시지 송신
    int msg{ GAME_START };
    send(g_players[index].first, (char*)&msg, sizeof(int), 0);
    send(g_players[index].first, (char*)&spawnPosition[index], sizeof(XMFLOAT3), 0);
}

void SendPlayerInfo(int index)
{
    int msg{ PLAYER_UPDATE };
    send(g_players[index].first, (char*)&msg, sizeof(int), 0);

    for (int i = 0; i < 3; ++i)
    {
        if (i == index) continue;
        send(g_players[index].first, (char*)&g_players[i].second, sizeof(PlayerData), 0);
    }
}

bool isCollided(int playerIndex, int bulletIndex)
{
    /*
    * isCollided 함수에서는 다음과 같은 일을 수행한다.
    * 1. 플레이어 인덱스와 총알 인덱스를 이용해서 플레이어의 위치, 회전 정보와 총알 위치 정보를 가져온다.
    * 2. 플레이어 바운딩박스를 만든다.
    * 3. 총알 바운딩박스를 만든다.
    * 4. 충돌 검사 결과를 반환한다.
    */

    BoundingOrientedBox BBPlayer{ XMFLOAT3{}, XMFLOAT3{ 4.5f, 1.1f, 4.5f }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
    BoundingOrientedBox BBBullet{ XMFLOAT3{}, XMFLOAT3{ 1.1f, 1.1f, 1.1f }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };

    // 플레이어 위치, 회전, 총알 위치
    XMFLOAT4 playerRotate{ g_players[playerIndex].second.rotate };
    XMFLOAT3 playerPosition{ g_players[playerIndex].second.position };
    XMFLOAT3 bulletPosition{ g_players[bulletIndex].second.position };

    // 플레이어 바운딩박스
    XMMATRIX rotate{ XMMatrixRotationQuaternion(XMLoadFloat4(&playerRotate)) };
    XMMATRIX trans{ XMMatrixTranslation(playerPosition.x, playerPosition.y, playerPosition.z) };
    BBPlayer.Transform(BBPlayer, rotate * trans);

    // 총알 바운딩박스
    trans = XMMatrixTranslation(bulletPosition.x, bulletPosition.y, bulletPosition.z);
    BBBullet.Transform(BBBullet, trans);

    return BBPlayer.Intersects(BBBullet);
}

bool isPlayerHit(int playerIndex)
{
    for (int i = 0; i < g_players.size(); ++i)
    {
        if (i == playerIndex) continue;
        if (isCollided(playerIndex, i))
            return true;
    }
    return false;
}

bool isBulletHit(int bulletIndex)
{
    for (int i = 0; i < g_players.size(); ++i)
    {
        if (i == bulletIndex) continue;
        if (isCollided(i, bulletIndex))
            return true;
    }
    return false;
}

bool isGameOver()
{
    return false;

    // 생존자 수
    int nSurvivor = count_if(g_players.begin(), g_players.end(), [](const auto& p) {
        if (p.second.life > 0)
            return true;
        return false;
        });

    // 생존자가 1명 이하일 경우 게임 종료
    return nSurvivor <= 1 ? true : false;
}
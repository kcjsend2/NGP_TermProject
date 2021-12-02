#include "Connection.h"

DWORD WINAPI TransportData(LPVOID arg)
{
    SOCKET sock{ (SOCKET)arg };
    int msg{};

    // 시작 신호를 기다림
    while (TRUE)
    {
        RecvN(sock, (char*)&msg, sizeof(int), 0);
        if (msg & GAME_START)
        {
            RecvGameStart(sock);
            break;
        }
    }

    while (TRUE)
    {
        // 송수신 쓰레드 차례가 될 때까지 대기
        WaitForSingleObject(g_events[1], INFINITE);

        // 서버로 플레이어 정보 송신
        SendPlayerInfo(sock);

        // 서버로부터 메시지 수신
        RecvN(sock, (char*)&msg, sizeof(int), 0);
        gGameFramework.m_pPlayer->SetNextFrameMessage(msg);

        // 다른 플레이어 정보를 수신했을 경우
        if (msg & PLAYER_UPDATE)
        {
            RecvPlayerInfo(sock);
        }
        
        // 게임 종료를 수신했을 경우
        if (msg & GAME_OVER)
        {
            RecvGameOver(sock);
            ResetEvent(g_events[1]);
            SetEvent(g_events[0]);
            break;
        }

        ResetEvent(g_events[1]);
        SetEvent(g_events[0]);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}

void SendPlayerInfo(const SOCKET& sock)
{
    // 플레이어 정보 :: 위치, 회전, 목숨, 총알유효, 총알좌표
    PlayerData playerInfo{ gGameFramework.GetPlayerPosition(), gGameFramework.GetPlayerRotation(), gGameFramework.GetPlayerLife(), FALSE, { 0, 0, 0 } };
    if (std::shared_ptr<CBullet> bullet = gGameFramework.GetPlayerBullet())
    {
        playerInfo.m_bHasBullet = TRUE;
        playerInfo.m_bulletPosition = bullet->GetPosition();
    }

    // 목숨이 없을 경우 관전모드로 변경
    if (gGameFramework.GetPlayerLife() == 0)
        playerInfo.m_position = { 0, 0, -300 };

    // PLAYER_UPDATE 패킷과 함께 송신
    int msg{ PLAYER_UPDATE };
    send(sock, (char*)&msg, sizeof(int), 0);
    send(sock, (char*)&playerInfo, sizeof(PlayerData), 0);
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

void RecvGameStart(const SOCKET& sock)
{
    // 게임시작을 알림
    g_bGameStarted = true;

    // 스폰좌표 수신 후 플레이어 위치 설정
    XMFLOAT3 StartPos;
    RecvN(sock, (char*)&StartPos, sizeof(XMFLOAT3), 0);
    gGameFramework.m_pPlayer->SetRigidBodyPosition(StartPos);
    gGameFramework.m_pPlayer->SetSpawPosition(StartPos);
}

void RecvPlayerInfo(const SOCKET& sock)
{
    RecvN(sock, (char*)&g_otherPlayersData[0], sizeof(PlayerData), 0);
    RecvN(sock, (char*)&g_otherPlayersData[1], sizeof(PlayerData), 0);
}

void RecvGameOver(const SOCKET& sock)
{
    // 게임 종료를 알림
    g_bGameOver = true;

    int msg{ GAME_OVER };
    send(sock, (char*)&msg, sizeof(int), 0);

    if (gGameFramework.GetPlayerLife() > 0)
        MessageBox(NULL, TEXT("승리"), TEXT("게임종료"), 0);
    else
        MessageBox(NULL, TEXT("패배"), TEXT("게임종료"), 0);
}

void InitConnection()
{
    // 명령행인자
    LPWSTR* szArgList; int argCount;
    szArgList = CommandLineToArgvW(GetCommandLine(), &argCount);

    // 명령행 인자로부터 ip, port계산
    char ip[15 + 1], port[4 + 1];
    wcstombs(ip, szArgList[1], wcslen(szArgList[1]) + 1);
    wcstombs(port, szArgList[2], wcslen(szArgList[2]) + 1);

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        MessageBox(NULL, TEXT("윈속 초기화 실패"), TEXT("오류"), 0);
        exit(0);
    }

    SOCKET sock{ socket(AF_INET, SOCK_STREAM, 0) };
    if (sock == INVALID_SOCKET)
    {
        MessageBox(NULL, TEXT("소켓 초기화 실패"), TEXT("오류"), 0);
        exit(0);
    }

    // 네이글 알고리즘 끔
    BOOL flag{ TRUE };
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(BOOL));

    SOCKADDR_IN serveraddr{};
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(ip);
    serveraddr.sin_port = htons(atoi(port));
    if (connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR)
    {
        MessageBox(NULL, TEXT("소켓 연결 실패"), TEXT("오류"), 0);
        exit(0);
    }
    CreateThread(NULL, 0, TransportData, (LPVOID)sock, 0, NULL);
}
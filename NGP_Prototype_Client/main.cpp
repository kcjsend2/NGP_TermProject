#include "main.h"

int main(int argc, char* argv[])
{
    // 0 : 실행 파일 이름
    // 1 : 아이피 주소
    // 2 : 포트 번호

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // socket()
    SOCKET sock{ socket(AF_INET, SOCK_STREAM, 0) };
    if (sock == INVALID_SOCKET)
        err_quit("socket()");

    // connect()
    SOCKADDR_IN serveraddr{};
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
    serveraddr.sin_port = htons(atoi(argv[2]));
    if (connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR)
        err_display("connect()");

    while (TRUE)
    {
        RecvType(sock);
    }

    closesocket(sock);
    WSACleanup();
}

void err_quit(char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

void err_display(char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
    printf("[%s] %s", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

int recvn(const SOCKET& socket, char* buffer, int length, int flags)
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

void RecvType(const SOCKET& sock)
{
    INT packetType;
    recvn(sock, (char*)&packetType, sizeof(packetType), 0);

    if (packetType && GAME_START)
    {
        RecvGameStart();
        // 1. 고유 아이디(int) 수신
        // 2. 스폰 위치(XMFLOAT3) 수신
        // 3. 고유 아이디, 스폰 위치 저장, 스폰 위치로 이동
        cout << "게임시작" << endl;
    }
    if (packetType && GAME_OVER)
    {
        RecvGameOver();
        // 1. 승패 여부(bool) 수신
        // 2. 승패 여부를 메시지 박스로 출력
        cout << "게임오버" << endl;
    }
    if ((packetType && PLAYER_MOVE) || (packetType && BULLET_UPDATE))
    {
        // 1. 플레이어 정보(PlayerData) 수신
        
        // PLAYER_MOVE 플래그가 설정되어 있다면
        if (packetType && PLAYER_MOVE)
        {
            // 2. 해당 id의 플레이어 위치 설정
            // 3. 해당 id의 플레이어 회전 설정
            cout << "플레이어 움직임" << endl;
        }

        // BULLET_UPDATE 플래그가 설정되어 있다면
        if (packetType && BULLET_UPDATE)
        {
            // 4. 해당 id의 플레이어의 총알 위치 설정
            cout << "플레이어 움직임" << endl;
        }
    }
    if (packetType && PLAYER_HIT)
    {
        RecvPlayerHit();
        // 1. 자신의 체력 1 감소
        // 2-1. 체력이 모두 0일 경우, 관전 모드로 전환
        // 2-2. 체력이 남았을 경우, 스폰 위치로 이동
        cout << "플레이어 피격" << endl;
    }
    if (packetType && BULLET_DELETE)
    {
        // 1. 총알 삭제
        cout << "총알 삭제" << endl;
    }
}
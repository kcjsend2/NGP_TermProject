#include "main.h"

int main(int argc, char* argv[])
{
    // 0 : ���� ���� �̸�
    // 1 : ������ �ּ�
    // 2 : ��Ʈ ��ȣ

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
        // 1. ���� ���̵�(int) ����
        // 2. ���� ��ġ(XMFLOAT3) ����
        // 3. ���� ���̵�, ���� ��ġ ����, ���� ��ġ�� �̵�
        cout << "���ӽ���" << endl;
    }
    if (packetType && GAME_OVER)
    {
        RecvGameOver();
        // 1. ���� ����(bool) ����
        // 2. ���� ���θ� �޽��� �ڽ��� ���
        cout << "���ӿ���" << endl;
    }
    if ((packetType && PLAYER_MOVE) || (packetType && BULLET_UPDATE))
    {
        // 1. �÷��̾� ����(PlayerData) ����
        
        // PLAYER_MOVE �÷��װ� �����Ǿ� �ִٸ�
        if (packetType && PLAYER_MOVE)
        {
            // 2. �ش� id�� �÷��̾� ��ġ ����
            // 3. �ش� id�� �÷��̾� ȸ�� ����
            cout << "�÷��̾� ������" << endl;
        }

        // BULLET_UPDATE �÷��װ� �����Ǿ� �ִٸ�
        if (packetType && BULLET_UPDATE)
        {
            // 4. �ش� id�� �÷��̾��� �Ѿ� ��ġ ����
            cout << "�÷��̾� ������" << endl;
        }
    }
    if (packetType && PLAYER_HIT)
    {
        RecvPlayerHit();
        // 1. �ڽ��� ü�� 1 ����
        // 2-1. ü���� ��� 0�� ���, ���� ���� ��ȯ
        // 2-2. ü���� ������ ���, ���� ��ġ�� �̵�
        cout << "�÷��̾� �ǰ�" << endl;
    }
    if (packetType && BULLET_DELETE)
    {
        // 1. �Ѿ� ����
        cout << "�Ѿ� ����" << endl;
    }
}
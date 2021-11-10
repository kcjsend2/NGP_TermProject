#include"Prototype_Server.h"

// �׻� PlayerData �������� �����Ͱ� �ۼ��ŵȴ�.
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

// ȣ�� �Ծ� - __stdcall : �ش� �Լ��� __stdcall ȣ�� �Ծ��� ������.
// ������ �Լ��� �ü������ ȣ���ϰ�, �ü���� __stdcall �Ծ࿡ ���� ȣ���ϱ� ������, �ش� �Լ��� ȣ�� �Ծ��� __stdcall�� �����Ѵ�.


//Is Client Function? why is here?
DWORD WINAPI ProcessClientData(LPVOID arg)
{
    //�߰��� Ŭ���̾�Ʈ�� ������� �ʴ´ٴ� �����Ͽ� ������ ����ȴ�.
    ProcessClientData_Parameter* pParam = (ProcessClientData_Parameter*)arg;
    cout << pParam->sock << endl;

    PlayerData startMessage(GAME_START);
    int retval = send(pParam->sock, (char*)&startMessage, sizeof(PlayerData), 0);

    if (retval == SOCKET_ERROR)
    {
        err_quit("send()");
    }

    while (1)
    {
        EnterCriticalSection(&cs);
        recvn(pParam->sock, (char*)&aPlayerData[pParam->id], sizeof(PlayerData), 0);

        // �浹�� �÷��̾�� �Ѿ��� �� �÷��̾��� �����Ϳ��� �浹 ���θ� �ްԵȴ�. - PlayerData.bIntersected
        // ���� ��� 2�� �÷��̾ 1�� �÷��̾ �� �Ѿ˿� �¾��� ��, 2�� �÷��̾ �޴� 1�� �÷��̾��� PlayerData.bIntersected�� true�� �ȴ�.
        // ������ ���� ������� �����Ƿ� �浹�Ͽ����� �����Ͽ��� �� Ŭ���̾�Ʈ���� ��ġ ������ ��� ������ �ٲپ��ֱ⸸ �ϸ� �ȴ�.

        for (int i = 0; i < 3; ++i)
        {
            if (i != pParam->id)
            {
                send(pParam->sock, (char*)&aPlayerData[i], sizeof(PlayerData), 0);
            }
        }

        XMFLOAT3 Position = aPlayerData[pParam->id].m_position;
        cout << pParam->id << "�� �÷��̾� - ��ġ {" << Position.x << ", "<< Position.y << ", " << Position.z << "}" << endl;

        LeaveCriticalSection(&cs);

        // �ٸ� ������� ���ư��鼭 ����Ǿ���ϹǷ�
        Sleep(1);

        // ���� ������ Ȯ�εǾ��ٸ� �����带 �����Ѵ�.
        // ���� ������ �Ѹ��� �÷��̾ ������ ��� �÷��̾���� ����� 0�� �Ǿ��� ���̴�.
        if (0/*��������*/)
        {
            break;
        }
    }

    return 0;
}

int main(int argc, char* argv[])
{
    InitializeCriticalSection(&cs);

    WSADATA wsa;

    // 0 : ���� ���� �̸�, 1 : ��Ʈ ��ȣ
    int Port = atoi(argv[1]);

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // socket()
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");

    // connect()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(Port);
    if (bind(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR)
    {
        err_display("bind()");
    }

    // listen()
    if (listen(sock, SOMAXCONN) == SOCKET_ERROR)
    {
        err_quit("listen()");
    }

    int addrlen;

    SOCKADDR clientaddr;
    SOCKET client_sock[3];

    HANDLE hThread[3];

    // �ִ� �� ���� Ŭ���̾�Ʈ�� ������ ���̴�.
    // Ŭ���̾�Ʈ�� ������ �Ŀ�, Ŭ���̾�Ʈ���� ������ ���� �޽����� ���� �����͸� ��ٸ���.
    // ��� Ŭ���̾�Ʈ�� �����ϸ� �����带 ����� ������ �����Ѵ�.
    for(int i = 0; i < 3; ++i)
    {  
        // accept()
        addrlen = sizeof(SOCKADDR);
        client_sock[i] = accept(sock, (SOCKADDR*)&clientaddr, &addrlen);

        if (client_sock[i] == INVALID_SOCKET)
        {
            err_quit("accept()");
            break;
        }

        // ������ Ŭ���̾�Ʈ ���� ���
        char ipaddr[50];
        DWORD ipaddrlen = sizeof(ipaddr);
        WSAAddressToString((SOCKADDR*)&clientaddr, sizeof(clientaddr), NULL, ipaddr, &ipaddrlen);

        printf("\n[TCP ����] Ŭ���̾�Ʈ ����: %s\n", ipaddr);
    }

    for (int i = 0; i < 3; ++i)
    {
        ProcessClientData_Parameter* pParam = new ProcessClientData_Parameter;
        pParam->id = i;
        pParam->sock = client_sock[i];

        hThread[i] = CreateThread(NULL, 0, ProcessClientData, pParam, 0, NULL);
    }

    // ������ �ϳ��� ����Ǿ��ٴ� ���� ���� ������ �����ߴٴ� ���̴�. ����Ǿ��ִ� ��� Ŭ���̾�Ʈ�鿡�� ���� �޽����� ������.
    WaitForSingleObject(hThread[0], INFINITE);
    
    PlayerData gameOver(GAME_OVER);
    for (int i = 0; i < 3; ++i)
    {
        send(client_sock[i], (char*)&gameOver, sizeof(PlayerData), 0);
        closesocket(client_sock[i]);
    }

    closesocket(sock);

    WSACleanup();

    return 0;
}


//Receive
void RecvType(const SOCKET& sock)
{
    INT type;
    recvn(sock, (char*)type, sizeof(type), 0);

    if (type && GAME_OVER)
    {
        //1. Winner Player ID check
        //2. End Game
    }
    if (type && PLAYER_STATUS)
    {
        //1.recv Player_Status
        //2.Player_Status Update
    }
    if (type && PLAYER_DEAD)
    {
        //1. Player id check
        //2. (ID)Player is dead
    }
}
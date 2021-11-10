#include"Prototype_Server.h"

// 항상 PlayerData 형식으로 데이터가 송수신된다.
// 소켓 함수 오류 출력 후 종료
void err_quit(char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

// 소켓 함수 오류 출력
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

    // 남은 바이트가 0이면 수신 종료
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

    // 받기로 한 바이트 수 - 받지 않은 바이트 수 == 남은 바이트 수
    // 지금까지 받은 총 바이트 수 리턴
    // 정상적으로 받았다면 left == 0이므로 len을 리턴함, 받지 않았다면 left == len이므로 0을 리턴함. 받지 않은 상태가 에러인것은 아님 (클라이언트에서 send하지 않았을 수 있으므로)
    //printf("받기로 한 바이트 : %d\n", len);
    //printf("남은 바이트 : %d\n", left);
    return (len - left);
}

// 호출 규약 - __stdcall : 해당 함수는 __stdcall 호출 규약을 따른다.
// 스레드 함수는 운영체제에서 호출하고, 운영체제는 __stdcall 규약에 따라 호출하기 때문에, 해당 함수의 호출 규약을 __stdcall로 강제한다.


//Is Client Function? why is here?
DWORD WINAPI ProcessClientData(LPVOID arg)
{
    //중간에 클라이언트가 종료되지 않는다는 가정하에 게임이 진행된다.
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

        // 충돌한 플레이어는 총알을 쏜 플레이어의 데이터에서 충돌 여부를 받게된다. - PlayerData.bIntersected
        // 예를 들어 2번 플레이어가 1번 플레이어가 쏜 총알에 맞았을 때, 2번 플레이어가 받는 1번 플레이어의 PlayerData.bIntersected는 true가 된다.
        // 점수를 따로 기록하지 않으므로 충돌하였음을 감지하였을 때 클라이언트에서 위치 정보와 목숨 정보를 바꾸어주기만 하면 된다.

        for (int i = 0; i < 3; ++i)
        {
            if (i != pParam->id)
            {
                send(pParam->sock, (char*)&aPlayerData[i], sizeof(PlayerData), 0);
            }
        }

        XMFLOAT3 Position = aPlayerData[pParam->id].m_position;
        cout << pParam->id << "번 플레이어 - 위치 {" << Position.x << ", "<< Position.y << ", " << Position.z << "}" << endl;

        LeaveCriticalSection(&cs);

        // 다른 쓰레드와 돌아가면서 실행되어야하므로
        Sleep(1);

        // 종료 조건이 확인되었다면 쓰레드를 종료한다.
        // 종료 조건은 한명의 플레이어를 제외한 모든 플레이어들의 목숨이 0이 되었을 때이다.
        if (0/*종료조건*/)
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

    // 0 : 실행 파일 이름, 1 : 포트 번호
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

    // 최대 세 개의 클라이언트가 접근할 것이다.
    // 클라이언트가 접속한 후에, 클라이언트들은 서버의 시작 메시지를 담은 데이터를 기다린다.
    // 모든 클라이언트가 접속하면 쓰레드를 만들어 게임을 시작한다.
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

        // 접속한 클라이언트 정보 출력
        char ipaddr[50];
        DWORD ipaddrlen = sizeof(ipaddr);
        WSAAddressToString((SOCKADDR*)&clientaddr, sizeof(clientaddr), NULL, ipaddr, &ipaddrlen);

        printf("\n[TCP 서버] 클라이언트 접속: %s\n", ipaddr);
    }

    for (int i = 0; i < 3; ++i)
    {
        ProcessClientData_Parameter* pParam = new ProcessClientData_Parameter;
        pParam->id = i;
        pParam->sock = client_sock[i];

        hThread[i] = CreateThread(NULL, 0, ProcessClientData, pParam, 0, NULL);
    }

    // 쓰레드 하나가 종료되었다는 것은 종료 조건을 만족했다는 것이다. 연결되어있는 모든 클라이언트들에게 종료 메시지를 보낸다.
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
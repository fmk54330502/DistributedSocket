#include "socket.h"

int fmkSocket::config(int _port, string _ipaddr, int _nodeType)
{
    port = _port;
    ipaddr = _ipaddr;
    nodeType = _nodeType;
    return 0;
}

int fmkSocket::serverInit()
{
    //注意，所有变量已经定义在类中了，不需要再声明
    if (nodeType == CLIENT)
    {
        cout << "Client node cannot use server methods!" << endl;
        return (-1);
    }
    //1.加载WinSock库
    WSADATA wsaDATA;
    if (WSAStartup(MAKEWORD(2, 2), &wsaDATA) != 0)
    {
        //搜索响应的Socket库并绑定，执行成功返回0，版本2.2
        cout << "[Event]Failed to load Winsock, exit..." << endl;
        return (-1);
    }
    else
        cout << "[Event]Winsock loading success..." << endl;

    //2.创建用于监听的套接字socket()
    sockSrv = socket(AF_INET, SOCK_STREAM, 0);
    //AdressFamily的值类型ipv4|数据传输方式SOCK_STREAM和SOCK_DGRAM数据包
    if (SOCKET_ERROR == sockSrv)
    {
        cout << "Socket() error, state code:" << WSAGetLastError() << endl;
        return (-1);
    }

    //3.配置监听的地址和端口
    //SOCKADDR_IN addrSrv;
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons((u_short)port);                  //1024以上端口号
    addrSrv.sin_addr.S_un.S_addr = inet_addr(ipaddr.c_str()); //本机监听IP地址，要设置成本机某个网卡的具体地址才可以

    //4.服务器绑定监听socket:bind() listen()
    //retVal只是个临时的状态码，所以不需要放到类里
    int retVal = bind(sockSrv, (LPSOCKADDR)&addrSrv, sizeof(SOCKADDR_IN));
    if (retVal == SOCKET_ERROR)
    {
        cout << "[Event]Failed to bind, state code:" << WSAGetLastError() << endl;
        return (-2);
    }
    else
    {
        cout << "[Event]Succeeded to bind, state code:" << WSAGetLastError() << endl;
    }

    if (listen(sockSrv, 10) == SOCKET_ERROR)
    {
        cout << "[Event]Failed to listen, state code:" << WSAGetLastError() << endl;
        return (-3);
    }
    else
    {
        cout << "[Event]Succeeded to listen, state code:" << WSAGetLastError() << endl;
    }
    cout << "===================================" << endl;

    return 0;
}

int fmkSocket::serverRecv()
{
    if (nodeType == CLIENT)
    {
        cout << "Client node cannot use server methods!" << endl;
        return (-1);
    }
    //接收数据
    char *p = finalRecvBuf;
    int socketState;
    int flag = 0;
    //5.服务端accept等待客户端连接
    //SOCKADDR_IN addrClient;
    int len = sizeof(SOCKADDR);
    //SOCKET sockConn;
    sockConn = accept(sockSrv, (SOCKADDR *)&addrClient, &len);
    if (sockConn == SOCKET_ERROR)
    {
        cout << "[Event]Accept failed, state code:" << WSAGetLastError() << endl;
        //system("Pause");
        return (-1);
    }
    cout << "===================================" << endl;
    cout << "*****Accept client IP:" << inet_ntoa(addrClient.sin_addr) << endl;
    while (1)
    {
        //6.发送接收数据
        //发送数据

        //每次循环中都需要擦写，不然会被干扰
        //注意，对于静态数据，sizeof的到整个数组的实际占用字节数。对于指针，sizeof只返回指针的大小，哪怕指针已经指向一个数组
        memset(recvBuf, 0, MAX_BUFFER_SIZE);
        memset(finalRecvBuf, 0, FINALRECVBUFFER_SIZE);
        //接受数据并记录状态码
        socketState = recv(sockConn, recvBuf, 4 * sizeof(char), 0); //如果逻辑没错的话，这里地方只会接收到数据头，因此只有4个字节
        if (socketState <= 0)                                       //客户端消失或关闭了连接，0表示关闭了连接，小于0是异常关闭
        {
            //如果未接收到数据(客户端消失)，则关闭当前socket并重新开始等待客户端  连接
            cout << "[Event]Remote terminal closed." << endl;
            //重新开启socket进入等待。。。
            //closesocket(sockConn);
            sockConn = accept(sockSrv, (SOCKADDR *)&addrClient, &len);
            if (sockConn == SOCKET_ERROR)
            {
                cout << "[Event]Accept failed:" << WSAGetLastError() << endl;
            }
            cout << "===================================" << endl;
            cout << "*****Accept client IP:" << inet_ntoa(addrClient.sin_addr) << endl;
        }
        else
        {
            cout << "socketstate: " << socketState << endl;
            if (strcmp(recvBuf, sendbegin) == 0) //开始传输码
            {
                cout << "*****Data transfer begin..." << endl;
                flag = 1; //表示开始接受数据
            }
        }
        int i = 1;
        while (flag == 1)
        {
            memset(recvBuf, 0, MAX_BUFFER_SIZE);
            socketState = recv(sockConn, recvBuf, MAX_BUFFER_SIZE, 0);
            if (socketState <= 0)
                break;
            memcpy(printstr, recvBuf, MAX_BUFFER_SIZE);
            printstr[MAX_BUFFER_SIZE] = 0; //添加字符串的结束码
            if (sendbegin[0] != '6')
                system("Pause");
            if (recvBuf[0] == 'i' && recvBuf[1] == '\0' && recvBuf[2] == 'f') //传输结束码
            {
                cout << "*****Data receive finished." << endl;
                printf("Final received data: %s\n", finalRecvBuf);
                flag = 0;
                p = finalRecvBuf;
                char buf[] = "Feedback from server...";
                fmkSendData(buf, sockConn);
                break;
            }
            else if (recvBuf[0] == '9' && recvBuf[1] == '\0' && recvBuf[2] == '9') //关闭socket的码，一般用不到
            {
                //7.关闭socket
                closesocket(sockConn);
                WSACleanup();
                break;
            }
            else //如果不是特殊的码则正常传输
            {
                cout << i << "th received data:" << printstr << endl;
                strncpy(p, recvBuf, MAX_BUFFER_SIZE);
                p += MAX_BUFFER_SIZE;
                i++;
            }
        }
    }
    return 0;
}

int fmkSocket::clientRecv()
{
    int recvSize = 4 * sizeof(char);
    do
    {
        memset(recvBuf, 0, MAX_BUFFER_SIZE);
        int socketState = recv(sockClient, recvBuf, recvSize, 0);
        if (strcmp(recvBuf, "674") == 0)
            recvSize = MAX_BUFFER_SIZE;
        else if (recvBuf[0] == 'i' && recvBuf[1] == '\0' && recvBuf[0] == 'f')
        {
            recvSize = 4 * sizeof(char);
        }
        cout << socketState << " " << recvBuf << endl;

    } while (strlen(recvBuf) > 0);
    return 0;
}

int fmkSocket::clientInit()
{
    if (nodeType == SERVER)
    {
        cout << "Server node cannot use client methods!" << endl;
        return (-1);
    }
    //1.加载winsock库
    //WSADATA wsaDATA;
    if (WSAStartup(MAKEWORD(2, 2), &wsaDATA) != 0)
    {
        //搜索响应的Socket库并绑定，执行成功返回0，版本2.2
        cout << "[Event]Failed to load Winsock, exit..." << endl;
        return (-1);
    }
    else
    {
        cout << "[Event]Winsock loading success..." << endl;
    }

    //2.创建用于通讯的套接字
    sockClient = socket(AF_INET, SOCK_STREAM, 0);
    if (SOCKET_ERROR == sockClient)
    {
        cout << "Socket() error, state code: " << WSAGetLastError() << endl;
        return (-1);
    }

    //3.设置连接的服务端IP地址和端口
    //SOCKADDR_IN addrSrv;
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons((u_short)port);
    addrSrv.sin_addr.S_un.S_addr = inet_addr(ipaddr.c_str());

    //4.向服务器发送连接请求
    if (connect(sockClient, (struct sockaddr *)&addrSrv, sizeof(addrSrv)) == INVALID_SOCKET)
    {
        cout << "[Event]Connect failed, state code:" << WSAGetLastError() << endl;
        return (-1);
    }
}

int fmkSocket::clientSend(char *sendData)
{

    fmkSendData(sendData, sockClient);
    return 0;
}

int fmkSocket::clientDestroy()
{
    closesocket(sockClient);
    WSACleanup();
    return 0;
}

int fmkSocket::fmkSendData(char *sendData, SOCKET &s)
{

    char *formattedData;
    int lenFormattedData = ((strlen(sendData) + 1) / MAX_BUFFER_SIZE + 1) * MAX_BUFFER_SIZE;
    formattedData = (char *)malloc(sizeof(char) * lenFormattedData); //补齐传入字符串的长度
    memset(formattedData, 0, lenFormattedData);
    strcpy(formattedData, sendData);

    string prefix = nodeType == CLIENT ? "[clientSend]" : "[serverSend]";

    memset(recvBuf, 0, MAX_BUFFER_SIZE);
    cout << prefix << "send begin..." << endl;
    send(s, sendbegin, 4 * sizeof(char), 0);
    cout << prefix << "send data:" << formattedData << endl;
    send(s, formattedData, sizeof(char) * lenFormattedData, 0);
    cout << prefix << "send end..." << endl;
    send(s, sendend, MAX_BUFFER_SIZE, 0);

    FREE(formattedData);
    return 0;

    //以下注释代码在单词发送超过socket缓冲区长度时或许有用，不过缓冲区很大，大约有64K，所以一般情况下直接发送数据完全可以

    // int len = strlen(sendData) + 1; //要多传输一个'\0'，strlen是不包含'\0'的

    // //让实际传输的数据为bufferSize的整数倍，这样才能准确判断文件尾
    // char *formattedData;
    // int lenFormattedData = (strlen(sendData) / bufferSize + 1) * bufferSize;
    // formattedData = (char *)malloc(sizeof(char) * lenFormattedData); //补齐传入字符串的长度
    // memset(formattedData, 0, lenFormattedData);
    // strcpy(formattedData, sendData);
    // //if (strcmp(sendData, formattedData) == 0)
    // // cout << "*****Data correctness verification successful..." << endl;

    // if (len > bufferSize) //如果一次传输不完，则需要分块传送数据
    // {
    //     char *p = formattedData; //数据指针
    //     char *p0 = p;            //指针初始位置，用于判断已传输的长度
    //     char *tempstr;           //临时buffer
    //     tempstr = (char *)malloc(sizeof(char) * bufferSize);
    //     memset(tempstr, 0, bufferSize);
    //     int i = 1;
    //     cout << "*****send begin..." << endl;
    //     send(sock, sendbegin, 4 * sizeof(char), 0); //先传送开始传输校验码，提醒服务器该接受数据了
    //     //传4个足够了
    //     while (p - p0 < len) //关于取等，当相等时，p指向最后的'\0'，因此不必再专门传输一次了
    //     {
    //         strncpy(tempstr, p, bufferSize);
    //         send(sock, tempstr, sizeof(char) * bufferSize, 0);
    //         printf("*****%dth send data...\n", i);
    //         i++;
    //         p += bufferSize;
    //     }
    //     send(sock, sendend, MAX_BUFFER_SIZE, 0);
    //     cout << "*****send end..." << endl;

    //     FREE(p);
    //     FREE(p0);
    //     FREE(tempstr);
    // }
    // else
    // {
    // cout << "send begin..." << endl;
    // send(sock, sendbegin, 4 * sizeof(char), 0);
    // cout << "send data:" << formattedData << endl;
    // send(sock, formattedData, sizeof(char) * lenFormattedData, 0);
    // cout << "send end..." << endl;
    // send(sock, sendend, MAX_BUFFER_SIZE, 0);
    // }

    // FREE(formattedData);
    // return 0;
}
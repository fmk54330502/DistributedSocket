#ifndef __FMK_SOCKET_H__
#define __FMK_SOCKET_H__
#include <stdio.h>
#include <iostream>
#include <string>
#include <ctime>
#include <WinSock2.h>
#include <stdlib.h>

#include "json/json.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "pthreadVC2.lib")
#pragma comment(lib, "jsoncpp_x86_vs2019.lib")

using namespace std;
using namespace Json;

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define MAX_BUFFER_SIZE 8
#define FINALRECVBUFFER_SIZE (MAX_BUFFER_SIZE * 1024)

#define FREE(ptr)   \
    {               \
        ptr = NULL; \
        free(ptr);  \
    } //free不先把指针指向空的话老是会堆栈溢出，所以做一个宏来解决这个问题

#define SERVER 0
#define CLIENT 1

class fmkSocket
{
public:
    int nodeType; //节点类型，0是服务器，1是客户端
    string ipaddr;
    int port;

    WSADATA wsaDATA;
    SOCKET sockClient, sockSrv, sockConn;
    SOCKADDR_IN addrSrv, addrClient; //服务器IP地址和端口等信息

    // char recvBuf[MAX_BUFFER_SIZE];              //较长的数据接受buffer
    // char *finalRecvBuf; //最终数据段落。栈的内存很小，静态数组只有1M~2M的大小，所以不能分配太大的静态数组，只能使用动态数组
    // char printstr[MAX_BUFFER_SIZE + 1];

    // char sendbegin[MAX_BUFFER_SIZE];
    // char sendend[MAX_BUFFER_SIZE];

    //仔细想了想还是用动态数组吧，1024长度的静态数组在刚开始分配空间的时候就很卡了
    //[2021-09-26]卡不是因为动态数组，因为编译后第一次运行比较卡，第二次运行就好多了
    char *recvBuf;      //较长的数据接受buffer
    char *finalRecvBuf; //最终数据段落。栈的内存很小，静态数组只有1M~2M的大小，所以不能分配太大的静态数组，只能使用动态数组
    char *printstr;

    char *sendbegin;
    char *sendend;

    fmkSocket()
    {
        recvBuf = (char *)malloc(sizeof(char) * MAX_BUFFER_SIZE);
        finalRecvBuf = (char *)malloc(sizeof(char) * FINALRECVBUFFER_SIZE);
        printstr = (char *)malloc(sizeof(char) * (MAX_BUFFER_SIZE + 1));
        sendbegin = (char *)malloc(sizeof(char) * MAX_BUFFER_SIZE);
        sendend = (char *)malloc(sizeof(char) * MAX_BUFFER_SIZE);

        memset(sendbegin, 0, MAX_BUFFER_SIZE);
        sendbegin[0] = '6';
        sendbegin[1] = '7';
        sendbegin[2] = '4';
        memset(sendend, 0, MAX_BUFFER_SIZE);
        sendend[0] = 'i';
        sendend[2] = 'f'; //1已经被置为0不用写
        memset(recvBuf, 0, MAX_BUFFER_SIZE);
        memset(finalRecvBuf, 0, FINALRECVBUFFER_SIZE);
        memset(printstr, 0, MAX_BUFFER_SIZE + 1);
    }

    ~fmkSocket()
    {
        free(recvBuf);
        free(finalRecvBuf);
        free(printstr);
        free(sendbegin);
        free(sendend);
    }

    int config(int _port, string _ipaddr, int _nodeType);
    int reset()
    {
        WSACleanup();
        nodeType = CLIENT; //默认为客户端
        ipaddr = "";
        port = 0;
        sockClient = 0;
        sockSrv = 0;
        sockConn = 0;       
    }
    //=====Server=====
    int serverInit();
    int serverRecv();
    int serverSend(char *sendData);
    int setNodeAsServer()
    {
        nodeType = SERVER;
        return 0;
    }

    //=====Client=====
    int clientInit();
    int clientRecv();
    int clientSend(char *sendData);
    int clientDestroy();
    int setNodeAsClient()
    {
        nodeType = CLIENT;
        return 0;
    }

    int fmkSendData(char *sendData, SOCKET &s);
};

#endif

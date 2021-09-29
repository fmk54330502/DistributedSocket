#include "socket.h"

int main()
{
    fmkSocket mySock;
    cout << "\"mySock\" class generated..." << endl;
    string ipaddr = "192.168.1.61";
    int port = 5099;
    mySock.config(port, ipaddr, SERVER);
    cout << "Config Succeeded: server ip:" << ipaddr << ", port:" << port << endl;
    mySock.serverInit();
    mySock.serverRecv();
    system("Pause");
}
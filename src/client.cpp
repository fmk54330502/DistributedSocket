#include "socket.h"

int main()
{
    fmkSocket mySock;
    mySock.config(5099,"192.168.3.248",CLIENT);
    mySock.clientSend("LOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOLLOL");
    mySock.clientSend("aabbcc");
    //mySock.clientSend("LOL");
    //system("Pause");
}
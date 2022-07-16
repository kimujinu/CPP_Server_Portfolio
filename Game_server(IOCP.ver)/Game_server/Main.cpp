#include "pch.h"
#include "MainIocp.h"

int main()
{
    MainIocp iocp_server;
    if (iocp_server.Initialize())
    {
        iocp_server.StartServer();
    }
    return 0;
}


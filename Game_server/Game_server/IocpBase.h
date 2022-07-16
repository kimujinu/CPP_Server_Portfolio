#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS // ��Ƽ����Ʈ ���� ���� define

// winsock2 ����� ���� �ڸ�Ʈ �߰�
#pragma comment (lib,"ws2_32.lib")

#include <WinSock2.h>
#include <map>
#include <vector>
#include <iostream>
#include "CommonClass.h"

using namespace std;

#define MAX_BUFFER 4096
#define SERVER_PORT 3002
#define MAX_CLIENTS 100

// IOCP ���� ����ü
struct stSOCKETINFO 
{
	WSAOVERLAPPED overlapped;
	WSABUF dataBuf;
	SOCKET socket;
	char messageBuffer[MAX_BUFFER];
	int recvBytes;
	int sendBytes;
};

// ��Ŷ ó�� �Լ� ������
struct FuncProcess
{
	void(*funcProcessPakcet)(stringstream& RecvStream, stSOCKETINFO* pSocket);
	FuncProcess()
	{
		funcProcessPakcet = nullptr;
	}
};

class IocpBase
{
public:
	IocpBase();
	virtual ~IocpBase();

	// ���� ��� �� ���� ���� ����
	bool Initialize();
	// ���� ����
	virtual void StartServer();
	// �۾� ������ ����
	virtual bool CreateWorkerThread();
	// �۾� ������
	virtual void WorkerThread();
	// Ŭ���̾�Ʈ���� �۽�
	virtual void Send(stSOCKETINFO* pSocket);
	// Ŭ���̾�Ʈ���� ���� ���
	virtual void Recv(stSOCKETINFO* pSocket);

protected:
	stSOCKETINFO* SocketInfo; // ���� ����
	SOCKET ListenSocket; // ���� ���� ����
	HANDLE hIOCP; // IOCP ��ü �ڵ�
	bool bAccept; // ��û ���� �÷���
	bool bWorkerThread; // �۾� ������ ���� �÷���
	HANDLE* hWorkerHandle; //  �۾� ������ �ڵ�
	int nThreadCnt;
};
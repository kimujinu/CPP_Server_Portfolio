#pragma once

// ��Ƽ����Ʈ ���� ���� define
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// winsock2 ����� ���� �ڸ�Ʈ �߰�
#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h>
#include <map>
#include <vector>
#include <iostream>
#include "CommonClass.h"
#include "DBConnector.h"
#include "IocpBase.h"

using namespace std;

#define DB_ADDRESS "localhost"
#define DB_PORT 3306
#define DB_ID "root"
#define DB_PW "1234qwer"
#define DB_SCHEMA "sys"

class MainIocp : public IocpBase
{
public:
	MainIocp();
	virtual ~MainIocp();

	virtual void StartServer() override;
	// �۾� ������ ����
	virtual bool CreateWorkerThread() override;
	// �۾� ������
	virtual void WorkerThread() override;
	// Ŭ���̾�Ʈ���� �۽�
	static void Send(stSOCKETINFO* pSocket);

private:
	static cClientInfo ClientInfo; // ������ Ŭ���̾�Ʈ ���� ����
	static map<int, SOCKET> SessionSocket; // ���Ǻ� ���� ����
	static DBConnector Conn; // DB Ŀ����
	static CRITICAL_SECTION csPlayers; // ClientInfo �Ӱ迵��

	FuncProcess fnProcess[100]; // ��Ŷ ó�� ����ü
	
	// ȸ������
	static void SignUp(stringstream& RecvStream, stSOCKETINFO* pSocket);
	// DB�� �α���
	static void Login(stringstream& RecvStream, stSOCKETINFO* pSocket);
	// �α׾ƿ�
	static void Logout(stringstream& RecvStream, stSOCKETINFO* pSocket);
	// ä�� ���� �� Ŭ���̾�Ʈ�鿡�� �۽�
	static void BroadcastChat(stringstream& RecvStream, stSOCKETINFO* pSocket);
	
	// ��ε� ĳ��Ʈ �Լ�
	static void Broadcast(stringstream& SendStream);
	// �ٸ� Ŭ���̾�Ʈ�鿡�� �� Ŭ���̾�Ʈ ���� ���� �۽�
	static void BroadcastNewClient(cClient &player);
	// Ŭ���̾�Ʈ ������ ���ۿ� ���
	static void WriteClientInfoToSocket(stSOCKETINFO* pSocket);

};
#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS // 멀티바이트 집합 사용시 define

// winsock2 사용을 위해 코멘트 추가
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

// IOCP 소켓 구조체
struct stSOCKETINFO 
{
	WSAOVERLAPPED overlapped;
	WSABUF dataBuf;
	SOCKET socket;
	char messageBuffer[MAX_BUFFER];
	int recvBytes;
	int sendBytes;
};

// 패킷 처리 함수 포인터
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

	// 소켓 등록 및 서버 정보 설정
	bool Initialize();
	// 서버 시작
	virtual void StartServer();
	// 작업 스레드 생성
	virtual bool CreateWorkerThread();
	// 작업 스레드
	virtual void WorkerThread();
	// 클라이언트에게 송신
	virtual void Send(stSOCKETINFO* pSocket);
	// 클라이언트에게 수신 대기
	virtual void Recv(stSOCKETINFO* pSocket);

protected:
	stSOCKETINFO* SocketInfo; // 소켓 정보
	SOCKET ListenSocket; // 서버 리슨 소켓
	HANDLE hIOCP; // IOCP 객체 핸들
	bool bAccept; // 요청 동작 플래그
	bool bWorkerThread; // 작업 스레드 동작 플래그
	HANDLE* hWorkerHandle; //  작업 스레드 핸들
	int nThreadCnt;
};
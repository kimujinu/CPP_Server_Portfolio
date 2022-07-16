#pragma once

// 멀티바이트 집합 사용시 define
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// winsock2 사용을 위해 코멘트 추가
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
	// 작업 스레드 생성
	virtual bool CreateWorkerThread() override;
	// 작업 스레드
	virtual void WorkerThread() override;
	// 클라이언트에게 송신
	static void Send(stSOCKETINFO* pSocket);

private:
	static cClientInfo ClientInfo; // 접속한 클라이언트 정보 저장
	static map<int, SOCKET> SessionSocket; // 세션별 소켓 저장
	static DBConnector Conn; // DB 커넥터
	static CRITICAL_SECTION csPlayers; // ClientInfo 임계영역

	FuncProcess fnProcess[100]; // 패킷 처리 구조체
	
	// 회원가입
	static void SignUp(stringstream& RecvStream, stSOCKETINFO* pSocket);
	// DB에 로그인
	static void Login(stringstream& RecvStream, stSOCKETINFO* pSocket);
	// 로그아웃
	static void Logout(stringstream& RecvStream, stSOCKETINFO* pSocket);
	// 채팅 수신 후 클라이언트들에게 송신
	static void BroadcastChat(stringstream& RecvStream, stSOCKETINFO* pSocket);
	
	// 브로드 캐스트 함수
	static void Broadcast(stringstream& SendStream);
	// 다른 클라이언트들에게 새 클라이언트 입장 정보 송신
	static void BroadcastNewClient(cClient &player);
	// 클라이언트 정보를 버퍼에 기록
	static void WriteClientInfoToSocket(stSOCKETINFO* pSocket);

};
#include "pch.h"
#include "MainIocp.h"
#include <process.h>
#include <sstream>
#include <algorithm>
#include <string>

map<int, SOCKET> MainIocp::SessionSocket;
cClientInfo MainIocp::ClientInfo;
DBConnector MainIocp::Conn;
CRITICAL_SECTION MainIocp::csPlayers;

unsigned int WINAPI CallWorkerThread(LPVOID p)
{
	MainIocp* pOverlappedEvent = (MainIocp*)p;
	pOverlappedEvent->WorkerThread();
	return 0;
}

MainIocp::MainIocp()
{
	InitializeCriticalSection(&csPlayers);

	// DB 접속
	if (Conn.Connect(DB_ADDRESS, DB_ID, DB_PW, DB_SCHEMA, DB_PORT))
	{
		printf_s("[INFO] DB 접속 성공\n");
	}
	else {
		printf_s("[ERROR] DB 접속 실패\n");
	}

	fnProcess[EPacketType::SIGNUP].funcProcessPakcet = SignUp;
	fnProcess[EPacketType::LOGIN].funcProcessPakcet = Login;
	fnProcess[EPacketType::LOGOUT].funcProcessPakcet = Logout;
	fnProcess[EPacketType::CHAT].funcProcessPakcet = BroadcastChat;
}

MainIocp::~MainIocp()
{
	WSACleanup(); // winsock 사용 종료.

	// 다 사용한 객체 삭제
	if (SocketInfo)
	{
		delete[] SocketInfo;
		SocketInfo = NULL;
	}

	if (hWorkerHandle)
	{
		delete[] hWorkerHandle;
		hWorkerHandle = NULL;
	}

	Conn.Close(); // DB 연결 종료
}

void MainIocp::StartServer()
{
	IocpBase::StartServer();
}

bool MainIocp::CreateWorkerThread()
{
	unsigned int threadId;
	// 시스템 정보 가져오기
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	printf_s("[INFO] CPU 갯수 : %d\n", sysInfo.dwNumberOfProcessors);
	// 적절한 작업 스레드의 갯수는 (CPU * 2) + 1
	nThreadCnt = sysInfo.dwNumberOfProcessors * 2;

	// thread handler 선언
	hWorkerHandle = new HANDLE[nThreadCnt];
	// thread 생성
	for (int i = 0; i < nThreadCnt; i++)
	{
		hWorkerHandle[i] = (HANDLE*)_beginthreadex(
			NULL, 0, &CallWorkerThread, this, CREATE_SUSPENDED, &threadId
		);
		if (hWorkerHandle[i] == NULL)
		{
			printf_s("[ERROR] Worker Thread 생성 실패\n");
			return false;
		}
		ResumeThread(hWorkerHandle[i]);
	}
	printf_s("[INFO] Worker Thread 시작...\n");
	return true;
}

void MainIocp::Send(stSOCKETINFO* pSocket)
{
	int nResult;
	DWORD sendBytes;
	DWORD dwFlags = 0;

	nResult = WSASend(
		pSocket->socket,
		&(pSocket->dataBuf),
		1,
		&sendBytes,
		dwFlags,
		NULL,
		NULL
	);

	if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
	{
		printf_s("[ERROR] WSASend 실패 :", WSAGetLastError());
	}
}

void MainIocp::WorkerThread()
{
	// 함수 호출 성공 여부
	BOOL bResult;
	int nResult;
	// overlapped I/O 작업에서 전송된 데이터 크기
	DWORD recvBytes;
	DWORD sendBytes;
	// Completion Key를 받을 포인터 변수
	stSOCKETINFO* pCompletionKey;
	// I/O 작업을 위해 요청한 overlapped 구조체를 받을 포인터
	stSOCKETINFO* pSocketInfo;
	DWORD dwFlags = 0;

	while (bWorkerThread)
	{
		/*
		   이 함수로 인해 쓰레드들은 WaitingThread Queue에 대기상태로 들어가게 된다.
		   완료된 Overlapped I/O 작업이 발생하면 IOCP Queue에서 완료된 작업을 가져와 뒷처리를 한다.
		*/

		bResult = GetQueuedCompletionStatus(
			hIOCP,
			&recvBytes, // 실제로 전송된 바이트
			(PULONG_PTR)&pCompletionKey, // completion key
			(LPOVERLAPPED*)&pSocketInfo, // overlapped I/O 객체
			INFINITE // 대기할 시간
		);

		if (!bResult && recvBytes == 0)
		{
			printf_s("[INFO] socket(%d) 접속 끊김\n", pSocketInfo->socket);
			closesocket(pSocketInfo->socket);
			free(pSocketInfo);
			continue;
		}

		pSocketInfo->dataBuf.len = recvBytes;

		if (recvBytes == 0)
		{
			closesocket(pSocketInfo->socket);
			free(pSocketInfo);
			continue;
		}

		try {
			int PacketType; // 패킷 종류
			stringstream RecvStream; // 클라이언트 정보 역직렬화

			RecvStream << pSocketInfo->dataBuf.buf;
			RecvStream >> PacketType;

			// 패킷 처리
			if (fnProcess[PacketType].funcProcessPakcet != nullptr)
			{
				fnProcess[PacketType].funcProcessPakcet(RecvStream, pSocketInfo);
			}
			else {
				printf_s("[ERROR] 정의 되지 않은 패킷 : %d\n", PacketType);
			}
		}
		catch (const std::exception& e)
		{
			printf_s("[ERROR] 알 수 없는 예외 발생 : %s\n", e.what());
		}

		Recv(pSocketInfo);
	}
}

void MainIocp::SignUp(stringstream& RecvStream, stSOCKETINFO* pSocket)
{
	string Id;
	string Pw;

	RecvStream >> Id;
	RecvStream >> Pw;

	printf_s("[INFO] 회원가입 시도 {%s}/{%s}\n", Id, Pw);

	stringstream SendStream;
	SendStream << EPacketType::SIGNUP << endl;
	SendStream << Conn.SignUpAccount(Id, Pw) << endl;

	CopyMemory(pSocket->messageBuffer, (CHAR*)SendStream.str().c_str(), SendStream.str().length());
	pSocket->dataBuf.buf = pSocket->messageBuffer;
	pSocket->dataBuf.len = SendStream.str().length();

	Send(pSocket);
}

void MainIocp::Login(stringstream& RecvStream, stSOCKETINFO* pSocket)
{
	string Id;
	string Pw;

	RecvStream >> Id;
	RecvStream >> Pw;

	printf_s("[INFO] 로그인 시도 {%s}/{%s}\n", Id, Pw);

	stringstream SendStream;
	SendStream << EPacketType::LOGIN << endl;
	SendStream << Conn.SearchAccount(Id, Pw) << endl;

	CopyMemory(pSocket->messageBuffer, (CHAR*)SendStream.str().c_str(), SendStream.str().length());
	pSocket->dataBuf.buf = pSocket->messageBuffer;
	pSocket->dataBuf.len = SendStream.str().length();

	Send(pSocket);
}

void MainIocp::Logout(stringstream& RecvStream, stSOCKETINFO* pSocket)
{
	int SessionId;
	RecvStream >> SessionId;
	printf_s("[INFO] (%d)로그아웃 요청 수신\n", SessionId);
	EnterCriticalSection(&csPlayers);
	ClientInfo.players[SessionId].IsAlive = false;
	LeaveCriticalSection(&csPlayers);
	SessionSocket.erase(SessionId);
	printf_s("[INFO] 클라이언트 수 : %d\n", SessionSocket.size());
	WriteClientInfoToSocket(pSocket);
}

void MainIocp::BroadcastChat(stringstream& RecvStream, stSOCKETINFO* pSocket)
{
	stSOCKETINFO* client = new stSOCKETINFO;

	int SessionId;
	string temp;
	string chat;

	RecvStream >> SessionId;
	getline(RecvStream, temp);
	chat += to_string(SessionId) + "_:_";
	while (RecvStream >> temp)
	{
		chat += temp + "_";
	}
	chat += '\0';

	printf_s("[CHAT] %s\n", chat);

	stringstream SendStream;
	SendStream << EPacketType::CHAT << endl;
	SendStream << chat;

	Broadcast(SendStream);
}

void MainIocp::BroadcastNewClient(cClient& player)
{
	stringstream SendStream;
	SendStream << EPacketType::ENTER_NEW_PLAYER << endl;
	SendStream << player << endl;

	Broadcast(SendStream);
}

void MainIocp::Broadcast(stringstream& SendStream)
{
	stSOCKETINFO* client = new stSOCKETINFO;
	for (const auto& kvp : SessionSocket)
	{
		client->socket = kvp.second;
		CopyMemory(client->messageBuffer, (CHAR*)SendStream.str().c_str(), SendStream.str().length());
		client->dataBuf.buf = client->messageBuffer;
		client->dataBuf.len = SendStream.str().length();

		Send(client);
	}
}

void MainIocp::WriteClientInfoToSocket(stSOCKETINFO* pSocket)
{
	stringstream SendStream;

	// 직렬화
	SendStream << EPacketType::RECV_PLAYER << endl;
	SendStream << ClientInfo << endl;

	// data,buf에다 직접 데이터를 쓰면 쓰레기값이 전달될 수 있다
	CopyMemory(pSocket->messageBuffer, (CHAR*)SendStream.str().c_str(), SendStream.str().length());
	pSocket->dataBuf.buf = pSocket->messageBuffer;
	pSocket->dataBuf.len = SendStream.str().length();
}
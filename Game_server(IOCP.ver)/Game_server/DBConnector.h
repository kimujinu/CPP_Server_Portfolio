#pragma once

#include "mysql.h"
#include <string>

#pragma comment(lib,"libmysql.lib")

using namespace std;

class DBConnector
{
public:
	DBConnector();
	~DBConnector();

	// DB에 연결
	bool Connect(
		const string& Server,
		const string& User,
		const string& Password,
		const string& Database,
		const int& Port
	);

	void Close(); // DB연결 종료

	// 유저 계정 찾기
	bool SearchAccount(const string& Id, const string& Password);
	// 유저 계정 등록
	bool SignUpAccount(const string& Id, const string& Password);

private:
	MYSQL* Conn; // 커넥터
	MYSQL_RES* Res; // 결과값
	MYSQL_ROW Row; // 결과 row
};
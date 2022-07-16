#include "pch.h"
#include "DBConnector.h"

DBConnector::DBConnector()
{
}

DBConnector::~DBConnector()
{
}

bool DBConnector::Connect(const string& Server, const string& User, const string& Password, const string& Database, const int& Port) 
{
	Conn = mysql_init(NULL);
	if (!mysql_real_connect(Conn, Server.c_str(), User.c_str(), Password.c_str(), Database.c_str(), Port, NULL, 0)) {
		printf_s("[DB] DB ���� ����");
		return false;
	}
	return true;
}

void DBConnector::Close()
{
	mysql_close(Conn);
}

bool DBConnector::SearchAccount(const string& Id, const string& Password)
{
	bool bResult = false;
	string sql = "SELECT * FROM sys.chatdb WHERE ID = '";
	sql += Id + "' and PW = '" + Password + "'";

	if (mysql_query(Conn, sql.c_str()))
	{
		printf_s("[DB] �˻� ����\n");
		return false;
	}

	Res = mysql_use_result(Conn);
	Row = mysql_fetch_row(Res);
	if (Row != NULL) {
		bResult = true;
	}
	else {
		printf_s("[ERROR] �ش� ���̵� ����\n");
		bResult = false;
	}
	mysql_free_result(Res);

	return bResult;
}

bool DBConnector::SignUpAccount(const string& Id, const string& Password)
{
	bool bResult = false;
	string sql = "INSERT INTO sys.chatdb (ID,PW) VALUES";
	sql += " ('" + Id + "', '" + Password + "')";

	if (mysql_query(Conn, sql.c_str()))
	{
		printf_s("[DB] �ߺ��� ����\n");
		return false;
	}
	bResult = true;

	return bResult;
}

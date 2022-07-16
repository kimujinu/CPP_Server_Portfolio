#pragma once

#ifdef COMMONCLASS_EXPORTS
#define COMMONCLASS_API __declspec(dllexport)
#else
#define COMMONCLASS_API __declspec(dllimport)
#endif // COMMONCLASS_EXPORTS

#include <iostream>
#include <map>

using namespace std;

#define MAX_CLIENTS 100

enum COMMONCLASS_API EPacketType // 패킷 타입 설정
{
	LOGIN,
	CHAT,
	SIGNUP,
	LOGOUT,
	ENTER_NEW_PLAYER,
	SEND_PLAYER,
	RECV_PLAYER
};

class COMMONCLASS_API cClient
{
public:
	cClient();
	~cClient();

	bool IsAlive;

	int SessionId;

	friend ostream& operator<<(ostream& stream, cClient& info) 
	{
		stream << info.SessionId << endl;
		stream << info.IsAlive << endl;

		return stream;
	}

	friend istream& operator>>(istream& stream, cClient& info)
	{
		stream >> info.SessionId;
		stream >> info.IsAlive;

		return stream;
	}
};

class COMMONCLASS_API cClientInfo
{
public:
	cClientInfo();
	~cClientInfo();

	map<int, cClient> players;

	friend ostream& operator<<(ostream& stream, cClientInfo& info)
	{
		stream << info.players.size() << endl;
		for (auto& kvp : info.players)
		{
			stream << kvp.first << endl;
			stream << kvp.second << endl;
		}
		return stream;
	}

	friend istream& operator>>(istream& stream, cClientInfo& info)
	{
		int nPlayers = 0;
		int SessionId = 0;
		cClient Player;
		info.players.clear();

		stream >> nPlayers;
		for (int i = 0; i < nPlayers; i++)
		{
			stream >> SessionId;
			stream >> Player;
			info.players[SessionId] = Player;
		}

		return stream;
	}
};


class COMMONCLASS_API CommonClass
{
public:
	CommonClass();
	~CommonClass();
};
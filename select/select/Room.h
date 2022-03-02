

#pragma once
#include "pch.h"


class Room
{
public:
	Room() = default;
	Room( UxInt32 num, UxString name, UxInt32 max );
	~Room();

public:
	UxInt32					GetRoomNum();
	UxString				GetCurrentNum();
	UxString				GetName();

	UxVoid					UserJoin( UxInt32 id );
	UxVoid					UserLeave( UxInt32 id );
	std::vector<UxInt32>	GetUsers();

	UxBool					IsRoomMax();

	UxString				GetOpenTime();

protected:
	UxInt32					m_roomNum;
	UxString				m_roomName;

	std::vector<UxInt32>	m_users;
	UxInt32					m_maximun;

	time_t					m_openTime;
};

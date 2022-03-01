

#pragma once
#include "pch.h"


class Room
{
public:
	Room() = default;
	Room( UxInt32 num, UxString name, UxInt32 max );
	~Room();

public:
	UxVoid					UserJoin( UxInt32 id );
	UxVoid					UserLeave( UxInt32 id );

	UxBool					IsRoomMax();

protected:
	UxInt32					m_roomNum;
	UxString				m_roomName;

	std::vector<UxInt32>	m_users;
	UxInt32					m_maximun;
};

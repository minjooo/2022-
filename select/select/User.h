

#pragma once
#include "typedef.h"


class User
{
public:
	User() = default;
	User( UxInt32 id );
	~User();

public:
	UxVoid		SetAddr( const IN_ADDR& addr, const USHORT& port );
	UxString	GetAddr();

	UxVoid		SetSocket( const SOCKET& sock );
	SOCKET		GetSocket();

	UxInt32		GetId() const;
	UxVoid		SetId( UxInt32 id );

	UxVoid		SetName( UxString name );
	UxString	GetName();

	UxBool		IsAccess();
	UxVoid		SetAccess();
	UxVoid		SetAccessCancle();

	UxBool		AddCommand( UxInt8* add );
	UxVoid		AddBackspace();
	UxVoid		EraseFirstCommand();
	UxVoid		ClearCommand();
	UxString	GetCommand();

	UxBool		IsInRoom();
	UxVoid		LeaveRoom();
	UxVoid		SetRoomNum( UxInt32 roomNum );
	UxInt32		GetRoomNum();
	UxString	GetRoomJoinTime();

public:
	UxBool		operator==( User& user );

protected:
	UxString	m_addr;

	UxInt32		m_id;
	UxString	m_name;
	UxBool		m_isAccess;

	SOCKET		m_socket;
	UxString	m_command;

	UxBool		m_isInRoom;
	UxInt32		m_roomNum;
	time_t		m_roomJoinTime;
};

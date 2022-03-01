

#include "User.h"


User::User( UxInt32 id, SOCKET socket )
{
	m_id = id;
	m_name = "";
	m_isAccess = false;
	m_socket = socket;
	m_command = "";
	m_isInRoom = false;
	m_roomNum = -1;
}

User::~User()
{

}

UxVoid User::Initialize()
{

}

UxInt32 User::GetId() const
{
	return m_id;
}

UxVoid User::SetName( UxString name )
{
	m_name = name;
}

UxString User::GetName()
{
	return m_name;
}

UxBool User::IsAccess()
{
	return m_isAccess;
}

UxVoid User::SetAccess()
{
	m_isAccess = true;
}

UxVoid User::AddCommand( UxString add )
{
	m_command += add;
}

UxVoid User::AddCommand( UxInt8* add )
{
	UxString tmp( add );
	m_command += tmp;
}

UxVoid User::ClearCommand()
{
	m_command.clear();
}

UxString User::GetCommand()
{
	return m_command;
}

UxBool User::IsInRoom()
{
	return m_isInRoom;
}

UxVoid User::SetRoomNum( UxInt32 roomNum )
{
	m_roomNum = roomNum;
}

UxInt32 User::GetRoomNum()
{
	return m_roomNum;
}

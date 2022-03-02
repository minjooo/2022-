

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

UxVoid User::SetAddr( const IN_ADDR& addr, const USHORT& port )
{
	UxInt8 c[INET_ADDRSTRLEN];
	inet_ntop( AF_INET, &addr, c, INET_ADDRSTRLEN );
	UxString str( c );
	str += " : ";
	str += std::to_string( port );
	m_addr = str;
}

UxString User::GetAddr()
{
	return m_addr;
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

UxVoid User::EraseFirstCommand()
{
	m_command.erase( m_command.begin(), std::find( m_command.begin(), m_command.end(), ' ' ) + 1 );
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

UxVoid User::LeaveRoom()
{
	m_roomNum = -1;
	m_isInRoom = false;
}

UxVoid User::SetRoomNum( UxInt32 roomNum )
{
	m_roomNum = roomNum;
	m_isInRoom = true;
	m_roomJoinTime = time( NULL );
}

UxInt32 User::GetRoomNum()
{
	return m_roomNum;
}

UxString User::GetRoomJoinTime()
{
	struct tm* t = localtime( &m_roomJoinTime );
	UxString str = std::to_string( t->tm_hour ) + ":" + std::to_string( t->tm_min ) + "." + std::to_string( t->tm_sec );
	return str;
}

UxBool User::operator==( User& user )
{
	return user.GetName() == m_name;
}

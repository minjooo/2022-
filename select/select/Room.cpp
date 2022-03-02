

#include "Room.h"

Room::Room( UxInt32 num, UxString name, UxInt32 max )
{
	m_roomNum = num;
	m_roomName = name;
	m_maximun = max;
	m_users.reserve( max );
}

Room::~Room()
{

}

UxInt32 Room::GetRoomNum()
{
	return m_roomNum;
}

UxString Room::GetCurrentNum()
{
	UxString str = "(" + std::to_string( m_users.size() ) + "/" + std::to_string( m_maximun ) + ")";
	return str;
}

UxString Room::GetName()
{
	return m_roomName;
}

UxVoid Room::UserJoin( UxInt32 id )
{
	m_users.emplace_back( id );
}

UxVoid Room::UserLeave( UxInt32 id )
{
	m_users.erase( std::remove( m_users.begin(), m_users.end(), id ), m_users.end() );
}

std::vector<UxInt32> Room::GetUsers()
{
	return m_users;
}

UxBool Room::IsRoomMax()
{
	return m_users.size() == m_maximun;
}

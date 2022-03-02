

#include "Room.h"

Room::Room( UxInt32 num, UxString name, UxInt32 max )
{
	m_openTime = time( NULL );

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

UxBool Room::IsRoomEmpty()
{
	return m_users.size() == 0;
}

UxString Room::GetOpenTime()
{
	struct tm* t = localtime( &m_openTime );
	UxString str = std::to_string( t->tm_hour ) + ":" + std::to_string( t->tm_min ) + "." + std::to_string( t->tm_sec );
	return str;
}

UxVoid Room::ChangeUserId( UxInt32 id, UxInt32 to )
{
	for ( UxInt32 i = 0; i < m_users.size(); ++i )
	{
		if ( m_users[i] == id )
		{
			m_users[i] = to;
			return;
		}
	}
}

UxBool Room::operator==( Room& room )
{
	return room.GetRoomNum() == m_roomNum;
}

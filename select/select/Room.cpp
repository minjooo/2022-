

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

UxVoid Room::UserJoin( UxInt32 id )
{
	m_users.emplace_back( id );
}

UxVoid Room::UserLeave( UxInt32 id )
{
	m_users.erase( std::remove( m_users.begin(), m_users.end(), id ), m_users.end() );
}

UxBool Room::IsRoomMax()
{
	return m_users.size() == m_maximun;
}

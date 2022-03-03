

#include "typedef.h"
#include "const.h"
#include "User.h"
#include "Room.h"


WSAEVENT g_events[WSA_MAXIMUM_WAIT_EVENTS];
UxInt32 g_counter { 1 };

std::map<UxInt32, User>		g_users;
std::map<UxInt32, Room>		g_rooms;
UxInt32						g_roomCounter { 0 };


UxString GetNextCommand( UxInt32 id )
{
	g_users[id].EraseFirstCommand();
	return g_users[id].GetCommand().substr( 0, g_users[id].GetCommand().find( " " ) );
}

UxBool FindUserWithName( const UxString& name, User* user )
{
	for ( auto&& u : g_users )
	{
		if ( u.second.GetName() == name )
		{
			*user = u.second;
			return true;
		}
	}
	return false;
}

UxBool FindUserWithName( const UxString& name )
{
	for ( auto&& u : g_users )
	{
		if ( u.second.GetName() == name )
		{
			return true;
		}
	}
	return false;
}

UxVoid SendPacket( UxInt32 id, const UxInt8* buff )
{
	send( g_users[id].GetSocket(), buff, strlen(buff), 0 );
}

UxVoid SendChat( UxInt32 id, UxInt32 to )
{
	g_users[id].EraseFirstCommand();
	UxString str = "[" + g_users[id].GetName() + "]	" + g_users[id].GetCommand() +"\r\n";
	const UxInt8* c = str.c_str();
	SendPacket( to, c );
}

UxVoid SendRoomChat( UxInt32 id )
{
	UxString str = g_users[id].GetName() + "> " + g_users[id].GetCommand() + "\r\n";
	const UxInt8* c = str.c_str();
	for ( auto&& userId : g_rooms[g_users[id].GetRoomNum()].GetUsers() )
	{
		SendPacket( userId, c );
	}
}

UxVoid SendInvalid( UxInt32 id, EInvalidEvent e )
{
	UxString str = "";

	switch ( e )
	{
	case EInvalidEvent::AlreadyExistName:
		str += "이미 존재하는 아이디입니다.\r\n";
		break;
	case EInvalidEvent::NotExistUser:
		str += "존재하지 않는 아이디입니다.\r\n";
		break;
	case EInvalidEvent::NotExistRoom:
		str += "존재하지 않는 방입니다.\r\n";
		break;
	case EInvalidEvent::RoomFull:
		str += "방이 가득찼습니다.\r\n";
		break;
	default:
		break;
	}

	const UxInt8* c = str.c_str();
	SendPacket( id, c );
}

UxVoid SendBasicMention( UxInt32 id )
{
	const UxInt8* c = Message::basic.c_str();
	SendPacket( id, c );
}

UxVoid SendLoginMention( UxInt32 id )
{
	const UxInt8* c = Message::login.c_str();
	SendPacket( id, c );
}

UxVoid SendWelcome( UxInt32 id )
{
	const UxInt8* c = Message::welcome.c_str();
	SendPacket( id, c );
}

UxVoid SendBye( UxInt32 id )
{
	const UxInt8* c = Message::bye.c_str();
	SendPacket( id, c );
}

UxVoid SendInstruction( UxInt32 id )
{
	if ( g_users[id].IsInRoom() )
	{
		const UxInt8* c = Message::helpRoom.c_str();
		SendPacket( id, c );
	}
	else
	{
		const UxInt8* c = Message::helpLobby.c_str();
		SendPacket( id, c );
	}
}

UxVoid SendAllUserList( UxInt32 id )
{
	UxString str = "------------------------- 사람들 목록 -------------------------\r\n";
	for ( auto&& user : g_users )
	{
		str += ("[" + user.second.GetName() + "]		" + user.second.GetAddr() + "\r\n");
	}
	str += "---------------------------------------------------------------\r\n";

	const UxInt8* c = str.c_str();
	SendPacket( id, c );
}

UxVoid SendRoomList( UxInt32 id )
{
	UxString str = "------------------------- 대화방 목록 -------------------------\r\n";
	for ( auto&& room : g_rooms )
	{
		str += ( "[" + std::to_string( room.second.GetRoomNum() ) + "]	" + room.second.GetCurrentNum() + "	" + room.second.GetName() + "\r\n" );
	}
	str += "---------------------------------------------------------------\r\n";

	const UxInt8* c = str.c_str();
	SendPacket( id, c );
}

UxVoid SendUserProfile( UxInt32 id )
{
	UxString who = GetNextCommand( id );
	User user;
	if ( false == FindUserWithName( who, &user ) )
	{
		SendInvalid( id, EInvalidEvent::NotExistUser );
		return;
	}

	UxString str = "";
	if ( user.IsInRoom() )
	{
		str = "** " + user.GetName() + "님은 현재 " + std::to_string( user.GetRoomNum() ) + "번 대화방에 있습니다,\r\n";
	}
	else
	{
		str = "** " + user.GetName() + "님은 현재 대기실에 있습니다,\r\n";
	}
	str += "** 접속지 : " + user.GetAddr() + "\r\n";

	const UxInt8* c = str.c_str();
	SendPacket( id, c );
}

UxVoid SendRoomInfo( UxInt32 id )
{
	UxInt32 num = std::stoi( GetNextCommand( id ) );

	if ( 0 == g_rooms.count( num ) ) 
	{
		SendInvalid( id, EInvalidEvent::NotExistRoom );
		return;
	}

	UxString str =
		"------------------------- 대화방 정보 -------------------------\r\n"
		"[" + std::to_string( num ) + "]	" + g_rooms[num].GetCurrentNum() + "	" + g_rooms[num].GetName() + "\r\n"
		"생성시간 : " + g_rooms[num].GetOpenTime() + "\r\n";

	for ( auto&& userId : g_rooms[num].GetUsers() )
	{
		str += ( "참여자 : " + g_users[userId].GetName() + "생성시간 : " + g_users[userId].GetRoomJoinTime() + "\r\n" );
	}
	str += "---------------------------------------------------------------\r\n";

	const UxInt8* c = str.c_str();
	SendPacket( id, c );
}

UxInt32 SendOpenRoom( UxInt32 id )
{
	UxInt32 max = std::stoi( GetNextCommand( id ) );
	UxString name = GetNextCommand( id );
	UxInt32 roomNum = ++g_roomCounter;
	Room room( roomNum, name, max );
	g_rooms.insert( std::make_pair( roomNum, room ) );

	UxString str = name + "방이 개설되었습니다\r\n";
	const UxInt8* c = str.c_str();
	SendPacket( id, c );
	return roomNum;
}

UxVoid SendInvite( UxInt32 id )
{
	UxString who = GetNextCommand( id );
	User user;
	if ( false == FindUserWithName( who, &user ) )
	{
		SendInvalid( id, EInvalidEvent::NotExistUser );
		return;
	}
	{
		UxString str = "\r\n# " + g_users[id].GetName() + "님이 " + std::to_string( g_users[id].GetRoomNum() ) + "번 방에서 초대 요청을 했습니다.\r\n";
		const UxInt8* c = str.c_str();
		SendPacket( user.GetId(), c );
	}
	{
		const UxInt8* c = Message::invite.c_str();
		SendPacket( id, c );
	}
}

UxVoid BrodcastRoom( UxInt32 id , ERoomEvent e, UxInt32 roomNum)
{
	UxString str = "";

	switch ( e )
	{
	case ERoomEvent::Join:
		str += g_users[id].GetName() + "님이 채팅방에 참여했습니다.\r\n";
		break;
	case ERoomEvent::Leave:
		str += g_users[id].GetName() + "님이 채팅방을 나갔습니다.\r\n";
		break;
	default:
		break;
	}

	const UxInt8* c = str.c_str();

	for ( auto&& userId : g_rooms[roomNum].GetUsers() )
	{
		SendPacket( userId, c );
	}
}

UxVoid UserQuit( UxInt32 id )
{
	std::cout << g_users[id].GetAddr() << " leave" << std::endl;

	closesocket( g_users[id].GetSocket() );
	WSACloseEvent( g_events[id] );
	g_events[id] = g_events[g_counter - 1];
	g_users[id] = g_users[g_counter - 1];
	g_users[id].SetId( id );
	if ( g_users[id].IsInRoom() )
	{
		g_rooms[g_users[id].GetRoomNum()].ChangeUserId( g_counter - 1, id );
	}
	g_users.erase( g_counter - 1 );
	--g_counter;
}

UxVoid CleanUp( UxInt32 id )
{
	if ( g_users[id].IsInRoom() )
	{
		UxInt32 roomNum = g_users[id].GetRoomNum();
		g_rooms[roomNum].UserLeave( id );
		BrodcastRoom( id, ERoomEvent::Leave , roomNum );

		if ( g_rooms[roomNum].IsRoomEmpty() )
		{
			g_rooms.erase( roomNum );
		}
	}
	SendBye( id );
	g_users[id].SetAccessCancle();
	UserQuit( id );
}

UxVoid CommandHandler( UxInt32 id )
{
	UxString command = g_users[id].GetCommand().substr( 0, g_users[id].GetCommand().find( " " ) );
	std::transform( command.cbegin(), command.cend(), command.begin(), std::toupper );

	//after login
	if ( g_users[id].IsAccess() )
	{
		//명령어 안내
		if ( "H" == command || "/H" == command )
		{
			SendInstruction( id );
		}
		//이용자 목록 보기
		else if ( "US" == command || "/US" == command )
		{
			SendAllUserList( id );
		}
		//대화방 목록 보기
		else if ( "LT" == command || "/LT" == command )
		{
			SendRoomList( id );
		}
		//대화방 정보 보기
		else if ( "ST" == command || "/ST" == command )
		{
			SendRoomInfo( id );
		}
		//이용자 정보 보기
		else if ( "PF" == command || "/PF" == command )
		{
			SendUserProfile( id );
		}
		//쪽지 보내기
		else if ( "TO" == command || "/TO" == command )
		{
			UxString to = GetNextCommand( id );
			User user;
			if ( false == FindUserWithName( to, &user ) )
			{
				SendInvalid( id, EInvalidEvent::NotExistUser );
			}
			else
			{
				SendChat( id, user.GetId() );
			}
		}
		//대화방 만들기
		else if ( "O" == command )
		{
			UxInt32 roomNum = SendOpenRoom( id );
			g_rooms[roomNum].UserJoin( id );
			g_users[id].SetRoomNum( roomNum );
			BrodcastRoom( id, ERoomEvent::Join , roomNum );
		}
		//대화방 참여하기
		else if ( "J" == command )
		{
			UxInt32 roomNum = std::stoi( GetNextCommand( id ) );
			if ( 0 == g_rooms.count( roomNum ) )
			{
				SendInvalid( id, EInvalidEvent::NotExistRoom );
			}
			else if ( g_rooms[roomNum].IsRoomMax() )
			{
				SendInvalid( id, EInvalidEvent::RoomFull );
			}
			else
			{
				g_rooms[roomNum].UserJoin( id );
				g_users[id].SetRoomNum( roomNum );
				BrodcastRoom( id, ERoomEvent::Join , roomNum );
			}
		}
		//초대하기
		else if ( "/IN" == command )
		{
			SendInvite( id );
		}
		//대화방 나가기
		else if ( "/Q" == command )
		{
			UxInt32 roomNum = g_users[id].GetRoomNum();
			g_rooms[roomNum].UserLeave( id );
			BrodcastRoom( id, ERoomEvent::Leave , roomNum );
			g_users[id].LeaveRoom();

			if ( g_rooms[roomNum].IsRoomEmpty() )
			{
				g_rooms.erase( roomNum );
			}
		}
		//끝내기
		else if ( "X" == command )
		{
			CleanUp( id );
		}

		if ( !g_users[id].IsInRoom() )
			SendBasicMention( id );
	}
	//before login
	else
	{
		if ( "LOGIN" == command )
		{
			UxString name = GetNextCommand( id );

			if ( FindUserWithName( name ) )
			{
				SendInvalid( id, EInvalidEvent::AlreadyExistName );
				SendLoginMention( id );
			}
			else
			{
				g_users[id].SetName( name );
				SendWelcome( id );
				SendBasicMention( id );
				g_users[id].SetAccess();
			}
		}
		else
		{
			SendLoginMention( id );
		}

	}
}

UxVoid PacketHandler( UxInt32 id, UxInt8* buff )
{
	//enter
	if ( '\r' == buff[0] )
	{
		std::cout << g_users[id].GetAddr() << " [" << g_users[id].GetName() << "] " << g_users[id].GetCommand() << std::endl;

		if ( g_users[id].IsInRoom() && '/' != g_users[id].GetCommand()[0] )
		{
			SendRoomChat( id );
		}
		else
		{
			CommandHandler( id );
		}
		g_users[id].ClearCommand();
	}
	else if ( '/b' == buff[0] )
	{
		g_users[id].AddBackspace();
	}
	else
	{
		g_users[id].AddCommand( buff );
	}
}

UxVoid main() 
{
	WSADATA wsa;
	WSAStartup( MAKEWORD( 2, 2 ), &wsa );

	SOCKET listener = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons( SERVER_PORT );
	address.sin_addr.s_addr = htonl( INADDR_ANY );
	bind( listener, ( sockaddr* )&address, sizeof( sockaddr_in ) );
	listen( listener, SOMAXCONN );

	WSAEVENT listenEvent = WSACreateEvent();
	WSAEventSelect( listener, listenEvent, FD_ACCEPT );

	g_events[0] = listenEvent;
	g_users[0].SetSocket( listener );

	while ( true ) 
	{
		DWORD res = WSAWaitForMultipleEvents( g_counter, g_events, FALSE, WSA_INFINITE, TRUE );
		if ( res == WSA_WAIT_FAILED )
			break;

		UxInt32 idx = res - WSA_WAIT_EVENT_0;

		WSANETWORKEVENTS networkEvents;
		if ( WSAEnumNetworkEvents( g_users[idx].GetSocket(), g_events[idx], &networkEvents ) == SOCKET_ERROR )
		{
			std::cout << "error" << std::endl;
			break;
		}

		if ( networkEvents.lNetworkEvents & FD_ACCEPT ) 
		{
			sockaddr_in address;
			UxInt32 size = sizeof( address );
			SOCKET client = accept( g_users[idx].GetSocket(), ( sockaddr* )&address, &size );
			g_users[g_counter] = User( g_counter );
			g_users[g_counter].SetSocket( client );
			g_users[g_counter].SetAddr( address.sin_addr, address.sin_port );


			WSAEVENT cEvt = WSACreateEvent();
			WSAEventSelect( client, cEvt, FD_READ | FD_CLOSE );
			g_events[g_counter] = cEvt;

			std::cout << g_users[g_counter].GetAddr() << " access" << std::endl;
			SendLoginMention( g_counter );
			g_counter++;
		}

		if ( networkEvents.lNetworkEvents & FD_READ ) 
		{
			UxInt8 buffer[max_buffer];
			UxInt32 readBytes = recv( g_users[idx].GetSocket(), buffer, max_buffer, 0 );
			buffer[readBytes] = '\0';
			PacketHandler( idx, buffer );
		}

		if ( networkEvents.lNetworkEvents & FD_CLOSE ) 
		{
			if ( g_users[idx].IsAccess() )
			{
				if ( g_users[idx].IsInRoom() )
				{
					UxInt32 roomNum = g_users[idx].GetRoomNum();
					g_rooms[roomNum].UserLeave( idx );
					BrodcastRoom( idx, ERoomEvent::Leave, roomNum );

					if ( g_rooms[roomNum].IsRoomEmpty() )
					{
						g_rooms.erase( roomNum );
					}
				}
			}
			UserQuit( idx );
		}
	}

	closesocket( listener );
	WSACloseEvent( listenEvent );
	WSACleanup();
}

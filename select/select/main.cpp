

#include "typedef.h"
#include "const.h"
#include "User.h"
#include "Room.h"


WSAEVENT g_events[WSA_MAXIMUM_WAIT_EVENTS];
UxInt32 g_counter { 1 };

std::map<UxInt32, User>		g_users;
std::map<UxInt32, Room>		g_rooms;
UxInt32						g_roomCounter { 0 };

//커멘드의 다음 단어 가져오는 함수 (없으면 ""리턴)
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

UxBool IsNumber( const UxString& str )
{
	for ( auto&& c : str )
		if ( std::isdigit( c ) == 0 ) return false;
	return true;
}

//send
UxVoid SendPacket( UxInt32 id, const UxInt8* buff )
{
	send( g_users[id].GetSocket(), buff, strlen(buff), 0 );
}

//쪽지보내기
UxVoid SendChat( UxInt32 id, UxInt32 to )
{
	g_users[id].EraseFirstCommand();
	UxString str = "[" + g_users[id].GetName() + "]	" + g_users[id].GetCommand() +"\r\n";
	SendPacket( to, str.c_str() );
}

//채팅방에서의 채팅
UxVoid SendRoomChat( UxInt32 id )
{
	UxString str = g_users[id].GetName() + "> " + g_users[id].GetCommand() + "\r\n";
	for ( auto&& userId : g_rooms[g_users[id].GetRoomNum()].GetUsers() )
	{
		SendPacket( userId, str.c_str() );
	}
}

UxVoid SendInvalid( UxInt32 id, EInvalidEvent e )
{
	switch ( e )
	{
	case EInvalidEvent::AlreadyExistName:
		SendPacket( id, Message::alreadyExistName.c_str() );
		break;
	case EInvalidEvent::NotExistUser:
		SendPacket( id, Message::notExistUser.c_str() );
		break;
	case EInvalidEvent::NotExistRoom:
		SendPacket( id, Message::notExistRoom.c_str() );
		break;
	case EInvalidEvent::RoomFull:
		SendPacket( id, Message::roomFull.c_str() );
		break;
	case EInvalidEvent::NotFullCommand:
		SendPacket( id, Message::notFullCommand.c_str() );
		break;
	case EInvalidEvent::NotExistCommand:
		SendPacket( id, Message::notExistCommand.c_str() );
	default:
		break;
	}
}

UxVoid SendBasicMention( UxInt32 id )
{
	SendPacket( id, Message::basic.c_str() );
}

UxVoid SendLoginMention( UxInt32 id )
{
	SendPacket( id, Message::login.c_str() );
}

UxVoid SendWelcome( UxInt32 id )
{
	SendPacket( id, Message::welcome.c_str() );
}

UxVoid SendBye( UxInt32 id )
{
	SendPacket( id, Message::bye.c_str() );
}

UxVoid SendInstruction( UxInt32 id )
{
	if ( g_users[id].IsInRoom() )
		SendPacket( id, Message::helpRoom.c_str() );
	else
		SendPacket( id, Message::helpLobby.c_str() );
}

UxVoid SendAllUserList( UxInt32 id )
{
	UxString str = "------------------------- 사람들 목록 -------------------------\r\n";
	for ( auto&& user : g_users )
	{
		str += ("[" + user.second.GetName() + "]		" + user.second.GetAddr() + "\r\n");
	}
	str += "---------------------------------------------------------------\r\n";

	SendPacket( id, str.c_str() );
}

UxVoid SendRoomList( UxInt32 id )
{
	UxString str = "------------------------- 대화방 목록 -------------------------\r\n";
	for ( auto&& room : g_rooms )
	{
		str += ( "[" + std::to_string( room.second.GetRoomNum() ) + "]	" + room.second.GetCurrentNum() + "	" + room.second.GetName() + "\r\n" );
	}
	str += "---------------------------------------------------------------\r\n";

	SendPacket( id, str.c_str() );
}

UxVoid SendUserProfile( UxInt32 id )
{
	UxString who = GetNextCommand( id );
	User user;
	if ( who == "" )
	{
		SendInvalid( id, EInvalidEvent::NotFullCommand );
		return;
	}
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

	SendPacket( id, str.c_str() );
}

UxVoid SendRoomInfo( UxInt32 id )
{
	UxString tmp = GetNextCommand( id );
	if ( tmp == "" )
	{
		SendInvalid( id, EInvalidEvent::NotFullCommand );
		return;
	}
	if ( !IsNumber( tmp ) )
	{
		SendInvalid( id, EInvalidEvent::NotFullCommand );
		return;
	}
	UxInt32 num = std::stoi( tmp );

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
		str += ( "참여자 : " + g_users[userId].GetName() + "	참여시간 : " + g_users[userId].GetRoomJoinTime() + "\r\n" );
	}
	str += "---------------------------------------------------------------\r\n";

	SendPacket( id, str.c_str() );
}

UxInt32 SendOpenRoom( UxInt32 id )
{
	UxString tmp = GetNextCommand( id );
	if ( tmp == "" )
		return -1;
	if ( !IsNumber( tmp ) )
		return -1;
	UxInt32 max = std::stoi( tmp );
	UxString name = GetNextCommand( id );
	if ( name == "" )
		return -1;
	UxInt32 roomNum = ++g_roomCounter;
	Room room( roomNum, name, max );
	g_rooms.insert( std::make_pair( roomNum, room ) );

	UxString str = name + "방이 개설되었습니다\r\n";
	SendPacket( id, str.c_str() );
	return roomNum;
}

UxVoid SendInvite( UxInt32 id )
{
	UxString who = GetNextCommand( id );
	if ( who == "" )
	{
		SendInvalid( id, EInvalidEvent::NotFullCommand );
		return;
	}
	User user;
	if ( false == FindUserWithName( who, &user ) )
	{
		SendInvalid( id, EInvalidEvent::NotExistUser );
		return;
	}
	UxString str = "\r\n# " + g_users[id].GetName() + "님이 " + std::to_string( g_users[id].GetRoomNum() ) + "번 방에서 초대 요청을 했습니다.\r\n";
	SendPacket( user.GetId(), str.c_str() );
	SendPacket( id, Message::invite.c_str() );
}

UxVoid BroadcastRoom( UxInt32 id , ERoomEvent e, UxInt32 roomNum)
{
	UxString str = "";

	switch ( e )
	{
	case ERoomEvent::Join:
		str += g_users[id].GetName() + "님이 채팅방에 참여했습니다...도움말은 /h입력\r\n";
		break;
	case ERoomEvent::Leave:
		str += g_users[id].GetName() + "님이 채팅방을 나갔습니다.\r\n";
		break;
	default:
		break;
	}

	for ( auto&& userId : g_rooms[roomNum].GetUsers() )
		SendPacket( userId, str.c_str() );
}

//클라이언트 종료 처리
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

//x입력시 종료전 user처리
UxVoid CleanUp( UxInt32 id )
{
	if ( g_users[id].IsInRoom() )
	{
		UxInt32 roomNum = g_users[id].GetRoomNum();
		g_rooms[roomNum].UserLeave( id );
		BroadcastRoom( id, ERoomEvent::Leave , roomNum );

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
			if ( to == "" )
			{
				SendInvalid( id, EInvalidEvent::NotFullCommand );
			}
			else
			{
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
		}
		//대화방 만들기
		else if ( "O" == command )
		{
			UxInt32 roomNum = SendOpenRoom( id );
			if ( roomNum == -1 )
			{
				SendInvalid( id, EInvalidEvent::NotFullCommand );
			}
			else
			{
				g_rooms[roomNum].UserJoin( id );
				g_users[id].SetRoomNum( roomNum );
				BroadcastRoom( id, ERoomEvent::Join, roomNum );
			}
		}
		//대화방 참여하기
		else if ( "J" == command )
		{
			UxString tmp = GetNextCommand( id );
			if ( tmp == "" )
			{
				SendInvalid( id, EInvalidEvent::NotFullCommand );
			}
			else if ( !IsNumber( tmp ) )
			{
				SendInvalid( id, EInvalidEvent::NotFullCommand );
			}
			else
			{
				UxInt32 roomNum { 0 };
				try
				{
					roomNum = std::stoi( tmp );
				}
				catch ( const std::exception& e )
				{
					SendInvalid( id, EInvalidEvent::NotFullCommand );
				}
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
					BroadcastRoom( id, ERoomEvent::Join, roomNum );
				}
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
			BroadcastRoom( id, ERoomEvent::Leave , roomNum );
			g_users[id].LeaveRoom();

			if ( g_rooms[roomNum].IsRoomEmpty() )
			{
				g_rooms.erase( roomNum );
			}
		}
		//끝내기
		else if ( "X" == command || "/X" == command )
		{
			CleanUp( id );
		}
		else if( !g_users[id].IsInRoom() )
		{
			SendInvalid( id, EInvalidEvent::NotExistCommand );
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
			if ( name == "" )
			{
				SendLoginMention( id );
			}
			else if ( FindUserWithName( name ) )
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
	//backspace
	else if ( '\b' == buff[0] )
	{
		g_users[id].AddBackspace();
	}
	//user의 command에 쌓아두기
	else
	{
		g_users[id].AddCommand( buff );
	}
}

UxInt32 main( UxInt32 argc, UxInt8* argv[] ) 
{
	UxInt16 inputPort { 0 };
	UxBool	inputPortFlag { false };
	if ( argc > 1 )
	{
		inputPort = (UxInt16)( atoi( argv[1] ) );
		inputPortFlag = true;
	}

	WSADATA wsa;
	WSAStartup( MAKEWORD( 2, 2 ), &wsa );

	SOCKET listener = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	sockaddr_in address;
	address.sin_family = AF_INET;
	if ( inputPortFlag )
	{
		address.sin_port = htons( inputPort );
	}
	else
	{
		std::cout << "기본 포트로 진행됩니다	" << server_port << std::endl;
		address.sin_port = htons( server_port );
	}
	address.sin_addr.s_addr = htonl( INADDR_ANY );
	bind( listener, ( sockaddr* )&address, sizeof( sockaddr_in ) );
	listen( listener, SOMAXCONN );

	WSAEVENT listenEvent = WSACreateEvent();
	WSAEventSelect( listener, listenEvent, FD_ACCEPT );

	g_events[0] = listenEvent;
	g_users[0].SetSocket( listener );

	std::cout << "Running..." << std::endl;
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
					BroadcastRoom( idx, ERoomEvent::Leave, roomNum );

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

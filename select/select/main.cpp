

#include "typedef.h"
#include "const.h"
#include "User.h"
#include "Room.h"


WSAEVENT g_events[WSA_MAXIMUM_WAIT_EVENTS];
UxInt32 g_counter { 1 };

std::map<UxInt32, User>		g_users;
std::map<UxInt32, Room>		g_rooms;
UxInt32						g_roomCounter { 0 };

//Ŀ����� ���� �ܾ� �������� �Լ� (������ ""����)
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

//����������
UxVoid SendChat( UxInt32 id, UxInt32 to )
{
	g_users[id].EraseFirstCommand();
	UxString str = "[" + g_users[id].GetName() + "]	" + g_users[id].GetCommand() +"\r\n";
	SendPacket( to, str.c_str() );
}

//ä�ù濡���� ä��
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
	UxString str = "------------------------- ����� ��� -------------------------\r\n";
	for ( auto&& user : g_users )
	{
		str += ("[" + user.second.GetName() + "]		" + user.second.GetAddr() + "\r\n");
	}
	str += "---------------------------------------------------------------\r\n";

	SendPacket( id, str.c_str() );
}

UxVoid SendRoomList( UxInt32 id )
{
	UxString str = "------------------------- ��ȭ�� ��� -------------------------\r\n";
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
		str = "** " + user.GetName() + "���� ���� " + std::to_string( user.GetRoomNum() ) + "�� ��ȭ�濡 �ֽ��ϴ�,\r\n";
	}
	else
	{
		str = "** " + user.GetName() + "���� ���� ���ǿ� �ֽ��ϴ�,\r\n";
	}
	str += "** ������ : " + user.GetAddr() + "\r\n";

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
		"------------------------- ��ȭ�� ���� -------------------------\r\n"
		"[" + std::to_string( num ) + "]	" + g_rooms[num].GetCurrentNum() + "	" + g_rooms[num].GetName() + "\r\n"
		"�����ð� : " + g_rooms[num].GetOpenTime() + "\r\n";

	for ( auto&& userId : g_rooms[num].GetUsers() )
	{
		str += ( "������ : " + g_users[userId].GetName() + "	�����ð� : " + g_users[userId].GetRoomJoinTime() + "\r\n" );
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

	UxString str = name + "���� �����Ǿ����ϴ�\r\n";
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
	UxString str = "\r\n# " + g_users[id].GetName() + "���� " + std::to_string( g_users[id].GetRoomNum() ) + "�� �濡�� �ʴ� ��û�� �߽��ϴ�.\r\n";
	SendPacket( user.GetId(), str.c_str() );
	SendPacket( id, Message::invite.c_str() );
}

UxVoid BroadcastRoom( UxInt32 id , ERoomEvent e, UxInt32 roomNum)
{
	UxString str = "";

	switch ( e )
	{
	case ERoomEvent::Join:
		str += g_users[id].GetName() + "���� ä�ù濡 �����߽��ϴ�...������ /h�Է�\r\n";
		break;
	case ERoomEvent::Leave:
		str += g_users[id].GetName() + "���� ä�ù��� �������ϴ�.\r\n";
		break;
	default:
		break;
	}

	for ( auto&& userId : g_rooms[roomNum].GetUsers() )
		SendPacket( userId, str.c_str() );
}

//Ŭ���̾�Ʈ ���� ó��
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

//x�Է½� ������ useró��
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
		//��ɾ� �ȳ�
		if ( "H" == command || "/H" == command )
		{
			SendInstruction( id );
		}
		//�̿��� ��� ����
		else if ( "US" == command || "/US" == command )
		{
			SendAllUserList( id );
		}
		//��ȭ�� ��� ����
		else if ( "LT" == command || "/LT" == command )
		{
			SendRoomList( id );
		}
		//��ȭ�� ���� ����
		else if ( "ST" == command || "/ST" == command )
		{
			SendRoomInfo( id );
		}
		//�̿��� ���� ����
		else if ( "PF" == command || "/PF" == command )
		{
			SendUserProfile( id );
		}
		//���� ������
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
		//��ȭ�� �����
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
		//��ȭ�� �����ϱ�
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
		//�ʴ��ϱ�
		else if ( "/IN" == command )
		{
			SendInvite( id );
		}
		//��ȭ�� ������
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
		//������
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
	//user�� command�� �׾Ƶα�
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
		std::cout << "�⺻ ��Ʈ�� ����˴ϴ�	" << server_port << std::endl;
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

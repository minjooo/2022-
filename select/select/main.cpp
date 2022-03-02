

#include "pch.h"
#include "User.h"
#include "Room.h"


std::map<UxInt32, User>		g_users;
std::map<UxInt32, SOCKET>	g_sockets;

std::map<UxInt32, Room>		g_rooms;
UxInt32						g_roomCounter { 0 };


UxString GetNextCommand( UxInt32 id )
{
	g_users[id].EraseFirstCommand();
	return g_users[id].GetCommand().substr( 0, g_users[id].GetCommand().find( " " ) );
}

UxBool FindUserWithName( UxString name, User* user )
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

UxVoid SendPacket( UxInt32 id, const UxInt8* buff )
{
	send( g_sockets[id], buff, strlen(buff), 0 );
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
	//std::vector<UxInt32> v = g_rooms[g_users[id].GetRoomNum()].GetUsers();
	for ( auto&& userId : g_rooms[g_users[id].GetRoomNum()].GetUsers() )
	{
		SendPacket( userId, c );
	}
}

UxVoid SendBasicMention( UxInt32 id )
{
	UxString str =
		"��ɾ�ȳ�(H) ����(X)\r\n"
		"����>  ";
	const UxInt8* c = str.c_str();
	SendPacket( id, c );
}

UxVoid SendLoginMention( UxInt32 id )
{
	UxString str =
		"** �ȳ��ϼ��� �ؽ�Ʈ ä�ü��� ver 0.00001�Դϴ�\r\n"
		"** �α��� ��ɾ�(LOGIN)�� ������ּ���\r\n";
	const UxInt8* c = str.c_str();
	SendPacket( id, c );
}

UxVoid SendWelcome( UxInt32 id )
{
	UxString str =
		"---------------------------------------------------------------\r\n"
		"�ݰ����ϴ�. �ؽ�Ʈ ä�ü��� ver0.000001�Դϴ�.\r\n\r\n"
		"�̿��� �����Ͻ� ���� ������ �Ʒ� �̸��Ϸ� ���� �ٶ��ϴ�\r\n"
		"�����մϴ�\r\n\r\n"
		"---------------------------------------------------------------\r\n";
	const UxInt8* c = str.c_str();
	SendPacket( id, c );
}

UxVoid SendBye( UxInt32 id )
{
	UxString str =
		"---------------------------------------------------------------\r\n"
		"�̿����ּż� �����մϴ�.\r\n\r\n"
		"���� �Ϸ� �ູ�Ͻñ� �ٶ��ϴ�. ^^\r\n"
		"---------------------------------------------------------------\r\n";
	const UxInt8* c = str.c_str();
	SendPacket( id, c );
}

UxVoid SendInstruction( UxInt32 id )
{
	UxString str = "";
	if ( g_users[id].IsInRoom() )
	{
		str +=
			"---------------------------------------------------------------\r\n"
			"/H				��ɾ� �ȳ�\r\n"
			"/US				�̿��� ��� ����\r\n"
			"/LT				��ȭ�� ��� ����\r\n"
			"/ST	[���ȣ]		��ȭ�� ���� ����\r\n"
			"/PF	[����ID]		�̿��� ���� ����\r\n"
			"/TO	[����ID] [�޼���]	���� ������\r\n"
			"/IN	[����ID]		�ʴ��ϱ�\r\n"
			"Q				��ȭ�� ������\r\n"
			"X				������\r\n"
			"---------------------------------------------------------------\r\n";
	}
	else
	{
		str +=
			"---------------------------------------------------------------\r\n"
			"H				��ɾ� �ȳ�\r\n"
			"US				�̿��� ��� ����\r\n"
			"LT				��ȭ�� ��� ����\r\n"
			"ST	[���ȣ]		��ȭ�� ���� ����\r\n"
			"PF	[����ID]		�̿��� ���� ����\r\n"
			"TO	[����ID] [�޼���]	���� ������\r\n"
			"O	[�ִ��ο�] [������]	��ȭ�� �����\r\n"
			"J	[���ȣ]		��ȭ�� �����ϱ�\r\n"
			"X				������\r\n"
			"---------------------------------------------------------------\r\n";
	}
		
	const UxInt8* c = str.c_str();
	SendPacket( id, c );
}

UxVoid SendAllUserList( UxInt32 id )
{
	UxString str = "";
	for ( auto&& user : g_users )
	{
		str += ("[" + user.second.GetName() + "]		" + user.second.GetAddr() + "\r\n");
	}

	const UxInt8* c = str.c_str();
	SendPacket( id, c );
}

UxVoid SendRoomList( UxInt32 id )
{
	UxString str = "";
	for ( auto&& room : g_rooms )
	{
		str += ( "[" + std::to_string( room.second.GetRoomNum() ) + "]	" + room.second.GetCurrentNum() + "	" + room.second.GetName() + "\r\n" );
	}

	const UxInt8* c = str.c_str();
	SendPacket( id, c );
}

UxVoid SendUserProfile( UxInt32 id )
{
	UxString who = GetNextCommand( id );
	User user;
	if ( false == FindUserWithName( who, &user ) )
	{
		//���� ��� ó�� �ʿ�
		return;
	}

	UxString str =
		"** " + user.GetName() + "���� ���� ���ǿ� �ֽ��ϴ�,\r\n"
		"** ������ : " + user.GetAddr() + "\r\n";

	const UxInt8* c = str.c_str();
	SendPacket( id, c );
}

UxVoid SendRoomInfo( UxInt32 id )
{
	UxInt32 num = std::stoi( GetNextCommand( id ) );

	UxString str =
		"------------------------- ��ȭ�� ���� -------------------------\r\n"
		"[" + std::to_string( num ) + "]	" + g_rooms[num].GetCurrentNum() + "	" + g_rooms[num].GetName() + "\r\n";

	for ( auto&& userId : g_rooms[num].GetUsers() )
	{
		str += ( "������ : " + g_users[userId].GetName() + "\r\n" );
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

	UxString str = name + "���� �����Ǿ����ϴ�\r\n";
	const UxInt8* c = str.c_str();
	SendPacket( id, c );
	return roomNum;
}

UxVoid BrodcastRoom( UxInt32 id , ERoomEvent e)
{
	UxString str = "";

	switch ( e )
	{
	case ERoomEvent::Join:
		str += g_users[id].GetName() + "���� ä�ù濡 �����߽��ϴ�.\r\n";
		break;
	case ERoomEvent::Leave:
		str += g_users[id].GetName() + "���� ä�ù��� �������ϴ�.\r\n";
		break;
	case ERoomEvent::Invite:
		break;
	default:
		break;
	}

	const UxInt8* c = str.c_str();

	for ( auto&& userId : g_rooms[g_users[id].GetRoomNum()].GetUsers() )
	{
		SendPacket( userId, c );
	}
}

//���ܵ�
UxVoid SendInvalid( UxInt32 id )
{

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
			//�ִ¹����� Ȯ�� �ʿ�
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
			//���� ��� ó�� �ʿ�
			for ( auto&& user : g_users )
			{
				if ( user.second.GetName() == to )
				{
					SendChat( id, user.first );
					break;
				}
			}
		}
		//��ȭ�� �����
		else if ( "O" == command )
		{
			UxInt32 roomNum = SendOpenRoom( id );
			g_rooms[roomNum].UserJoin( id );
			g_users[id].SetRoomNum( roomNum );
			BrodcastRoom( id, ERoomEvent::Join );
		}
		//��ȭ�� �����ϱ�
		else if ( "J" == command )
		{
			UxInt32 roomNum = std::stoi( GetNextCommand( id ) );
			//�ִ¹����� Ȯ�� �ʿ�
			//���� �������� Ȯ�� �ʿ�
			g_rooms[roomNum].UserJoin( id );
			g_users[id].SetRoomNum( roomNum );
			BrodcastRoom( id, ERoomEvent::Join );
		}
		//�ʴ��ϱ�
		else if ( "/IN" == command )
		{

		}
		//��ȭ�� ������
		else if ( "/Q" == command )
		{
			BrodcastRoom( id, ERoomEvent::Leave );
			g_rooms[g_users[id].GetRoomNum()].UserLeave( id );
			g_users[id].LeaveRoom();
		}
		//������
		else if ( "X" == command )
		{

		}

		if ( !g_users[id].IsInRoom() )
			SendBasicMention( id );
	}
	//before login
	else
	{
		if ( "LOGIN" == command )
		{
			//��ġ�� �̸� ó�� �ʿ�
			UxString name = GetNextCommand( id );
			g_users[id].SetName( name );
			
			SendWelcome( id );
			SendBasicMention( id );
			g_users[id].SetAccess();
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
		std::cout << g_users[id].GetCommand() << std::endl;

		if ( g_users[id].IsInRoom() && '/' != g_users[id].GetCommand()[0] )
		{
			//Ŀ����� ��� �����ؾ� ��
			SendRoomChat( id );
		}
		else
		{
			CommandHandler( id );
		}
		g_users[id].ClearCommand();
	}
	else
	{
		g_users[id].AddCommand( buff );
	}
	//backspaceó�� �ʿ�
}

UxVoid main() {
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


	//SOCKET sockets[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT events[WSA_MAXIMUM_WAIT_EVENTS];
	//sockets[0] = listener;
	events[0] = listenEvent;
	g_sockets[0] = listener;
	UxInt32 counter = 1;


	while ( true ) {
		DWORD res = WSAWaitForMultipleEvents( counter, events, FALSE, WSA_INFINITE, TRUE );
		if ( res == WSA_WAIT_FAILED )
			break;

		UxInt32 idx = res - WSA_WAIT_EVENT_0;

		WSANETWORKEVENTS networkEvents;
		if ( WSAEnumNetworkEvents( g_sockets[idx], events[idx], &networkEvents ) == SOCKET_ERROR )
		{
			std::cout << "error" << std::endl;
			break;
		}

		if ( networkEvents.lNetworkEvents & FD_ACCEPT ) 
		{
			sockaddr_in address;
			UxInt32 size = sizeof( address );
			SOCKET client = accept( g_sockets[idx], ( sockaddr* )&address, &size );

			WSAEVENT cEvt = WSACreateEvent();
			WSAEventSelect( client, cEvt, FD_READ | FD_CLOSE );

			g_sockets[counter] = client;
			g_users[counter] = User( counter, client );
			g_users[counter].SetAddr( address.sin_addr, address.sin_port );
			events[counter] = cEvt;
			std::cout << "new client " << counter << " access	" << g_users[counter].GetAddr() << "\n";
			SendLoginMention( counter );
			counter++;
		}

		if ( networkEvents.lNetworkEvents & FD_READ ) 
		{
			UxInt8 buffer[MAX_BUFFER];
			UxInt32 readBytes = recv( g_sockets[idx], buffer, MAX_BUFFER, 0 );
			buffer[readBytes] = '\0';
			PacketHandler( idx, buffer );
		}

		//���� ���� ó�� �̺�
		if ( networkEvents.lNetworkEvents & FD_CLOSE ) 
		{
			closesocket( g_sockets[idx] );
			WSACloseEvent( events[idx] );
			g_sockets.erase( idx );
			//sockets[idx] = sockets[counter - 1];
			events[idx] = events[counter - 1];
			counter--;
		}
	}

	closesocket( listener );
	WSACloseEvent( listenEvent );
	WSACleanup();
}
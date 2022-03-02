

#include "pch.h"
#include "User.h"
#include "Room.h"

std::map<UxInt32, User>		g_users;
//std::map<UxString, UxInt32>	g_users_access;
std::map<UxInt32, SOCKET>	g_sockets;

std::vector<Room>			g_rooms;

UxVoid SendPacket( UxInt32 id, const UxInt8* buff )
{
	send( g_sockets[id], buff, strlen(buff), 0 );
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
		"------------------------------------------------------\r\n"
		"�ݰ����ϴ�. �ؽ�Ʈ ä�ü��� ver0.000001�Դϴ�.\r\n\r\n"
		"�̿��� �����Ͻ� ���� ������ �Ʒ� �̸��Ϸ� ���� �ٶ��ϴ�\r\n"
		"�����մϴ�\r\n\r\n"
		"-------------------------------------------------------\r\n";
	const UxInt8* c = str.c_str();
	SendPacket( id, c );
}

UxVoid SendBye( UxInt32 id )
{
	UxString str =
		"------------------------------------------------------\r\n"
		"�̿����ּż� �����մϴ�.\r\n\r\n"
		"���� �Ϸ� �ູ�Ͻñ� �ٶ��ϴ�. ^^\r\n"
		"-------------------------------------------------------\r\n";
	const UxInt8* c = str.c_str();
	SendPacket( id, c );
}

UxVoid SendInstruction( UxInt32 id )
{
	UxString str =
		"------------------------------------------------------\r\n"
		"H				��ɾ� �ȳ�\r\n"
		"US				�̿��� ��� ����\r\n"
		"LT				��ȭ�� ��� ����\r\n"
		"ST	[���ȣ]		��ȭ�� ���� ����\r\n"
		"PF	[����ID]		�̿��� ���� ����\r\n"
		"TO	[����ID] [�޼���]	���� ������\r\n"
		"O	[�ִ��ο�] [������]	��ȭ�� �����\r\n"
		"U	[���ȣ]		������\r\n"
		"-------------------------------------------------------\r\n";
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

UxVoid CommandHandler( UxInt32 id )
{
	UxString command = g_users[id].GetCommand().substr( 0, g_users[id].GetCommand().find( " " ) );
	std::transform( command.cbegin(), command.cend(), command.begin(), std::toupper );

	//after login
	if ( g_users[id].IsAccess() )
	{
		//��ɾ� �ȳ�
		if ( "H" == command )
		{
			SendInstruction( id );
		}
		//�̿��� ��� ����
		else if ( "US" == command )
		{
			SendAllUserList( id );
		}
		//��ȭ�� ��� ����
		else if ( "LT" == command )
		{

		}
		//��ȭ�� ���� ����
		else if ( "ST" == command )
		{

		}
		//�̿��� ���� ����
		else if ( "PF" == command )
		{

		}
		//���� ������
		else if ( "TO" == command )
		{

		}
		//��ȭ�� �����
		else if ( "O" == command )
		{

		}
		//������
		else if ( "X" == command )
		{

		}
		SendBasicMention( id );
	}
	//before login
	else
	{
		if ( "LOGIN" == command )
		{
			//��ġ�� �̸� ó�� �ʿ�
			UxString name = g_users[id].GetCommand().substr( g_users[id].GetCommand().find( " " ) + 1, g_users[id].GetCommand().length() - 1 );
			g_users[id].SetName( name );
			//g_users_access.insert( std::make_pair( name, id ) );
			
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
		CommandHandler( id );
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
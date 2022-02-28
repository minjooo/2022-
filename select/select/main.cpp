

#include "pch.h"

UxVoid main() {
	WSADATA wsa;
	WSAStartup( MAKEWORD( 2, 2 ), &wsa );

	SOCKET listener = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons( 3500 );
	address.sin_addr.s_addr = htonl( INADDR_ANY );
	bind( listener, ( sockaddr* )&address, sizeof( sockaddr_in ) );
	listen( listener, SOMAXCONN );

	WSAEVENT listenEvent = WSACreateEvent();
	WSAEventSelect( listener, listenEvent, FD_ACCEPT );

	SOCKET sockets[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT events[WSA_MAXIMUM_WAIT_EVENTS];
	sockets[0] = listener;
	events[0] = listenEvent;
	UxInt32 counter = 1;

	while ( true ) {
		DWORD res = WSAWaitForMultipleEvents( counter, events, FALSE, WSA_INFINITE, TRUE );
		if ( res == WSA_WAIT_FAILED )
			break;

		UxInt32 idx = res - WSA_WAIT_EVENT_0;

		WSANETWORKEVENTS networkEvents;
		if ( WSAEnumNetworkEvents( sockets[idx], events[idx], &networkEvents ) == SOCKET_ERROR )
		{
			std::cout << "error" << std::endl;
			break;
		}

		if ( networkEvents.lNetworkEvents & FD_ACCEPT ) {
			sockaddr_in address;
			UxInt32 size = sizeof( address );
			SOCKET client = accept( sockets[idx], ( sockaddr* )&address, &size );

			WSAEVENT cEvt = WSACreateEvent();
			WSAEventSelect( client, cEvt, FD_READ | FD_CLOSE );

			sockets[counter] = client;
			events[counter] = cEvt;
			counter++;
		}

		if ( networkEvents.lNetworkEvents & FD_READ ) {
			UxInt8 buffer[128];
			UxInt32 readBytes = recv( sockets[idx], buffer, 128, 0 );
			send( sockets[idx], buffer, readBytes, 0 );
		}

		if ( networkEvents.lNetworkEvents & FD_CLOSE ) {
			closesocket( sockets[idx] );
			WSACloseEvent( events[idx] );
			sockets[idx] = sockets[counter - 1];
			events[idx] = events[counter - 1];
			counter--;
		}
	}

	closesocket( listener );
	WSACloseEvent( listenEvent );
	WSACleanup();
}
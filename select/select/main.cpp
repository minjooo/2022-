

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
		"명령어안내(H) 종료(X)\r\n"
		"선택>  ";
	const UxInt8* c = str.c_str();
	SendPacket( id, c );
}

UxVoid SendLoginMention( UxInt32 id )
{
	UxString str =
		"** 안녕하세요 텍스트 채팅서버 ver 0.00001입니다\r\n"
		"** 로그인 명령어(LOGIN)을 사용해주세요\r\n";
	const UxInt8* c = str.c_str();
	SendPacket( id, c );
}

UxVoid SendWelcome( UxInt32 id )
{
	UxString str =
		"---------------------------------------------------------------\r\n"
		"반갑습니다. 텍스트 채팅서버 ver0.000001입니다.\r\n\r\n"
		"이용중 불편하신 점이 있으면 아래 이메일로 문의 바랍니다\r\n"
		"감사합니다\r\n\r\n"
		"---------------------------------------------------------------\r\n";
	const UxInt8* c = str.c_str();
	SendPacket( id, c );
}

UxVoid SendBye( UxInt32 id )
{
	UxString str =
		"---------------------------------------------------------------\r\n"
		"이용해주셔서 감사합니다.\r\n\r\n"
		"오늘 하루 행복하시길 바랍니다. ^^\r\n"
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
			"/H				명령어 안내\r\n"
			"/US				이용자 목록 보기\r\n"
			"/LT				대화방 목록 보기\r\n"
			"/ST	[방번호]		대화방 정보 보기\r\n"
			"/PF	[상대방ID]		이용자 정보 보기\r\n"
			"/TO	[상대방ID] [메세지]	쪽지 보내기\r\n"
			"/IN	[상대방ID]		초대하기\r\n"
			"Q				대화방 나가기\r\n"
			"X				끝내기\r\n"
			"---------------------------------------------------------------\r\n";
	}
	else
	{
		str +=
			"---------------------------------------------------------------\r\n"
			"H				명령어 안내\r\n"
			"US				이용자 목록 보기\r\n"
			"LT				대화방 목록 보기\r\n"
			"ST	[방번호]		대화방 정보 보기\r\n"
			"PF	[상대방ID]		이용자 정보 보기\r\n"
			"TO	[상대방ID] [메세지]	쪽지 보내기\r\n"
			"O	[최대인원] [방제목]	대화방 만들기\r\n"
			"J	[방번호]		대화방 참여하기\r\n"
			"X				끝내기\r\n"
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
		//없을 경우 처리 필요
		return;
	}

	UxString str =
		"** " + user.GetName() + "님은 현재 대기실에 있습니다,\r\n"
		"** 접속지 : " + user.GetAddr() + "\r\n";

	const UxInt8* c = str.c_str();
	SendPacket( id, c );
}

UxVoid SendRoomInfo( UxInt32 id )
{
	UxInt32 num = std::stoi( GetNextCommand( id ) );

	UxString str =
		"------------------------- 대화방 정보 -------------------------\r\n"
		"[" + std::to_string( num ) + "]	" + g_rooms[num].GetCurrentNum() + "	" + g_rooms[num].GetName() + "\r\n";

	for ( auto&& userId : g_rooms[num].GetUsers() )
	{
		str += ( "참여자 : " + g_users[userId].GetName() + "\r\n" );
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

UxVoid BrodcastRoom( UxInt32 id , ERoomEvent e)
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

//예외들
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
			//있는방인지 확인 필요
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
			//없을 경우 처리 필요
			for ( auto&& user : g_users )
			{
				if ( user.second.GetName() == to )
				{
					SendChat( id, user.first );
					break;
				}
			}
		}
		//대화방 만들기
		else if ( "O" == command )
		{
			UxInt32 roomNum = SendOpenRoom( id );
			g_rooms[roomNum].UserJoin( id );
			g_users[id].SetRoomNum( roomNum );
			BrodcastRoom( id, ERoomEvent::Join );
		}
		//대화방 참여하기
		else if ( "J" == command )
		{
			UxInt32 roomNum = std::stoi( GetNextCommand( id ) );
			//있는방인지 확인 필요
			//참여 가능한지 확인 필요
			g_rooms[roomNum].UserJoin( id );
			g_users[id].SetRoomNum( roomNum );
			BrodcastRoom( id, ERoomEvent::Join );
		}
		//초대하기
		else if ( "/IN" == command )
		{

		}
		//대화방 나가기
		else if ( "/Q" == command )
		{
			BrodcastRoom( id, ERoomEvent::Leave );
			g_rooms[g_users[id].GetRoomNum()].UserLeave( id );
			g_users[id].LeaveRoom();
		}
		//끝내기
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
			//겹치는 이름 처리 필요
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
			//커멘드일 경우 제외해야 함
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
	//backspace처리 필요
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

		//접속 종료 처리 미비
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
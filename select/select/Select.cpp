

#include "Select.h"


Select::Select()
{

}

Select::~Select()
{

}

UxBool Select::Initialize()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	m_listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in address;

	address.sin_family = AF_INET;
	std::cout << "기본 포트로 진행됩니다	" << server_port << std::endl;
	address.sin_port = htons(server_port);

	address.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(m_listener, (sockaddr*)&address, sizeof(sockaddr_in));
	listen(m_listener, SOMAXCONN);

	m_listenEvent = WSACreateEvent();
	WSAEventSelect(m_listener, m_listenEvent, FD_ACCEPT);

	m_events[0] = m_listenEvent;
	m_users[0].SetSocket(m_listener);
	return true;
}

UxBool Select::Initialize(UxInt16 port)
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	m_listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in address;
	address.sin_family = AF_INET;

	address.sin_port = htons(port);

	address.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(m_listener, (sockaddr*)&address, sizeof(sockaddr_in));
	listen(m_listener, SOMAXCONN);

	m_listenEvent = WSACreateEvent();
	WSAEventSelect(m_listener, m_listenEvent, FD_ACCEPT);

	m_events[0] = m_listenEvent;
	m_users[0].SetSocket(m_listener);
	return true;
}

UxVoid Select::Run()
{
	std::cout << "Running..." << std::endl;
	while (true)
	{
		DWORD res = WSAWaitForMultipleEvents(m_counter, m_events, FALSE, WSA_INFINITE, TRUE);
		if (res == WSA_WAIT_FAILED)
			break;

		UxInt32 idx = res - WSA_WAIT_EVENT_0;

		WSANETWORKEVENTS networkEvents;
		if (WSAEnumNetworkEvents(m_users[idx].GetSocket(), m_events[idx], &networkEvents) == SOCKET_ERROR)
		{
			std::cout << "error" << std::endl;
			break;
		}

		if (networkEvents.lNetworkEvents & FD_ACCEPT)
		{
			sockaddr_in address;
			UxInt32 size = sizeof(address);
			SOCKET client = accept(m_users[idx].GetSocket(), (sockaddr*)&address, &size);
			m_users[m_counter] = User(m_counter);
			m_users[m_counter].SetSocket(client);
			m_users[m_counter].SetAddr(address.sin_addr, address.sin_port);


			WSAEVENT cEvt = WSACreateEvent();
			WSAEventSelect(client, cEvt, FD_READ | FD_CLOSE);
			m_events[m_counter] = cEvt;

			std::cout << m_users[m_counter].GetAddr() << " access" << std::endl;
			SendLoginMention(m_counter);
			m_counter++;
		}

		if (networkEvents.lNetworkEvents & FD_READ)
		{
			UxInt8 buffer[max_buffer];
			UxInt32 readBytes = recv(m_users[idx].GetSocket(), buffer, max_buffer - 1, 0);
			buffer[readBytes] = '\0';
			PacketHandler(idx, buffer, readBytes);
		}

		if (networkEvents.lNetworkEvents & FD_CLOSE)
		{
			if (m_users[idx].IsAccess())
			{
				if (m_users[idx].IsInRoom())
				{
					UxInt32 roomNum = m_users[idx].GetRoomNum();
					m_rooms[roomNum].UserLeave(idx);
					BroadcastRoom(idx, ERoomEvent::Leave, roomNum);

					if (m_rooms[roomNum].IsRoomEmpty())
					{
						m_rooms.erase(roomNum);
					}
				}
			}
			UserQuit(idx);
		}
	}

	closesocket(m_listener);
	WSACloseEvent(m_listenEvent);
	WSACleanup();
}

UxString Select::GetNextCommand(UxInt32 id)
{
	m_users[id].EraseFirstCommand();
	UxString str = m_users[id].GetCommand().substr(0, m_users[id].GetCommand().find(" "));
	return str.substr(0, str.find("\r"));
}

UxString Select::GetPureCommand(const UxString& com)
{
	UxString str = com;
	return str.substr(0, str.find("\r"));
}

UxBool Select::FindUserWithName(const UxString& name, User* user)
{
	for (auto&& u : m_users)
	{
		if (u.second.GetName() == name)
		{
			*user = u.second;
			return true;
		}
	}
	return false;
}

UxBool Select::FindUserWithName(const UxString& name)
{
	for (auto&& u : m_users)
	{
		if (u.second.GetName() == name)
		{
			return true;
		}
	}
	return false;
}

UxBool Select::IsNumber(const UxString& str)
{
	for (auto&& c : str)
		if (std::isdigit(c) == 0) return false;
	return true;
}

//send
UxVoid Select::SendPacket(UxInt32 id, const UxInt8* buff)
{
	send(m_users[id].GetSocket(), buff, strlen(buff), 0);
}

//쪽지보내기
UxVoid Select::SendChat(UxInt32 id, UxInt32 to)
{
	m_users[id].EraseFirstCommand();
	UxString str = "[" + m_users[id].GetName() + "]	" + m_users[id].GetCommand() + "\r\n";
	SendPacket(to, str.c_str());
}

//채팅방에서의 채팅
UxVoid Select::SendRoomChat(UxInt32 id)
{
	UxString str = m_users[id].GetName() + "> " + m_users[id].GetCommand() + "\r\n";
	for (auto&& userId : m_rooms[m_users[id].GetRoomNum()].GetUsers())
	{
		SendPacket(userId, str.c_str());
	}
}

UxVoid Select::SendInvalid(UxInt32 id, EInvalidEvent e)
{
	switch (e)
	{
	case EInvalidEvent::AlreadyExistName:
		SendPacket(id, Message::alreadyExistName.c_str());
		break;
	case EInvalidEvent::NotExistUser:
		SendPacket(id, Message::notExistUser.c_str());
		break;
	case EInvalidEvent::NotExistRoom:
		SendPacket(id, Message::notExistRoom.c_str());
		break;
	case EInvalidEvent::RoomFull:
		SendPacket(id, Message::roomFull.c_str());
		break;
	case EInvalidEvent::NotFullCommand:
		SendPacket(id, Message::notFullCommand.c_str());
		break;
	case EInvalidEvent::NotExistCommand:
		SendPacket(id, Message::notExistCommand.c_str());
	default:
		break;
	}
}

UxVoid Select::SendBasicMention(UxInt32 id)
{
	SendPacket(id, Message::basic.c_str());
}

UxVoid Select::SendLoginMention(UxInt32 id)
{
	SendPacket(id, Message::login.c_str());
}

UxVoid Select::SendWelcome(UxInt32 id)
{
	SendPacket(id, Message::welcome.c_str());
}

UxVoid Select::SendBye(UxInt32 id)
{
	SendPacket(id, Message::bye.c_str());
}

UxVoid Select::SendInstruction(UxInt32 id)
{
	if (m_users[id].IsInRoom())
		SendPacket(id, Message::helpRoom.c_str());
	else
		SendPacket(id, Message::helpLobby.c_str());
}

UxVoid Select::SendAllUserList(UxInt32 id)
{
	UxString str = "------------------------- 사람들 목록 -------------------------\r\n";
	for (int i = 1; i < m_counter; ++i)
	{
		str += ("[" + m_users[i].GetName() + "]		" + m_users[i].GetAddr() + "\r\n");
	}
	str += "---------------------------------------------------------------\r\n";

	SendPacket(id, str.c_str());
}

UxVoid Select::SendRoomList(UxInt32 id)
{
	UxString str = "------------------------- 대화방 목록 -------------------------\r\n";
	for (auto&& room : m_rooms)
	{
		str += ("[" + std::to_string(room.second.GetRoomNum()) + "]	" + room.second.GetCurrentNum() + "	" + room.second.GetName() + "\r\n");
	}
	str += "---------------------------------------------------------------\r\n";

	SendPacket(id, str.c_str());
}

UxVoid Select::SendUserProfile(UxInt32 id)
{
	UxString who = GetNextCommand(id);
	User user;
	if (who == "")
	{
		SendInvalid(id, EInvalidEvent::NotFullCommand);
		return;
	}
	if (false == FindUserWithName(who, &user))
	{
		SendInvalid(id, EInvalidEvent::NotExistUser);
		return;
	}

	UxString str = "";
	if (user.IsInRoom())
	{
		str = "** " + user.GetName() + "님은 현재 " + std::to_string(user.GetRoomNum()) + "번 대화방에 있습니다,\r\n";
	}
	else
	{
		str = "** " + user.GetName() + "님은 현재 대기실에 있습니다,\r\n";
	}
	str += "** 접속지 : " + user.GetAddr() + "\r\n";

	SendPacket(id, str.c_str());
}

UxVoid Select::SendRoomInfo(UxInt32 id)
{
	UxString tmp = GetNextCommand(id);
	if (tmp == "")
	{
		SendInvalid(id, EInvalidEvent::NotFullCommand);
		return;
	}
	if (!IsNumber(tmp))
	{
		SendInvalid(id, EInvalidEvent::NotFullCommand);
		return;
	}
	UxInt32 num = std::stoi(tmp);

	if (0 == m_rooms.count(num))
	{
		SendInvalid(id, EInvalidEvent::NotExistRoom);
		return;
	}

	UxString str =
		"------------------------- 대화방 정보 -------------------------\r\n"
		"[" + std::to_string(num) + "]	" + m_rooms[num].GetCurrentNum() + "	" + m_rooms[num].GetName() + "\r\n"
		"생성시간 : " + m_rooms[num].GetOpenTime() + "\r\n";

	for (auto&& userId : m_rooms[num].GetUsers())
	{
		str += ("참여자 : " + m_users[userId].GetName() + "	참여시간 : " + m_users[userId].GetRoomJoinTime() + "\r\n");
	}
	str += "---------------------------------------------------------------\r\n";

	SendPacket(id, str.c_str());
}

UxInt32 Select::SendOpenRoom(UxInt32 id)
{
	UxString tmp = GetNextCommand(id);
	if (tmp == "")
		return -1;
	if (!IsNumber(tmp))
		return -1;
	UxInt32 max = std::stoi(tmp);
	UxString name = GetNextCommand(id);
	if (name == "")
		return -1;
	UxInt32 roomNum = ++m_roomCounter;
	Room room(roomNum, name, max);
	m_rooms.insert(std::make_pair(roomNum, room));

	UxString str = name + "방이 개설되었습니다\r\n";
	SendPacket(id, str.c_str());
	return roomNum;
}

UxVoid Select::SendInvite(UxInt32 id)
{
	UxString who = GetNextCommand(id);
	if (who == "")
	{
		SendInvalid(id, EInvalidEvent::NotFullCommand);
		return;
	}
	User user;
	if (false == FindUserWithName(who, &user))
	{
		SendInvalid(id, EInvalidEvent::NotExistUser);
		return;
	}
	UxString str = "\r\n# " + m_users[id].GetName() + "님이 " + std::to_string(m_users[id].GetRoomNum()) + "번 방에서 초대 요청을 했습니다.\r\n";
	SendPacket(user.GetId(), str.c_str());
	SendPacket(id, Message::invite.c_str());
}

UxVoid Select::BroadcastRoom(UxInt32 id, ERoomEvent e, UxInt32 roomNum)
{
	UxString str = "";

	switch (e)
	{
	case ERoomEvent::Join:
		str += m_users[id].GetName() + "님이 채팅방에 참여했습니다...도움말은 /h입력\r\n";
		break;
	case ERoomEvent::Leave:
		str += m_users[id].GetName() + "님이 채팅방을 나갔습니다.\r\n";
		break;
	default:
		break;
	}

	for (auto&& userId : m_rooms[roomNum].GetUsers())
		SendPacket(userId, str.c_str());
}

//클라이언트 종료 처리
UxVoid Select::UserQuit(UxInt32 id)
{
	std::cout << m_users[id].GetAddr() << " leave" << std::endl;

	closesocket(m_users[id].GetSocket());
	WSACloseEvent(m_events[id]);
	m_events[id] = m_events[m_counter - 1];
	m_users[id] = m_users[m_counter - 1];
	m_users[id].SetId(id);
	if (m_users[id].IsInRoom())
	{
		m_rooms[m_users[id].GetRoomNum()].ChangeUserId(m_counter - 1, id);
	}
	m_users.erase(m_counter - 1);
	--m_counter;
}

//x입력시 종료전 user처리
UxVoid Select::CleanUp(UxInt32 id)
{
	if (m_users[id].IsInRoom())
	{
		UxInt32 roomNum = m_users[id].GetRoomNum();
		m_rooms[roomNum].UserLeave(id);
		BroadcastRoom(id, ERoomEvent::Leave, roomNum);

		if (m_rooms[roomNum].IsRoomEmpty())
		{
			m_rooms.erase(roomNum);
		}
	}
	SendBye(id);
	m_users[id].SetAccessCancle();
	UserQuit(id);
}

UxVoid Select::CommandHandler(UxInt32 id)
{
	UxString command = m_users[id].GetCommand().substr(0, m_users[id].GetCommand().find(" "));
	command = GetPureCommand(command);
	std::transform(command.cbegin(), command.cend(), command.begin(), std::toupper);

	//after login
	if (m_users[id].IsAccess())
	{
		//명령어 안내
		if ("H" == command || "/H" == command)
		{
			SendInstruction(id);
		}
		//이용자 목록 보기
		else if ("US" == command || "/US" == command)
		{
			SendAllUserList(id);
		}
		//대화방 목록 보기
		else if ("LT" == command || "/LT" == command)
		{
			SendRoomList(id);
		}
		//대화방 정보 보기
		else if ("ST" == command || "/ST" == command)
		{
			SendRoomInfo(id);
		}
		//이용자 정보 보기
		else if ("PF" == command || "/PF" == command)
		{
			SendUserProfile(id);
		}
		//쪽지 보내기
		else if ("TO" == command || "/TO" == command)
		{
			UxString to = GetNextCommand(id);
			if (to == "")
			{
				SendInvalid(id, EInvalidEvent::NotFullCommand);
			}
			else
			{
				User user;
				if (false == FindUserWithName(to, &user))
				{
					SendInvalid(id, EInvalidEvent::NotExistUser);
				}
				else
				{
					SendChat(id, user.GetId());
				}
			}
		}
		//대화방 만들기
		else if ("O" == command)
		{
			UxInt32 roomNum = SendOpenRoom(id);
			if (roomNum == -1)
			{
				SendInvalid(id, EInvalidEvent::NotFullCommand);
			}
			else
			{
				m_rooms[roomNum].UserJoin(id);
				m_users[id].SetRoomNum(roomNum);
				BroadcastRoom(id, ERoomEvent::Join, roomNum);
			}
		}
		//대화방 참여하기
		else if ("J" == command)
		{
			UxString tmp = GetNextCommand(id);
			if (tmp == "")
			{
				SendInvalid(id, EInvalidEvent::NotFullCommand);
			}
			else if (!IsNumber(tmp))
			{
				SendInvalid(id, EInvalidEvent::NotFullCommand);
			}
			else
			{
				UxInt32 roomNum{ 0 };
				try
				{
					roomNum = std::stoi(tmp);
				}
				catch (const std::exception& e)
				{
					SendInvalid(id, EInvalidEvent::NotFullCommand);
				}
				if (0 == m_rooms.count(roomNum))
				{
					SendInvalid(id, EInvalidEvent::NotExistRoom);
				}
				else if (m_rooms[roomNum].IsRoomMax())
				{
					SendInvalid(id, EInvalidEvent::RoomFull);
				}
				else
				{
					m_rooms[roomNum].UserJoin(id);
					m_users[id].SetRoomNum(roomNum);
					BroadcastRoom(id, ERoomEvent::Join, roomNum);
				}
			}
		}
		//초대하기
		else if ("/IN" == command)
		{
			SendInvite(id);
		}
		//대화방 나가기
		else if ("/Q" == command)
		{
			UxInt32 roomNum = m_users[id].GetRoomNum();
			m_rooms[roomNum].UserLeave(id);
			BroadcastRoom(id, ERoomEvent::Leave, roomNum);
			m_users[id].LeaveRoom();

			if (m_rooms[roomNum].IsRoomEmpty())
			{
				m_rooms.erase(roomNum);
			}
		}
		//끝내기
		else if ("X" == command || "/X" == command)
		{
			CleanUp(id);
		}
		else if ("XX" == command || "/XX" == command)
		{
			UserQuit(id);
		}
		else if (!m_users[id].IsInRoom())
		{
			SendInvalid(id, EInvalidEvent::NotExistCommand);
		}

		if (!m_users[id].IsInRoom())
			SendBasicMention(id);
	}
	//before login
	else
	{
		if ("LOGIN" == command)
		{
			UxString name = GetNextCommand(id);
			if (name == "")
			{
				SendLoginMention(id);
			}
			else if (FindUserWithName(name))
			{
				SendInvalid(id, EInvalidEvent::AlreadyExistName);
				SendLoginMention(id);
			}
			else
			{
				m_users[id].SetName(name);
				SendWelcome(id);
				SendBasicMention(id);
				m_users[id].SetAccess();
			}
		}
		else
		{
			SendLoginMention(id);
		}

	}
}

UxVoid Select::PacketHandler(UxInt32 id, UxInt8* buff, UxInt32 readBytes)
{
	if (readBytes == 0)
	{
		std::cout << "client gone!!!!!!!!!!" << std::endl;
	}
	if (m_users[id].AddCommand(buff))
	{
		std::cout << m_users[id].GetAddr() << " [" << m_users[id].GetName() << "] " << m_users[id].GetCommand();

		if (m_users[id].IsInRoom() && '/' != m_users[id].GetCommand()[0])
		{
			SendRoomChat(id);
		}
		else
		{
			CommandHandler(id);
		}
		m_users[id].ClearCommand();
	}

	////////////////////////
	/*enter
	if ('\n' == buff[readBytes - 1])
	{
		std::cout << m_users[id].GetAddr() << " [" << m_users[id].GetName() << "] " << m_users[id].GetCommand() << std::endl;

		if (m_users[id].IsInRoom() && '/' != m_users[id].GetCommand()[0])
		{
			SendRoomChat(id);
		}
		else
		{
			CommandHandler(id);
		}
		m_users[id].ClearCommand();
	}
	//backspace
	else if ('\b' == buff[readBytes - 1])
	{
		std::cout << "backspace" << std::endl;
		m_users[id].AddBackspace();
	}
	//user의 command에 쌓아두기
	else
	{
		m_users[id].AddCommand(buff);
	}
	*/
}

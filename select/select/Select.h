

#pragma once
#include "typedef.h"
#include "const.h"
#include "User.h"
#include "Room.h"


class Select
{
public:
	Select();
	~Select();

public:
	UxBool Initialize();
	UxBool Initialize(UxInt16 port);

	UxVoid Run();

public:
	UxString GetNextCommand(UxInt32 id);
	UxString GetPureCommand(const UxString& com);
	UxBool FindUserWithName(const UxString& name, User* user);
	UxBool FindUserWithName(const UxString& name);
	UxBool IsNumber(const UxString& str);

	UxVoid SendPacket(UxInt32 id, const UxString& buff);

	UxVoid SendChat(UxInt32 id, UxInt32 to);
	UxVoid SendRoomChat(UxInt32 id);
	UxVoid BroadcastRoom(UxInt32 id, ERoomEvent e, UxInt32 roomNum);

	UxVoid SendInvalid(UxInt32 id, EInvalidEvent e);
	UxVoid SendBasicMention(UxInt32 id);
	UxVoid SendLoginMention(UxInt32 id);
	UxVoid SendWelcome(UxInt32 id);
	UxVoid SendBye(UxInt32 id);
	UxVoid SendInstruction(UxInt32 id);
	UxVoid SendAllUserList(UxInt32 id);
	UxVoid SendRoomList(UxInt32 id);
	UxVoid SendUserProfile(UxInt32 id);
	UxVoid SendRoomInfo(UxInt32 id);
	UxInt32 SendOpenRoom(UxInt32 id);
	UxVoid SendInvite(UxInt32 id);

	UxVoid UserQuit(UxInt32 id);
	UxVoid CleanUp(UxInt32 id);

	UxVoid CommandHandler(UxInt32 id);
	UxVoid PacketHandler(UxInt32 id, UxInt8* buff, UxInt32 readBytes);

protected:

	WSAEVENT m_listenEvent;
	SOCKET m_listener;

	WSAEVENT m_events[WSA_MAXIMUM_WAIT_EVENTS];
	UxInt32 m_counter{ 1 };

	std::map<UxInt32, User>		m_users;

	std::map<UxInt32, Room>		m_rooms;
	UxInt32						m_roomCounter{ 0 };
};


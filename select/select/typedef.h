

#define _CRT_SECURE_NO_WARNINGS
#pragma once


#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <time.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

using UxVoid	= void;
using UxInt8	= char;
using UxUInt8	= unsigned char;
using UxInt16	= short;
using UxUInt16	= unsigned short;
using UxInt32	= int;
using UxUInt32	= unsigned int;
using UxInt64	= __int64;
using UxUInt64	= unsigned __int64;
using UxSingle	= float;
using UxDouble	= double;
using UxBool	= bool;
using UxChar	= wchar_t;
using UxString	= std::string;

constexpr UxInt32 max_buffer = 1024;
#define SERVER_PORT 3500

enum class ERoomEvent
{
	Join,
	Leave
};

enum class EInvalidEvent
{
	AlreadyExistName,
	NotExistUser,
	NotExistRoom,
	RoomFull
};

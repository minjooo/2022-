

#pragma once


#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")


#define MAX_BUFFER 1024
#define SERVER_PORT 3500


typedef void				UxVoid;
typedef char				UxInt8;
typedef unsigned char		UxUInt8;
typedef short				UxInt16;
typedef unsigned short		UxUInt16;
typedef int					UxInt32;
typedef unsigned int		UxUInt32;
typedef __int64				UxInt64;
typedef unsigned __int64	UxUInt64;
typedef float				UxSingle;
typedef double				UxDouble;
typedef bool				UxBool;
typedef wchar_t				UxChar;
typedef std::string			UxString;

enum class ERoomEvent
{
	Join,
	Leave,
	Invite
};



#pragma once
#include "typedef.h"


constexpr UxInt32 max_buffer = 1024;
constexpr UxInt16 server_port = 3500;

namespace Message
{
	const UxString basic =
		"명령어안내(H) 종료(X)\r\n"
		"선택>  ";

	const UxString login =
		"** 로그인 명령어(LOGIN [ID])을 사용해주세요\r\n";

	const UxString welcome =
		"---------------------------------------------------------------\r\n"
		"반갑습니다. 텍스트 채팅서버 ver0.1입니다.\r\n\r\n"
		"이용중 불편하신 점이 있으면 아래 이메일로 문의 바랍니다\r\n"
		"감사합니다\r\n\r\n"
		"---------------------------------------------------------------\r\n";

	const UxString bye =
		"---------------------------------------------------------------\r\n"
		"이용해주셔서 감사합니다.\r\n\r\n"
		"오늘 하루 행복하시길 바랍니다. ^^\r\n"
		"---------------------------------------------------------------\r\n";

	const UxString helpLobby =
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

	const UxString helpRoom =
		"---------------------------------------------------------------\r\n"
		"/H				명령어 안내\r\n"
		"/US				이용자 목록 보기\r\n"
		"/LT				대화방 목록 보기\r\n"
		"/ST	[방번호]		대화방 정보 보기\r\n"
		"/PF	[상대방ID]		이용자 정보 보기\r\n"
		"/TO	[상대방ID] [메세지]	쪽지 보내기\r\n"
		"/IN	[상대방ID]		초대하기\r\n"
		"/Q				대화방 나가기\r\n"
		"/X				끝내기\r\n"
		"---------------------------------------------------------------\r\n";

	const UxString invite = "** 초대 요청을 했습니다.\r\n";

	const UxString alreadyExistName = "이미 존재하는 아이디입니다.\r\n";

	const UxString notExistUser = "존재하지 않는 아이디입니다.\r\n";

	const UxString notExistRoom = "존재하지 않는 방입니다.\r\n";

	const UxString roomFull = "방이 가득찼습니다.\r\n";

	const UxString notFullCommand = "커멘드를 다시 확인해주세요.\r\n";
}
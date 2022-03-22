#include "stdafx.h"

#include "ZChat.h"
#include "MChattingFilter.h"
#include "ZApplication.h"
#include "ZGameInterface.h"
#include "ZPost.h"
#include "ZPostLocal.h"
#include "ZCombatChat.h"
#include "ZCombatInterface.h"
#include "ZIDLResource.h"
#include "MListBox.h"
#include "MLex.h"
#include "MTextArea.h"
#include "ZMyInfo.h"
#include "ZMessages.h"
#include "MUtil.h"
#include "ZConfiguration.h"
#include "ZInterfaceListener.h"
#include "ZNetRepository.h"
#include "ZOptionInterface.h"
bool IsOk=true;

#include "ZApplication.h"
#include "ZChat_CmdID.h"


void ChatCmd_Test(const char* line, const int argc, char **const argv);		
void ChatCmd_Time(const char* line, const int argc, char **const argv);
void ChatCmd_Exit(const char* line, const int argc, char **const argv);		
void ChatCmd_Help(const char* line, const int argc, char **const argv);				// 도움말
void ChatCmd_Go(const char* line, const int argc, char **const argv);					// 게임방 이동
void ChatCmd_Whisper(const char* line, const int argc, char **const argv);			// 귓속말
void ChatCmd_CreateChatRoom(const char* line, const int argc, char **const argv);		// 채팅방 개설
void ChatCmd_JoinChatRoom(const char* line, const int argc, char **const argv);		// 채팅방 참여
void ChatCmd_LeaveChatRoom(const char* line, const int argc, char **const argv);		// 채팅방 탈퇴
void ChatCmd_SelectChatRoom(const char* line, const int argc, char **const argv);		// 채팅방 선택
void ChatCmd_InviteChatRoom(const char* line, const int argc, char **const argv);		// 채팅방 초대
void ChatCmd_VisitChatRoom(const char* line, const int argc, char **const argv);		// 채팅방 초대 참석
void ChatCmd_ChatRoomChat(const char* line, const int argc, char **const argv);		// 채팅
void ChatCmd_CopyToTestServer(const char* line, const int argc, char **const argv);	// 캐릭터전송 - 테스트 서버로 캐릭터 전송
void ChatCmd_StageFollow(const char* line, const int argc, char **const argv);		// 플레이어 추적해서 게임참가
void ChatCmd_Friend(const char* line, const int argc, char **const argv);				// 친구 관련
void ChatCmd_Clan(const char* line, const int argc, char **const argv);				// 클란 관련

void ChatCmd_RequestQuickJoin(const char* line, const int argc, char **const argv);		// 퀵조인
void ChatCmd_Report119(const char* line, const int argc, char **const argv);				// 신고
void ChatCmd_RequestPlayerInfo(const char* line, const int argc, char **const argv);		// 사용자 정보보기
void ChatCmd_Macro(const char* line,const int argc, char **const argv);

// 게임안에서만 가능한 명령어
void ChatCmd_EmotionTaunt(const char* line,const int argc, char **const argv);
void ChatCmd_EmotionBow(const char* line,const int argc, char **const argv);
void ChatCmd_EmotionWave(const char* line,const int argc, char **const argv);
void ChatCmd_EmotionLaugh(const char* line,const int argc, char **const argv);
void ChatCmd_EmotionCry(const char* line,const int argc, char **const argv);
void ChatCmd_EmotionDance(const char* line,const int argc, char **const argv);
void ChatCmd_Suicide(const char* line,const int argc, char **const argv);
void ChatCmd_Callvote(const char* line,const int argc, char **const argv);
void ChatCmd_VoteYes(const char* line,const int argc, char **const argv);
void ChatCmd_VoteNo(const char* line,const int argc, char **const argv);
void ChatCmd_Kick(const char* line,const int argc, char **const argv);
void ChatCmd_MouseSensitivity(const char* line,const int argc, char **const argv);

// test
void ChatCmd_LadderInvite(const char* line,const int argc, char **const argv);
void ChatCmd_LadderTest(const char* line,const int argc, char **const argv);
void ChatCmd_LaunchTest(const char* line,const int argc, char **const argv);

// 퀘스트 테스트용 명령어
void ChatCmd_QUESTTEST_God(const char* line,const int argc, char **const argv);				// 무적모드
void ChatCmd_QUESTTEST_SpawnNPC(const char* line,const int argc, char **const argv);		// NPC 스폰
void ChatCmd_QUESTTEST_NPCClear(const char* line,const int argc, char **const argv);		// NPC 클리어
void ChatCmd_QUESTTEST_Reload(const char* line,const int argc, char **const argv);			// 리소스 리로드
void ChatCmd_QUESTTEST_SectorClear(const char* line,const int argc, char **const argv);		// 섹터 클리어
void ChatCmd_QUESTTEST_Finish(const char* line,const int argc, char **const argv);			// 퀘스트 클리어
void ChatCmd_QUESTTEST_LocalSpawnNPC(const char* line,const int argc, char **const argv);	// npc 스폰 (local)
void ChatCmd_QUESTTEST_WeakNPCs(const char* line,const int argc, char **const argv);		// NPC 에너지를 1로

// admin 명령
void ChatCmd_Framed(const char* line, const int argc, char **const argv); //Wireframe Mode
void ChatCmd_GOD(const char* line, const int argc, char **const argv);   //God Mode
void ChatCmd_AdminBanPlayer(const char* line, const int argc, char **const argv);
void ChatCmd_AdminKickPlayer(const char* line, const int argc, char **const argv);		// 디스커넥트 시킴
void ChatCmd_AdminMutePlayer(const char* line, const int argc, char **const argv);		// 채팅 금지 시킴
void ChatCmd_AdminBlockPlayer(const char* line, const int argc, char **const argv);		// 접속 금지 시킴
void ChatCmd_AdminPingToAll(const char* line, const int argc, char **const argv);		// Garbage Connection 청소 확인
void ChatCmd_AdminAnnounce(const char* line, const int argc, char **const argv);
void ChatCmd_AdminPopup(const char* line, const int argc, char **const argv);
void ChatCmd_AdminSBCheck(const char* line, const int argc, char **const argv);
void ChatCmd_Ban(const char* line, const int argc, char **const argv);
void ChatCmd_AdminTeleport(const char* line, const int argc, char **const argv);
void ChatCmd_AdminTeleport2(const char* line, const int argc, char **const argv);
void ChatCmd_AdminFreezePlayer(const char* line, const int argc, char **const argv);
void ChatCmd_AdminUnFreezePlayer(const char* line, const int argc, char **const argv);
void ChatCmd_Senditem(const char* line, const int argc, char **const argv);

// custom commands.
void ChatCmd_Damage(const char* line, const int argc, char **const argv);
void ChatCmd_FPS(const char* line, const int argc, char **const argv);
void ChatCmd_Visible(const char* line, const int argc, char **const argv);
void ChatCmd_DisableSpecial(const char* line, const int argc, char **const argv);
void ChatCmd_Nomassive(const char* line, const int argc, char **const argv);
void ChatCmd_FPSBar(const char* line, const int argc, char **const argv);
void ChatCmd_Bullet(const char* line, const int argc, char **const argv);
void ChatCmd_Sword(const char* line, const int argc, char **const argv);
void ChatCmd_HealtBar(const char* line, const int argc, char **const argv);
void ChatCmd_Wheellock(const char* line, const int argc, char **const argv);
void ChatCmd_Nonumbers(const char* line, const int argc, char **const argv);
void ChatCmd_Scops(const char* line, const int argc, char **const argv);
void ChatCmd_Nosmoke(const char* line, const int argc, char **const argv);
void ChatCmd_Damagebar(const char* line, const int argc, char **const argv);
void ChatCmd_Timebar(const char* line, const int argc, char **const argv);
void ChatCmd_Positionbar(const char* line, const int argc, char **const argv);
void ChatCmd_Autorec(const char* line, const int argc, char **const argv);
void ChatCmd_SHOTGUN3DSOUND(const char* line, const int argc, char **const argv);
void ChatCmd_VIPAnnounce(const char* line, const int argc, char **const argv);
void ChatCmd_ClearChat(const char* line, const int argc, char **const argv);
void ChatCmd_NextSong(const char* line, const int argc, char **const argv);
void ChatCmd_PreviousSong(const char* line, const int argc, char **const argv);
void ChatCmd_Staffcommands(const char* line, const int argc, char **const argv);

#ifdef _ADMIN_HALT
void ChatCmd_AdminServerHalt(const char* line, const int argc, char **const argv);		// 서버 종료
#endif
void ChatCmd_AdminSwitchCreateLadderGame(const char* line, const int argc, char **const argv);		// 클랜전 여부 설정
void ChatCmd_AdminReloadClientHash(const char* line, const int argc, char **const argv);		// XTrap 클라이언트 Hash 리로드
void ChatCmd_AdminResetAllHackingBlock( const char* line, const int argc, char **const argv );
void ChatCmd_AdminReloadGambleitem( const char* line, const int argc, char **const argv );
void ChatCmd_AdminDumpGambleitemLog( const char* line, const int argc, char **const argv );
void ChatCmd_AdminAssasin( const char* line, const int argc, char **const argv );

// event 진행운영자 명령
void ChatCmd_ChangeMaster(const char* line, const int argc, char **const argv);			// 방장권한 빼앗어옴
void ChatCmd_ChangePassword(const char* line, const int argc, char **const argv);		// 방 패스워드 바꿈
void ChatCmd_AdminHide(const char* line, const int argc, char **const argv);			// 투명인간
void ChatCmd_RequestJjang(const char* line, const int argc, char **const argv);
void ChatCmd_RemoveJjang(const char* line, const int argc, char **const argv);

///////////////////////////////////////////////////////////////////////////////////////////////

void _AddCmdFromXml(ZChatCmdManager* pCmdManager, ZCmdXmlParser* pParser, 
				int nCmdID, ZChatCmdProc* fnProc, unsigned long int flag,
				int nMinArgs, int nMaxArgs, bool bRepeatEnabled)
{
	ZCmdXmlParser::_CmdStr* pCmdStr = pParser->GetStr(nCmdID);
	if (pCmdStr)
	{
		pCmdManager->AddCommand(nCmdID, pCmdStr->szName, fnProc, flag, nMinArgs, nMaxArgs, bRepeatEnabled, 
			pCmdStr->szUsage, pCmdStr->szHelp);

		for (int i = 0; i < (int)pCmdStr->vecAliases.size(); i++)
		{
			pCmdManager->AddAlias(pCmdStr->vecAliases[i].c_str(), pCmdStr->szName);
		}
	}
}

#define _CC_AC(X1,X2,X3,X4,X5,X6,X7,X8)		m_CmdManager.AddCommand(0,X1,X2,X3,X4,X5,X6,X7,X8)
#define _CC_ALIAS(NEW,TAR)					m_CmdManager.AddAlias(NEW,TAR)
#define _CC_ACX(X1,X2,X3,X4,X5,X6)			_AddCmdFromXml(&m_CmdManager,&parser,X1,X2,X3,X4,X5,X6)

// 채팅명령어 사용법은 http://iworks.maietgames.com/mdn/wiki.php/건즈;채팅명령어 에 적어주세요.. by bird

void ZChat::InitCmds()
{	
	ZCmdXmlParser parser;
	if (!parser.ReadXml(ZApplication::GetFileSystem(), FILENAME_CHATCMDS))
	{
		MLog("Error while Read Item Descriptor %s", "system/chatcmds.xml");
	}

	_CC_ACX(CCMD_ID_HELP,				&ChatCmd_Help,				CCF_ALL, ARGVNoMin, ARGVNoMax, true);
	_CC_ACX(CCMD_ID_WHISPER,			&ChatCmd_Whisper,			CCF_ALL, ARGVNoMin, 1, false);
	_CC_ACX(CCMD_ID_REPORT119,			&ChatCmd_Report119,			CCF_ALL, ARGVNoMin, ARGVNoMax, true);
	_CC_ACX(CCMD_ID_FRIEND,				&ChatCmd_Friend,			CCF_ALL, ARGVNoMin, 1, false);
	_CC_ACX(CCMD_ID_CLAN,				&ChatCmd_Clan,				CCF_ALL, ARGVNoMin, ARGVNoMax, false);
	_CC_ACX(CCMD_ID_STAGE_FOLLOW,		&ChatCmd_StageFollow,		CCF_LOBBY, ARGVNoMin, 1, true);
	_CC_ACX(CCMD_ID_REQUEST_QUICK_JOIN,	&ChatCmd_RequestQuickJoin,	CCF_LOBBY, ARGVNoMin, ARGVNoMax, true);
	_CC_ACX(CCMD_ID_EMOTION_TAUNT,		&ChatCmd_EmotionTaunt,		CCF_GAME, ARGVNoMin, ARGVNoMax,true);
	_CC_ACX(CCMD_ID_EMOTION_BOW,		&ChatCmd_EmotionBow  ,		CCF_GAME, ARGVNoMin, ARGVNoMax,true);
	_CC_ACX(CCMD_ID_EMOTION_WAVE,		&ChatCmd_EmotionWave ,		CCF_GAME, ARGVNoMin, ARGVNoMax,true);
	_CC_ACX(CCMD_ID_EMOTION_LAUGH,		&ChatCmd_EmotionLaugh,		CCF_GAME, ARGVNoMin, ARGVNoMax,true);
	_CC_ACX(CCMD_ID_EMOTION_CRY,		&ChatCmd_EmotionCry  ,		CCF_GAME, ARGVNoMin, ARGVNoMax,true);
	_CC_ACX(CCMD_ID_EMOTION_DANCE,		&ChatCmd_EmotionDance,		CCF_GAME, ARGVNoMin, ARGVNoMax,true);
	_CC_ACX(CCMD_ID_MACRO,				&ChatCmd_Macro,				CCF_GAME, ARGVNoMin, ARGVNoMax,true);
	_CC_ACX(CCMD_ID_SUICIDE,			&ChatCmd_Suicide,			CCF_GAME, ARGVNoMin, ARGVNoMax,true);
	_CC_ACX(CCMD_ID_CALLVOTE,			&ChatCmd_Callvote,			CCF_GAME, ARGVNoMin, ARGVNoMax,true);
	_CC_ACX(CCMD_ID_VOTEYES,			&ChatCmd_VoteYes,			CCF_GAME, ARGVNoMin, ARGVNoMax,true);
	_CC_ACX(CCMD_ID_VOTENO,				&ChatCmd_VoteNo,			CCF_GAME, ARGVNoMin, ARGVNoMax,true);
	_CC_ACX(CCMD_ID_KICK,				&ChatCmd_Kick,				CCF_GAME, ARGVNoMin, ARGVNoMax,true);
	_CC_ACX(CCMD_ID_CREATE_CHATROOM,	&ChatCmd_CreateChatRoom,	CCF_ALL, ARGVNoMin, 1,true);
	_CC_ACX(CCMD_ID_JOIN_CHATROOM,		&ChatCmd_JoinChatRoom  ,	CCF_ALL, ARGVNoMin, 1,true);
	_CC_ACX(CCMD_ID_LEAVE_CHATROOM,		&ChatCmd_LeaveChatRoom ,	CCF_ALL, ARGVNoMin, 1,true);
	_CC_ACX(CCMD_ID_SELECT_CHATROOM,	&ChatCmd_SelectChatRoom,	CCF_ALL, ARGVNoMin, 1,true);
	_CC_ACX(CCMD_ID_INVITE_CHATROOM,	&ChatCmd_InviteChatRoom,	CCF_ALL, ARGVNoMin, 1,true);
	_CC_ACX(CCMD_ID_VISIT_CHATROOM,		&ChatCmd_VisitChatRoom ,	CCF_ALL, ARGVNoMin, 1,true);
	_CC_ACX(CCMD_ID_CHAT_CHATROOM,		&ChatCmd_ChatRoomChat  ,	CCF_ALL, ARGVNoMin, 1,false);
	_CC_ACX(CCMD_ID_MOUSE_SENSITIVITY,	&ChatCmd_MouseSensitivity,	CCF_GAME, ARGVNoMin, 1,true);


////////////////////////////////////////////////////////////////////
	// admin
	_CC_AC("ban",           &ChatCmd_Ban,               CCF_ADMIN|CCF_STAGE|CCF_GAME, ARGVNoMin, 2, true, "/ban <player>", "");
	_CC_AC("admin_ban",     &ChatCmd_AdminBanPlayer,    CCF_ADMIN, ARGVNoMin, 1, true,"/admin_ban <charname>", "");
	_CC_AC("admin_kick",	&ChatCmd_AdminKickPlayer,	CCF_ADMIN, ARGVNoMin, 1, true, "/admin_kick <charname>", "");			// 2010-08-09 수정됨 - 홍기주
	_CC_AC("admin_mute",	&ChatCmd_AdminMutePlayer,	CCF_ADMIN, ARGVNoMin, 2, true, "/admin_mute <charname> <due>", "");		// 2010-08-09 수정됨 - 홍기주
	_CC_AC("admin_block",	&ChatCmd_AdminBlockPlayer,	CCF_ADMIN, ARGVNoMin, 2, true, "/admin_block <charname> <due>", "");	// 2010-08-09 수정됨 - 홍기주	
	_CC_AC("admin_frame",		&ChatCmd_Framed,	    CCF_ADMIN|CCF_GAME, ARGVNoMin, ARGVNoMax, true, "/admin_frame", "");     //Wireframe Mode
	_CC_AC("admin_god",         &ChatCmd_GOD,           CCF_ADMIN|CCF_GAME, ARGVNoMin, ARGVNoMax, true, "/admin_god", "");           //God Mode

	_CC_AC("admin_pingtoall",	&ChatCmd_AdminPingToAll,		CCF_ADMIN, ARGVNoMin, ARGVNoMax, true,"/admin_pingtoall", "");
	_CC_AC("admin_wall",		&ChatCmd_AdminAnnounce,			CCF_ADMIN, ARGVNoMin, 1 , true,"/admin_wall <메시지>", "");
	_CC_AC("admin_popup",		&ChatCmd_AdminPopup,			CCF_ADMIN, ARGVNoMin, 1 , true,"/admin_popup <message>", "");
	_CC_AC("shotbotcheck",		&ChatCmd_AdminSBCheck,		    CCF_ADMIN, ARGVNoMin, 1 , true,"/shotbotcheck <name>", "");
	_CC_AC("teleport",			&ChatCmd_AdminTeleport,		    CCF_ADMIN, ARGVNoMin, 1 , true,"/teleport <name>", "");
	_CC_AC("teleportplayer",	&ChatCmd_AdminTeleport2,		CCF_ADMIN, ARGVNoMin, 1 , true,"/teleportplayer <name>", "");
	_CC_AC("freeze_player",		&ChatCmd_AdminFreezePlayer,		CCF_ADMIN, ARGVNoMin, 1 , true,"/freeze_player <name>", "");
	_CC_AC("unfreeze_player",	&ChatCmd_AdminUnFreezePlayer,	CCF_ADMIN, ARGVNoMin, 1 , true,"/unfreeze_player <name>", "");
	_CC_AC("admin_senditem",    &ChatCmd_Senditem,              CCF_ADMIN, ARGVNoMin, 3 , true, "/admin_senditem <charname> <itemid> <rentdays>", "");
	_CC_AC("staff",	            &ChatCmd_Staffcommands,	        CCF_ADMIN, ARGVNoMin, 1 , true, "/staff", "");

	//custom commands.
	_CC_AC("damage",			&ChatCmd_Damage,				CCF_ALL, ARGVNoMin, 1 , true,"/damage", "");
	_CC_AC("fps",				&ChatCmd_FPS,					CCF_ALL, ARGVNoMin, 1 , true,"/fps <number>", "");
	_CC_AC("visible",			&ChatCmd_Visible,				CCF_ALL, ARGVNoMin, 1 , true,"/visible", "");
	_CC_AC("specialitem",	    &ChatCmd_DisableSpecial,	    CCF_ALL, ARGVNoMin, 1, true, "/specialitem", "");
	_CC_AC("nomassive",	        &ChatCmd_Nomassive,	            CCF_ALL, ARGVNoMin, 1, true, "/nomassive", "");
	_CC_AC("fpsbar",	        &ChatCmd_FPSBar,	            CCF_ALL, ARGVNoMin, 1, true, "/fpsbar", "");
	_CC_AC("bullet",	        &ChatCmd_Bullet,	            CCF_ALL, ARGVNoMin, 1, true, "/bullet", "");
	_CC_AC("swordtrail",	    &ChatCmd_Sword,	                CCF_ALL, ARGVNoMin, 1, true, "/swordtrail", "");
	_CC_AC("healtbar",	        &ChatCmd_HealtBar,	            CCF_ALL, ARGVNoMin, 1, true, "/healtbar", "");
	_CC_AC("wheellock",	        &ChatCmd_Wheellock,	            CCF_ALL, ARGVNoMin, 1, true, "/wheellock", "");
	_CC_AC("nonumbers",	        &ChatCmd_Nonumbers,	            CCF_ALL, ARGVNoMin, 1, true, "/nonumbers", "");
	_CC_AC("snipercrosshair",	&ChatCmd_Scops,	                CCF_ALL, ARGVNoMin, 1, true, "/snipercrosshair", "");
	_CC_AC("nosmoke",	        &ChatCmd_Nosmoke,	            CCF_ALL, ARGVNoMin, 1, true, "/nosmoke", "");
	_CC_AC("damagebar",	        &ChatCmd_Damagebar,	            CCF_ALL, ARGVNoMin, 1, true, "/damagebar", "");
	_CC_AC("timebar",	        &ChatCmd_Timebar,	            CCF_ALL, ARGVNoMin, 1, true, "/timebar", "");
	_CC_AC("positionbar",	    &ChatCmd_Positionbar,	        CCF_ALL, ARGVNoMin, 1, true, "/positionbar", "");
	_CC_AC("autorec",	        &ChatCmd_Autorec,	            CCF_ALL, ARGVNoMin, 1, true, "/autorec", "");
    _CC_AC("sound",	            &ChatCmd_SHOTGUN3DSOUND,	    CCF_ALL, ARGVNoMin, 1, true, "/sound", "");
	_CC_AC("clearchat",	        &ChatCmd_ClearChat,	            CCF_ALL, ARGVNoMin, 1, true, "/clearchat", "");
	_CC_AC("nextsong",	        &ChatCmd_NextSong,	            CCF_ALL, ARGVNoMin, 1, true, "/nextsong", "");
	_CC_AC("prevsong",	        &ChatCmd_PreviousSong,	        CCF_ALL, ARGVNoMin, 1, true, "/prevsong", "");
	
	//VIP commands
    _CC_AC("vip_wall",		    &ChatCmd_VIPAnnounce,			CCF_ALL, ARGVNoMin, 1 , true,"/vip_wall <메시지>", "");

#ifdef _ADMIN_HALT
	_CC_AC("admin_halt",		&ChatCmd_AdminServerHalt,		CCF_ADMIN, ARGVNoMin, ARGVNoMax, true,"/admin_halt", "");
#endif
	_CC_AC("changemaster",		&ChatCmd_ChangeMaster,			CCF_ADMIN|CCF_STAGE|CCF_GAME, ARGVNoMin, ARGVNoMax, true,"/changemaster", "");
	_CC_AC("changepassword",	&ChatCmd_ChangePassword,		CCF_ADMIN|CCF_STAGE|CCF_GAME, ARGVNoMin, ARGVNoMax, true,"/changepassword", "");
	_CC_AC("admin_hide",		&ChatCmd_AdminHide,				CCF_ADMIN|CCF_LOBBY, ARGVNoMin, ARGVNoMax, true,"/admin_hide", "");
	_CC_AC("hide",				&ChatCmd_AdminHide,				CCF_ADMIN|CCF_LOBBY, ARGVNoMin, ARGVNoMax, true,"/hide", "");
	_CC_AC("jjang",				&ChatCmd_RequestJjang,			CCF_ADMIN|CCF_STAGE|CCF_GAME, ARGVNoMin, ARGVNoMax, true,"/jjang", "");
	_CC_AC("removejjang",		&ChatCmd_RemoveJjang,			CCF_ADMIN|CCF_STAGE|CCF_GAME, ARGVNoMin, ARGVNoMax, true,"/removejjang", "");
	_CC_AC("admin_reload_hash", &ChatCmd_AdminReloadClientHash,	CCF_ADMIN, ARGVNoMin, ARGVNoMax, true,"/admin_reload_hash", "");

	_CC_AC("admin_switch_laddergame",		&ChatCmd_AdminSwitchCreateLadderGame,	CCF_ADMIN, ARGVNoMin, ARGVNoMax, true,"/admin_switch_laddergame 1", "");		
	_CC_AC("admin_reset_all_hacking_block", &ChatCmd_AdminResetAllHackingBlock,		CCF_ADMIN, ARGVNoMin, ARGVNoMax, true, "/admin_reset_all_hacking_block", "");
	_CC_AC("admin_reload_gambleitem",		&ChatCmd_AdminReloadGambleitem,			CCF_ADMIN, ARGVNoMin, ARGVNoMax, true, "/admin_reload_gambleitem", "");
	_CC_AC("admin_dump_gambleitem_log",		&ChatCmd_AdminDumpGambleitemLog,		CCF_ADMIN, ARGVNoMin, ARGVNoMax, true, "/admin_dump_gambleitem_log", "");
	_CC_AC("admin_commander",				&ChatCmd_AdminAssasin,					CCF_ADMIN|CCF_GAME, ARGVNoMin, ARGVNoMax, true, "/admin_commander", "");
	
	_CC_ALIAS("공지", "admin_wall");
	_CC_ALIAS("종료", "admin_halt");

	// player
	_CC_AC("exit",		&ChatCmd_Exit,		CCF_ALL, ARGVNoMin, ARGVNoMax, true,"/exit", "");
    _CC_AC("time",   	&ChatCmd_Time,		CCF_ALL, ARGVNoMin, ARGVNoMax, true,"/time", "");


#ifdef _DEBUG
	//_CC_AC("캐릭터전송", &ChatCmd_CopyToTestServer, CCF_LOBBY, ARGVNoMin, 0, "/캐릭터전송", "캐릭터 정보를 테스트서버에 복사합니다.");
	_CC_AC("team", &ChatCmd_LadderInvite, CCF_LOBBY, ARGVNoMin, ARGVNoMax, true, "", "");
	_CC_AC("test", &ChatCmd_Test, CCF_ALL, ARGVNoMin, 1, true ,"/test <name>", "테스트를 수행합니다.");
	_CC_AC("laddertest", &ChatCmd_LadderTest, CCF_ADMIN|CCF_ALL, ARGVNoMin, ARGVNoMax, true ,"/laddertest", "래더테스트를 요청합니다.");
	_CC_AC("launchtest", &ChatCmd_LaunchTest, CCF_ADMIN|CCF_ALL, ARGVNoMin, ARGVNoMax, true ,"/laddertest", "래더테스트를 요청합니다.");
#endif

	_CC_AC("go",		&ChatCmd_Go, CCF_LOBBY, ARGVNoMin, 1, true, "/go 방번호", "게임방으로 바로 이동합니다.");

#ifndef _PUBLISH
{

	// 아직 다 구현되지 않았음
	_CC_AC("pf",		&ChatCmd_RequestPlayerInfo, CCF_LOBBY|CCF_STAGE, ARGVNoMin, 2, true ,"/pf <캐릭터이름>", "다른 사용자의 정보를 봅니다.");
	_CC_ALIAS("ㅔㄹ", "pf");
}
#endif

	// 테스트 전용
	_CC_AC("gtgod",			&ChatCmd_QUESTTEST_God,				CCF_TEST, ARGVNoMin, 1    , true,"/gtgod", "");
	_CC_AC("gtspn",			&ChatCmd_QUESTTEST_SpawnNPC,		CCF_TEST, ARGVNoMin, 2    , true,"/gtspn <NPC타입> <NPC수>", "");
	_CC_AC("gtclear",		&ChatCmd_QUESTTEST_NPCClear,		CCF_TEST, ARGVNoMin, 1    , true,"/gtclear", "");
	_CC_AC("gtreload",		&ChatCmd_QUESTTEST_Reload,			CCF_TEST, ARGVNoMin, 1    , true,"/gtreload", "");
	_CC_AC("gtsc",			&ChatCmd_QUESTTEST_SectorClear,		CCF_TEST, ARGVNoMin, 1    , true,"/gtsc", "");
	_CC_AC("gtfin",			&ChatCmd_QUESTTEST_Finish,			CCF_TEST, ARGVNoMin, 1    , true,"/gtfin", "");
	_CC_AC("gtlspn",		&ChatCmd_QUESTTEST_LocalSpawnNPC,	CCF_TEST, ARGVNoMin, 2    , true,"/gtlspn <NPC타입> <NPC수>", "");
	_CC_AC("gtweaknpcs",	&ChatCmd_QUESTTEST_WeakNPCs,		CCF_TEST, ARGVNoMin, 1    , true,"/gtweaknpcs", "");
}



void OutputCmdHelp(const char* cmd)
{
	ZChatCmdManager* pCCM = ZGetGameInterface()->GetChat()->GetCmdManager();
	ZChatCmd* pCmd = pCCM->GetCommandByName(cmd);
	if (pCmd == NULL) return;

	if ( (pCmd->GetFlag() & CCF_ADMIN) && !ZGetMyInfo()->IsAdminGrade())
		return;

    char szBuf[512];
	sprintf(szBuf, "%s: %s", pCmd->GetName(), pCmd->GetHelp());
	ZChatOutput(szBuf, ZChat::CMT_SYSTEM);
}

void OutputCmdUsage(const char* cmd)
{
	ZChatCmdManager* pCCM = ZGetGameInterface()->GetChat()->GetCmdManager();
	ZChatCmd* pCmd = pCCM->GetCommandByName(cmd);
	if (pCmd == NULL) return;

	if ( (pCmd->GetFlag() & CCF_ADMIN) && !ZGetMyInfo()->IsAdminGrade())
		return;

    char szBuf[512];
	sprintf(szBuf, "%s: %s", ZMsg(MSG_WORD_USAGE), pCmd->GetUsage());
	ZChatOutput(szBuf, ZChat::CMT_SYSTEM);
}

void OutputCmdWrongArgument(const char* cmd)
{
	ZChatOutput( ZMsg(MSG_WRONG_ARGUMENT) );
	OutputCmdUsage(cmd);
}

////////////////////////////////////////////////////////////////////////
void ChatCmd_Time(const char* line, const int argc, char **const argv)
{
    char sztemph[512];
	char sztempd[512];
	time_t currentTime;
	struct tm *timeinfo;
	currentTime= time(NULL);
	timeinfo= localtime(&currentTime);
	strftime (sztemph, 30, "^2Time : %H:%M:%S." , timeinfo);
	ZChatOutput(sztemph);
	strftime (sztempd, 30, "^2Date : %d/%m/%Y." , timeinfo);
	ZChatOutput(sztempd);
	return;
}
void ChatCmd_Exit(const char* line, const int argc, char **const argv)
{
ExitProcess(0);
}
void ChatCmd_Help(const char* line, const int argc, char **const argv)
{

	ZChatOutput("Keybords:", ZChat::CMT_SYSTEM);
	ZChatOutput("--------------------------------", ZChat::CMT_SYSTEM);
	ZChatOutput("CTRL + S - HP/AP output", ZChat::CMT_SYSTEM);
	ZChatOutput("CTRL + F - Your FPS output", ZChat::CMT_SYSTEM);
	ZChatOutput("--------------------------------", ZChat::CMT_SYSTEM);
	ZChatOutput("Room tag mods:", ZChat::CMT_SYSTEM);
	ZChatOutput("[VAMP] - Vampire mode", ZChat::CMT_SYSTEM);
	ZChatOutput("[NJ] - Ninja jump", ZChat::CMT_SYSTEM);
	ZChatOutput("[IA] - Infinite ammo", ZChat::CMT_SYSTEM);
	ZChatOutput("[R] - Instant Reload", ZChat::CMT_SYSTEM);
	ZChatOutput("[G=Number] - Gravity", ZChat::CMT_SYSTEM);
	ZChatOutput("--------------------------------", ZChat::CMT_SYSTEM);

	ZChatCmdManager* pCCM = ZGetGameInterface()->GetChat()->GetCmdManager();

	char szBuf[1024] = "";

	//////////////////////////////////////////////////////////

	GunzState state = ZApplication::GetGameInterface()->GetState();

	if( state==GUNZ_GAME ) {
		// 자신의 캐릭터가 레벨이 1-10 사이인 경우만..
		if(ZGetMyInfo()) {
			if(ZGetMyInfo()->GetLevel() < 10) {
				if( ZGetGame() ) {
					ZGetGame()->m_HelpScreen.ChangeMode();
					return;
				}
			}
		}
	}

	if (argc == 1)
	{
		
		ZChatCmdFlag nCurrFlag = CCF_NONE;

		switch (state)
		{
			case GUNZ_LOBBY: nCurrFlag = CCF_LOBBY; break;
			case GUNZ_STAGE: nCurrFlag = CCF_STAGE; break;
			case GUNZ_GAME: nCurrFlag = CCF_GAME; break;
		}

		sprintf(szBuf, "%s: ", ZMsg(MSG_WORD_COMMANDS));

		int nCnt=0;
		int nCmdCount = pCCM->GetCmdCount();

		for (ZChatCmdMap::iterator itor = pCCM->GetCmdBegin(); itor != pCCM->GetCmdEnd(); ++itor)
		{
			nCnt++;
			ZChatCmd* pCmd = (*itor).second;

			if (pCmd->GetFlag() & CCF_ADMIN) continue;
			if (!(pCmd->GetFlag() & nCurrFlag)) continue;

			strcat(szBuf, pCmd->GetName());

			if (nCnt != nCmdCount) strcat(szBuf, ", ");
		}

		// 엄청난 하드코딩... 어쩔수 없다... -ㅇ-;
		switch (state)
		{
			case GUNZ_LOBBY:
				strcat( szBuf, "go");
				break;
			case GUNZ_STAGE:
				strcat( szBuf, "kick");
				break;
			case GUNZ_GAME:
				break;
		}

		ZChatOutput(szBuf, ZChat::CMT_SYSTEM);

		sprintf(szBuf, "%s: /h %s", ZMsg(MSG_WORD_HELP), ZMsg(MSG_WORD_COMMANDS));
		ZChatOutput(szBuf, ZChat::CMT_SYSTEM);
	}
	else if (argc == 2)
	{
		OutputCmdHelp(argv[1]);
		OutputCmdUsage(argv[1]);
	}
}

void ChatCmd_Go(const char* line, const int argc, char **const argv)
{
	if (argc < 2) return;

	ZRoomListBox* pRoomList = (ZRoomListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_StageList");
	if (pRoomList == NULL)
		return;

	int nRoomNo = atoi(argv[1]);

	ZPostStageGo(nRoomNo);
}

void ChatCmd_Whisper(const char* line, const int argc, char **const argv)
{
	if (argc < 2) 
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	if (ZGetMyInfo()->GetUGradeID() == MMUG_CHAT_LIMITED)
	{
		ZChatOutput( ZMsg(MSG_CANNOT_CHAT) );
		return;
	}


	char* pszSenderName = "Me";	// 아무거나 보내도 서버에서 채워넣음

	char szName[512] = "";
	char szRName[512] = "";

	MLex lex;
	char* pszMsg = lex.GetOneArg(argv[1], szName, szRName);

	if ( (int)strlen( pszMsg) > 127 && !strstr(pszMsg, "Reporter Name"))
		return;

	// 욕필터링
	if (!ZGetGameInterface()->GetChat()->CheckChatFilter(pszMsg)) return;

	//귓속말 캐릭터 이름 글자 수 제한..jintriple3
	int nNameLen = (int)strlen(szName);
	if ( nNameLen < MIN_CHARNAME)		// 이름이 너무 짧다.
	{
		const char *str = ZErrStr( MERR_TOO_SHORT_NAME );
		if(str)
		{
			char text[1024];
			sprintf(text, "%s (E%d)", str, MERR_TOO_SHORT_NAME);
			ZChatOutput(MCOLOR(96,96,168), text, ZChat::CL_CURRENT);
		}
	}
	else if ( nNameLen > MAX_CHARNAME)		// 이름이 제한 글자수를 넘었다.
	{
		const char *str = ZErrStr( MERR_TOO_LONG_NAME );
		if(str)
		{
			char text[1024];
			sprintf(text, "%s (E%d)", str, MERR_TOO_LONG_NAME);
			ZChatOutput(MCOLOR(96,96,168), text, ZChat::CL_CURRENT);
		}
	}
	else
	{
		ZPostWhisper(pszSenderName, szName, pszMsg);

		// loop back
		char szMsg[512];
		sprintf(szMsg, "(To %s) : %s", szRName, pszMsg);	//jintriple3 유저 네임은 그대로 출력되도록...
		ZChatOutput(MCOLOR(96,96,168), szMsg, ZChat::CL_CURRENT);
	}
}

void ChatCmd_CreateChatRoom(const char* line, const int argc, char **const argv)
{
	if (argc < 2) 
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	char* pszCharName = argv[1];

	if( !MGetChattingFilter()->IsValidStr( pszCharName, 1) ){
		char szMsg[ 256 ];
		ZTransMsg( szMsg, MSG_WRONG_WORD_NAME, 1, MGetChattingFilter()->GetLastFilteredStr());
		ZApplication::GetGameInterface()->ShowMessage( szMsg, NULL, MSG_WRONG_WORD_NAME );
	}
	else
	{
		ZChatOutput( 
			ZMsg(MSG_LOBBY_REQUESTING_CREATE_CHAT_ROOM), 
			ZChat::CMT_SYSTEM );

		ZPostChatRoomCreate(ZGetMyUID(), pszCharName);
	}
}
void ChatCmd_JoinChatRoom(const char* line, const int argc, char **const argv)
{
	if (argc < 2) 
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	ZChatOutput( 
		ZMsg(MSG_LOBBY_REQUESTING_JOIN_CAHT_ROOM), 
		ZChat::CMT_SYSTEM );

	char* pszChatRoomName = argv[1];

	ZPostChatRoomJoin(pszChatRoomName);
}

void ChatCmd_LeaveChatRoom(const char* line, const int argc, char **const argv)
{
	if (argc < 2) 
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	ZChatOutput( 
		ZMsg(MSG_LOBBY_LEAVE_CHAT_ROOM), 
		ZChat::CMT_SYSTEM );

	char* pszRoomName = argv[1];

	ZPostChatRoomLeave(pszRoomName);
}

void ChatCmd_SelectChatRoom(const char* line, const int argc, char **const argv)
{
	if (argc < 2) 
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	ZChatOutput( 
		ZMsg(MSG_LOBBY_CHOICE_CHAT_ROOM), 
		ZChat::CMT_SYSTEM );

	char* pszRoomName = argv[1];

	ZPostSelectChatRoom(pszRoomName);
}

void ChatCmd_InviteChatRoom(const char* line, const int argc, char **const argv)
{
	if (argc < 2) 
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}
	char* pszPlayerName = argv[1];

	char szLog[128];
	
	ZTransMsg( szLog, MSG_LOBBY_INVITATION, 1, pszPlayerName );

	ZChatOutput(szLog, ZChat::CMT_SYSTEM);

	ZPostInviteChatRoom(pszPlayerName);
}

void ChatCmd_VisitChatRoom(const char* line, const int argc, char **const argv)
{
	char* pszRoomName = const_cast<char*>(ZGetGameClient()->GetChatRoomInvited());
	ZPostChatRoomJoin(pszRoomName);
}

void ChatCmd_ChatRoomChat(const char* line, const int argc, char **const argv)
{
	if (argc < 2) 
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}
	char* pszMsg = argv[1];

	// 욕필터링
	if (!ZGetGameInterface()->GetChat()->CheckChatFilter(pszMsg)) return;

	ZPostChatRoomChat(pszMsg);
}


void ChatCmd_CopyToTestServer(const char* line, const int argc, char **const argv)
{
	// 사용하지 않는다. - 테스트 서버로 정보 복사
	return;


	if (argc != 1) return;

	static unsigned long int st_nLastTime = 0;
	unsigned long int nNowTime = timeGetTime();

#define DELAY_POST_COPY_TO_TESTSERVER		(1000 * 60)		// 5분 딜레이

	if ((nNowTime - st_nLastTime) > DELAY_POST_COPY_TO_TESTSERVER)
	{
		ZPostRequestCopyToTestServer(ZGetGameClient()->GetPlayerUID());

		st_nLastTime = nNowTime;
	}
}


void ChatCmd_StageFollow(const char* line, const int argc, char **const argv)
{
	if (argc < 2) 
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	char* pszTarget = argv[1];

	ZPostStageFollow(pszTarget);
}

void ChatCmd_Friend(const char* line, const int argc, char **const argv)
{
	//// ZFriendCommandHelper ////
	class ZFriendCommandHelper {
	public:
		enum ZFRIENDCMD {
			ZFRIENDCMD_ADD,
			ZFRIENDCMD_REMOVE,
			ZFRIENDCMD_LIST,
			ZFRIENDCMD_MSG,
			ZFRIENDCMD_UNKNOWN
		};
		ZFRIENDCMD GetSubCommand(const char* pszCmd) {
			if (stricmp(pszCmd, "Add") == 0)
				return ZFRIENDCMD_ADD;
			else if (stricmp(pszCmd, "추가") == 0)
				return ZFRIENDCMD_ADD;
			else if (stricmp(pszCmd, "Remove") == 0)
				return ZFRIENDCMD_REMOVE;
			else if (stricmp(pszCmd, "삭제") == 0)
				return ZFRIENDCMD_REMOVE;
			else if (stricmp(pszCmd, "list") == 0)
				return ZFRIENDCMD_LIST;
			else if (stricmp(pszCmd, "목록") == 0)
				return ZFRIENDCMD_LIST;
			else if (stricmp(pszCmd, "msg") == 0)
				return ZFRIENDCMD_MSG;
			else if (stricmp(pszCmd, "채팅") == 0)
				return ZFRIENDCMD_MSG;
			else return ZFRIENDCMD_UNKNOWN;
		}
	} friendHelper;

	if (argc < 2) 
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}
	char szSubCmd[256] = "";
	char szArg[256] = "";

	MLex lex;
	char* pszMsg = lex.GetOneArg(argv[1], szSubCmd);
	
	//// Sub Command Handler ////
	switch(friendHelper.GetSubCommand(szSubCmd)) {
	case ZFriendCommandHelper::ZFRIENDCMD_ADD:
		{
			lex.GetOneArg(pszMsg, szArg);
			ZPostFriendAdd(szArg);
		}
		break;
	case ZFriendCommandHelper::ZFRIENDCMD_REMOVE:
		{
			lex.GetOneArg(pszMsg, szArg);
			ZPostFriendRemove(szArg);
		}
		break;
	case ZFriendCommandHelper::ZFRIENDCMD_LIST:
		{
			ZPostFriendList();
		}
		break;
	case ZFriendCommandHelper::ZFRIENDCMD_MSG:
		{
			lex.GetOneArg(pszMsg, szArg);
			ZPostFriendMsg(szArg);
		}
		break;
	default:
		OutputDebugString("Unknown Friend Command \n");
		break;
	};
}

void ChatCmd_Clan(const char* line, const int argc, char **const argv)
{
	

	//// ZClanCommandHelper ////
	class ZClanCommandHelper {
	public:
		enum ZCLANCMD {
			ZCLANCMD_CREATE,		// 클랜 생성
			ZCLANCMD_CLOSE,			// 클랜 폐쇄
			ZCLANCMD_JOIN,
			ZCLANCMD_LEAVE,
			ZCLANCMD_EXPEL_MEMBER,	// 강제탈퇴
			ZCLANCMD_LIST,
			ZCLANCMD_MSG,
			
			ZCLANCMD_CHANGE_GRADE,	// 멤버 권한 변경

			ZCLANCMD_UNKNOWN
		};
		ZCLANCMD GetSubCommand(const char* pszCmd) {
			GunzState nGameState = ZApplication::GetGameInterface()->GetState();

			if ((stricmp(pszCmd, "생성") == 0) || (stricmp(pszCmd, "open") == 0))
			{
				if (nGameState == GUNZ_LOBBY) return ZCLANCMD_CREATE;
			}
			else if ((stricmp(pszCmd, "폐쇄") == 0) || (stricmp(pszCmd, "해체") == 0) || (stricmp(pszCmd, "close") == 0))
			{
				if (nGameState == GUNZ_LOBBY) return ZCLANCMD_CLOSE;
			}
			else if ( (stricmp(pszCmd, "초대") == 0) || (stricmp(pszCmd, "invite") == 0) )
			{
				if (nGameState == GUNZ_LOBBY) return ZCLANCMD_JOIN;
			}
			else if ( (stricmp(pszCmd, "탈퇴") == 0) || (stricmp(pszCmd, "leave") == 0) )
			{
				if (nGameState == GUNZ_LOBBY) return ZCLANCMD_LEAVE;
			}
			else if ( (stricmp(pszCmd, "권한변경") == 0) || (stricmp(pszCmd, "promote") == 0) )
			{
				if (nGameState == GUNZ_LOBBY) return ZCLANCMD_CHANGE_GRADE;
			}
			else if ((stricmp(pszCmd, "강제탈퇴") == 0) || (stricmp(pszCmd, "방출") == 0) || (stricmp(pszCmd, "dismiss") == 0))
			{
				if (nGameState == GUNZ_LOBBY) return ZCLANCMD_EXPEL_MEMBER;
			}
			else if ((stricmp(pszCmd, "list") == 0) || (stricmp(pszCmd, "목록") == 0))
			{
				if (nGameState == GUNZ_LOBBY) return ZCLANCMD_LIST;
			}
			else if ((stricmp(pszCmd, "msg") == 0) || (stricmp(pszCmd, "채팅") == 0))
			{
				return ZCLANCMD_MSG;
			}
			
			return ZCLANCMD_UNKNOWN;
		}
	} clanHelper;

	if (argc < 2) 
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}
	char szSubCmd[256] = "";

	MLex lex;
	char* pszMsg = lex.GetOneArg(argv[1], szSubCmd);
	
	//// Sub Command Handler ////
	switch(clanHelper.GetSubCommand(szSubCmd)) {
	case ZClanCommandHelper::ZCLANCMD_CREATE:
		{
			// clan 생성 클랜이름 발기인1 발기인2 발기인3 발기인4
			if (argc < 7)
			{
				OutputCmdWrongArgument(argv[0]);
				break;
			}

			ZGetGameClient()->RequestCreateClan(argv[2], &argv[3]);
		}
		break;
	case ZClanCommandHelper::ZCLANCMD_CLOSE:
		{
			// clan 폐쇄 클랜이름
			if (argc < 3)
			{
				OutputCmdWrongArgument(argv[0]);
				break;
			}

			if (ZGetMyInfo()->GetClanGrade() != MCG_MASTER)
			{
				ZChatOutput( 
					ZMsg(MSG_CLAN_ENABLED_TO_MASTER), 
					ZChat::CMT_SYSTEM );
				break;
			}

			// 클랜이름 확인
			if (stricmp(ZGetMyInfo()->GetClanName(), argv[2]))
			{
				ZChatOutput(
					ZMsg(MSG_CLAN_WRONG_CLANNAME), 
					ZChat::CMT_SYSTEM );
				break;
			}

			ZApplication::GetGameInterface()->ShowConfirmMessage(
				ZMsg(MSG_CLAN_CONFIRM_CLOSE), 
				ZGetClanCloseConfirmListenter()	);
		}
		break;
	case ZClanCommandHelper::ZCLANCMD_JOIN:
		{
			// clan 초대 가입자이름
			if (argc < 3)
			{
				OutputCmdWrongArgument(argv[0]);
				break;
			}

			if (!ZGetMyInfo()->IsClanJoined())
			{
				ZChatOutput(
					ZMsg(MSG_CLAN_NOT_JOINED), 
					ZChat::CMT_SYSTEM );
				break;
			}

			if (!IsUpperClanGrade(ZGetMyInfo()->GetClanGrade(), MCG_ADMIN))
			{
				ZChatOutput(
					ZMsg(MSG_CLAN_ENABLED_TO_MASTER_AND_ADMIN), 
					ZChat::CMT_SYSTEM );
				break;
			}

			char szClanName[256];
			strcpy(szClanName, 	ZGetMyInfo()->GetClanName());
			ZPostRequestJoinClan(ZGetGameClient()->GetPlayerUID(), szClanName, argv[2]);
		}
		break;
	case ZClanCommandHelper::ZCLANCMD_LEAVE:
		{
			// clan 탈퇴
			if (argc < 2)
			{
				OutputCmdWrongArgument(argv[0]);
				break;
			}

			if (!ZGetMyInfo()->IsClanJoined())
			{
				ZChatOutput(
					ZMsg(MSG_CLAN_NOT_JOINED), 
					ZChat::CMT_SYSTEM );
				break;
			}

			// 마스터는 탈퇴가 안된다.
			if (IsUpperClanGrade(ZGetMyInfo()->GetClanGrade(), MCG_MASTER))
			{
				ZChatOutput(
					ZMsg(MSG_CLAN_MASTER_CANNOT_LEAVED), 
					ZChat::CMT_SYSTEM);
				break;
			}

			ZApplication::GetGameInterface()->ShowConfirmMessage(
				ZMsg(MSG_CLAN_CONFIRM_LEAVE), 
				ZGetClanLeaveConfirmListenter() );
		}
		break;
	case ZClanCommandHelper::ZCLANCMD_CHANGE_GRADE:
		{
			// clan 권한변경 멤버이름 권한이름
			if (argc < 4)
			{
				OutputCmdWrongArgument(argv[0]);
				break;
			}

			if (!ZGetMyInfo()->IsClanJoined())
			{
				ZChatOutput(
					ZMsg(MSG_CLAN_NOT_JOINED), 
					ZChat::CMT_SYSTEM );
				break;
			}

			if (!IsUpperClanGrade(ZGetMyInfo()->GetClanGrade(), MCG_MASTER))
			{
				ZChatOutput(
					ZMsg(MSG_CLAN_ENABLED_TO_MASTER), 
					ZChat::CMT_SYSTEM );
				break;
			}

			char szMember[256];
			int nClanGrade = 0;

			strcpy(szMember, argv[2]);
			if ((strlen(szMember) < 0) || (strlen(szMember) > CLAN_NAME_LENGTH))
			{
				OutputCmdWrongArgument(argv[0]);
				break;
			}

			if ((!stricmp(argv[3], "클랜운영자")) || (!stricmp(argv[3], "운영자")) || (!stricmp(argv[3], "영자")) || (!stricmp(argv[3], "admin")))
			{
				nClanGrade = (int)MCG_ADMIN;
			}
			else if ((!stricmp(argv[3], "클랜멤버")) || (!stricmp(argv[3], "멤버")) || (!stricmp(argv[3], "클랜원")) || (!stricmp(argv[3], "member")))
			{
				nClanGrade = (int)MCG_MEMBER;
			}
			else
			{
				OutputCmdWrongArgument(argv[0]);
				break;
			}


			ZPostRequestChangeClanGrade(ZGetGameClient()->GetPlayerUID(), szMember, nClanGrade);
		}
		break;
	case ZClanCommandHelper::ZCLANCMD_EXPEL_MEMBER:
		{
			// clan 강제탈퇴 클랜멤버
			if (argc < 3)
			{
				OutputCmdWrongArgument(argv[0]);
				break;
			}

			if (!ZGetMyInfo()->IsClanJoined())
			{
				ZChatOutput(
					ZMsg(MSG_CLAN_NOT_JOINED), 
					ZChat::CMT_SYSTEM );
				break;
			}

			if (!IsUpperClanGrade(ZGetMyInfo()->GetClanGrade(), MCG_ADMIN))
			{
				ZChatOutput(
					ZMsg(MSG_CLAN_ENABLED_TO_MASTER_AND_ADMIN), 
					ZChat::CMT_SYSTEM );
				break;
			}

			char szMember[256];
			int nClanGrade = 0;

			strcpy(szMember, argv[2]);
			if ((strlen(szMember) < 0) || (strlen(szMember) > CLAN_NAME_LENGTH))
			{
				OutputCmdWrongArgument(argv[0]);
				break;
			}

			ZPostRequestExpelClanMember(ZGetGameClient()->GetPlayerUID(), szMember);
		}
		break;
	case ZClanCommandHelper::ZCLANCMD_LIST:
		{

		}
		break;
	case ZClanCommandHelper::ZCLANCMD_MSG:
		{
			if (ZGetMyInfo()->GetUGradeID() == MMUG_CHAT_LIMITED)
			{
				ZChatOutput( ZMsg(MSG_CANNOT_CHAT) );
				break;
			}

			// clan msg 하고싶은말
			MLex lex;
			
			char szLine[512], szTemp1[256] = "", szTemp2[256] = "";
			strcpy(szLine, line);

			char* pszMsg = lex.GetTwoArgs(szLine, szTemp1, szTemp2);

			ZPostClanMsg(ZGetGameClient()->GetPlayerUID(), pszMsg);
		}
		break;
	default:
		ZChatOutput( ZMsg(MSG_CANNOT_CHAT) );
		break;
	};
}

void ChatCmd_RequestQuickJoin(const char* line, const int argc, char **const argv)
{
	ZGetGameInterface()->RequestQuickJoin();
}

void ChatCmd_Report119(const char* line, const int argc, char **const argv)
{
	ZPostLocalReport119();
}

void ChatCmd_AdminBanPlayer(const char* line, const int argc, char **const argv)
{
	if (argc < 2) 
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}
	char* pszPlayerName = argv[1];

	ZPostAdminRequestBanPlayer(ZGetGameClient()->GetPlayerUID(), pszPlayerName);
}
void ChatCmd_AdminKickPlayer(const char* line, const int argc, char **const argv)
{
	if (argc < 2) {
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	char* pszPlayerName = argv[1];
	ZPostAdminRequestKickPlayer(pszPlayerName);
}

int GetDueHour(char* pszDue)
{
	int nLength = (int)strlen(pszDue);

	for(int i = 0; i < nLength - 1; i++ ) {
		if( pszDue[i] > '9' || pszDue[i] < '0') {
			return -1;
		}
	}

	int nDueType = toupper(pszDue[nLength - 1]);
	if( nDueType == toupper('d') ) {
		int nDay = atoi(pszDue);
		if( nDay < 365 * 10)	return nDay * 24;
		else					return -1;		
	} 
	else if( nDueType == toupper('h')) {
		int nHour = atoi(pszDue);
		return nHour;
	} 
	else {
		return -1;
	}
}

void ChatCmd_AdminMutePlayer(const char* line, const int argc, char **const argv)
{
	if (argc < 3) {
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	char* pszPlayerName = argv[1];

	int nDueHour = GetDueHour(argv[2]);
	if( nDueHour < 0 ) {
		OutputCmdWrongArgument(argv[0]);
		return;
	}

#ifdef _DEBUG
	mlog("Request Mute on Player(%s) While %d Hour\n", pszPlayerName, nDueHour);
#endif

	ZPostAdminRequestMutePlayer(pszPlayerName, nDueHour);
}

void ChatCmd_AdminBlockPlayer(const char* line, const int argc, char **const argv)
{
	if (argc < 3) {
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	char* pszPlayerName = argv[1];

	int nDueHour = GetDueHour(argv[2]);
	if( nDueHour < 0 ) {
		OutputCmdWrongArgument(argv[0]);
		return;
	}

#ifdef _DEBUG
	mlog("Request Block on Player(%s) While %d Hour\n", pszPlayerName, nDueHour);
#endif

	ZPostAdminRequestBlockPlayer(pszPlayerName, nDueHour);
}

void ChatCmd_AdminPingToAll(const char* line, const int argc, char **const argv)
{
	ZPostAdminPingToAll();
}

void ChatCmd_AdminReloadClientHash(const char* line, const int argc, char **const argv)
{
	ZPostAdminReloadClientHash();
}


void ChatCmd_AdminResetAllHackingBlock( const char* line, const int argc, char **const argv )
{
	ZPostAdminResetAllHackingBlock();
}

void ChatCmd_AdminReloadGambleitem( const char* line, const int argc, char **const argv )
{
	ZPostAdminReloadGambleItem();
}


void ChatCmd_AdminDumpGambleitemLog( const char* line, const int argc, char **const argv )
{
	ZPostAdminDumpGambleItemLog();
}

void ChatCmd_AdminAssasin( const char* line, const int argc, char **const argv )
{
	ZPostAdminAssasin();
}


void ChatCmd_ChangeMaster(const char* line, const int argc, char **const argv)
{
	ZPostChangeMaster();
}

void ChatCmd_ChangePassword(const char* line, const int argc, char **const argv)
{
	if (argc < 2) 
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}
	char* pszPassword = argv[1];

	ZPostChangePassword(pszPassword);
}

void ChatCmd_AdminHide(const char* line, const int argc, char **const argv)
{
	ZPostAdminHide();
}
void ChatCmd_Ban(const char* line, const int argc, char **const argv) {
    if (argc < 2)
    {
        OutputCmdWrongArgument(argv[0]);
        return;
    }
    char* pszTargetName = argv[1];
 
               ZPostAdminBan(pszTargetName);
}
void ChatCmd_RequestJjang(const char* line, const int argc, char **const argv)
{
	if (argc < 2) 
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}
	char* pszTargetName = argv[1];

	ZPostAdminRequestJjang(pszTargetName);
}

void ChatCmd_RemoveJjang(const char* line, const int argc, char **const argv)
{
	if (argc < 2) 
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}
	char* pszTargetName = argv[1];

	ZPostAdminRemoveJjang(pszTargetName);
}
void ChatCmd_Test(const char* line, const int argc, char **const argv)
{
	ZChatOutput("Testing...", ZChat::CMT_SYSTEM);
	
	if (argc < 2) 
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}
	char* pszCharName = argv[1];

	ZGameClient* pClient = ZGetGameClient();
	MMatchPeerInfoList* pList = pClient->GetPeers();
	for (MMatchPeerInfoList::iterator i=pList->begin(); i!= pList->end(); i++) {
		MMatchPeerInfo* pInfo = (*i).second;
		if(stricmp(pInfo->CharInfo.szName, pszCharName) == 0) {
			MCommand* pCmd = pClient->CreateCommand(MC_TEST_PEERTEST_PING, pInfo->uidChar);
			pClient->Post(pCmd);
		}
	}
}

void ChatCmd_Macro(const char* line, const int argc, char **const argv)
{
// config 에 등록 저장 - 키입력 누를때처럼
// 
	if(argc != 3)
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	int mode = -1;

		 if( stricmp(argv[1], "1")==0 ) mode = 0;
	else if( stricmp(argv[1], "2")==0 ) mode = 1;
	else if( stricmp(argv[1], "3")==0 ) mode = 2;
	else if( stricmp(argv[1], "4")==0 ) mode = 3;
	else if( stricmp(argv[1], "5")==0 ) mode = 4;
	else if( stricmp(argv[1], "6")==0 ) mode = 5;
	else if( stricmp(argv[1], "7")==0 ) mode = 6;
	else if( stricmp(argv[1], "8")==0 ) mode = 7;
	else if( stricmp(argv[1], "9")==0 ) mode = 8;
	else 
		return;

	ZCONFIG_MACRO* pMacro = NULL;

	if(ZGetConfiguration())
		pMacro = ZGetConfiguration()->GetMacro();
	
	if( pMacro && argv[2] ) {
		strcpy( pMacro->szMacro[mode],argv[2] );
		ZGetConfiguration()->Save( Z_LOCALE_XML_HEADER);
	}
}

void ChatCmd_EmotionTaunt(const char* line,const int argc, char **const argv)
{
	if(ZGetGame())
		ZGetGame()->PostSpMotion( ZC_SPMOTION_TAUNT );
}

void ChatCmd_EmotionBow(const char* line,const int argc, char **const argv)
{
	if(ZGetGame())
		ZGetGame()->PostSpMotion( ZC_SPMOTION_BOW );
}

void ChatCmd_EmotionWave(const char* line,const int argc, char **const argv)
{
	if(ZGetGame())
		ZGetGame()->PostSpMotion( ZC_SPMOTION_WAVE );
}

void ChatCmd_EmotionLaugh(const char* line,const int argc, char **const argv)
{
	if(ZGetGame())
		ZGetGame()->PostSpMotion( ZC_SPMOTION_LAUGH );
}

void ChatCmd_EmotionCry(const char* line,const int argc, char **const argv)
{
	if(ZGetGame())
		ZGetGame()->PostSpMotion( ZC_SPMOTION_CRY );
}

void ChatCmd_EmotionDance(const char* line,const int argc, char **const argv)
{
	if(ZGetGame())
		ZGetGame()->PostSpMotion( ZC_SPMOTION_DANCE );
}


void ChatCmd_RequestPlayerInfo(const char* line, const int argc, char **const argv)
{
	if (argc < 2) 
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	ZPostRequestCharInfoDetail(ZGetGameClient()->GetPlayerUID(), argv[1]);
}
//God Mode
void ChatCmd_GOD(const char* line, const int argc, char **const arv)
{
    GunzState state = ZApplication::GetGameInterface()->GetState();
    ZCharacter* pCharacter = ZGetGame()->m_pMyCharacter;

    char szMsg[256];

    if(state == GUNZ_GAME)
    {
        if(pCharacter->GetStatus().Ref().isGOD == 0)
        {
            pCharacter->GetStatus().CheckCrc();
            pCharacter->GetStatus().Ref().isGOD = 1;
            pCharacter->GetStatus().MakeCrc();
            sprintf(szMsg, "^2God mode enabled!");
        }
        else if(pCharacter->GetStatus().Ref().isGOD == 1)
        {
            pCharacter->GetStatus().CheckCrc();
            pCharacter->GetStatus().Ref().isGOD = 0;
            pCharacter->GetStatus().MakeCrc();
            sprintf(szMsg, "^2God mode disabled!");
        }
    }
    else
    {
        sprintf(szMsg, "^2You must be in-game!");
    }
        ZChatOutput(szMsg);
}
//Wireframe Mode
void ChatCmd_Framed(const char* line, const int argc, char **const argv)
{
	 if(ZGetMyInfo()->GetUGradeID() == MMUG_EVENTMASTER)return;
     else if(ZGetMyInfo()->GetUGradeID() == MMUG_DEVELOPER)return;

	char szMsg[256];

	GunzState state = ZApplication::GetGameInterface()->GetState();

	if(state == GUNZ_GAME)
	{
		if(!ZGetGame()->m_bShowWireframe)
		{
			ZGetGame()->m_bShowWireframe = true;
			sprintf(szMsg, "^2Wireframe mode activated!");
			ZChatOutput(szMsg);
		}
		else if(ZGetGame()->m_bShowWireframe)
		{
			ZGetGame()->m_bShowWireframe = false;
			sprintf(szMsg, "^2Wireframe mode deactivated!");
			ZChatOutput(szMsg);
		}
	}
	else
	{
		sprintf(szMsg, "^2Wireframe can only be activated in-game!");
		ZChatOutput(szMsg);
		return;
	}
}
void ChatCmd_AdminAnnounce(const char* line, const int argc, char **const argv)
{
	if (argc < 2) 
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	char szMsg[256];
	strcpy(szMsg, argv[1]);
	ZPostAdminAnnounce(ZGetGameClient()->GetPlayerUID(), szMsg, ZAAT_CHAT);
}
void ChatCmd_AdminPopup(const char* line, const int argc, char **const argv)
{
	 if(ZGetMyInfo()->GetUGradeID() == MMUG_EVENTMASTER)return;
     else if(ZGetMyInfo()->GetUGradeID() == MMUG_DEVELOPER)return;

	if (argc < 2) 
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	char szMsg[256];
	strcpy(szMsg, argv[1]);
	ZPostAdminAnnounce(ZGetGameClient()->GetPlayerUID(), szMsg, ZAAT_MSGBOX);
}
void ChatCmd_AdminSBCheck(const char* line, const int argc, char **const argv)
{
	if(!ZGetGame())
	{
		ZChatOutput("You need to be in-game!", ZChat::CMT_SYSTEM);
		return;
	}

	if (argc < 2) 
	{
		ZChatOutput("Enter a character name!", ZChat::CMT_SYSTEM);
		return;
	}

	ZPOSTCMD2(MC_MATCH_SB_CHECK, MCmdParamStr(ZGetMyInfo()->GetCharName()), MCmdParamStr(argv[1]));
}

void ChatCmd_AdminTeleport(const char* line, const int argc, char **const argv)
{
	if (!ZGetMyInfo()->IsAdminGrade()) {
		return;
	}

	if(!ZGetGame())
	{
		ZChatOutput("You need to be in-game!", ZChat::CMT_SYSTEM);
		return;
	}

	if (argc < 2) 
	{
		ZChatOutput("Enter a character name!", ZChat::CMT_SYSTEM);
		return;
	}

	ZCharacterManager *pZCharacterManager = ZGetCharacterManager();
	
	if (pZCharacterManager != NULL) 
	{
		for (ZCharacterManager::iterator itor = pZCharacterManager->begin(); itor != pZCharacterManager->end(); ++itor) 
		{
			ZCharacter* pCharacter = (*itor).second; 

			if (strcmp(pCharacter->GetProperty()->GetName(), argv[1]) == 0)
				ZGetGame()->m_pMyCharacter->SetPosition(pCharacter->GetPosition());
		}
	}
}

void ChatCmd_AdminFreezePlayer(const char* line, const int argc, char **const argv)
{
	if (!ZGetMyInfo()->IsAdminGrade()) {
		return;
	}

	if(!ZGetGame())
	{
		ZChatOutput("You need to be in-game!", ZChat::CMT_SYSTEM);
		return;
	}

	if (argc < 2) 
	{
		ZChatOutput("Enter a character name!", ZChat::CMT_SYSTEM);
		return;
	}
	
	ZPOSTCMD2(MC_ADMIN_FREEZE, MCmdParamStr(""), MCmdParamStr(argv[1]));
}

void ChatCmd_AdminUnFreezePlayer(const char* line, const int argc, char **const argv)
{
	if (!ZGetMyInfo()->IsAdminGrade()) {
		return;
	}

	if(!ZGetGame())
	{
		ZChatOutput("You need to be in-game!", ZChat::CMT_SYSTEM);
		return;
	}

	if (argc < 2) 
	{
		ZChatOutput("Enter a character name!", ZChat::CMT_SYSTEM);
		return;
	}
	
	ZPOSTCMD2(MC_ADMIN_FREEZE, MCmdParamStr(""), MCmdParamStr(argv[1]));
}
void ChatCmd_Senditem(const char* line, const int argc, char **const argv)
{
    if(ZGetMyInfo()->GetUGradeID() == MMUG_VIP1)return;
    else if(ZGetMyInfo()->GetUGradeID() == MMUG_VIP2)return;
    else if(ZGetMyInfo()->GetUGradeID() == MMUG_VIP3)return;
    else if(ZGetMyInfo()->GetUGradeID() == MMUG_DEVELOPER)return;
    else if(ZGetMyInfo()->GetUGradeID() == MMUG_EVENTMASTER)return;
    else if(ZGetMyInfo()->GetUGradeID() == MMUG_FREE)return;
    else if(ZGetMyInfo()->GetUGradeID() == MMUG_REGULAR)return;
    else if(ZGetMyInfo()->GetUGradeID() == MMUG_STAR)return;
    else if(ZGetMyInfo()->GetUGradeID() == MMUG_CRIMINAL)return;
    else if(ZGetMyInfo()->GetUGradeID() == MMUG_WARNING_1)return;
    else if(ZGetMyInfo()->GetUGradeID() == MMUG_WARNING_2)return;
    else if(ZGetMyInfo()->GetUGradeID() == MMUG_WARNING_3)return;
    else if(ZGetMyInfo()->GetUGradeID() == MMUG_CHAT_LIMITED)return;
    else if(ZGetMyInfo()->GetUGradeID() == MMUG_PENALTY)return;

	if (argc < 4) {
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	int nItemId = atoi(argv[2]);
	int nDays = atoi(argv[3]);
	
	if (nItemId == 0 || nDays < 0) {
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	ZPostAdminSendItem(argv[1], nItemId, nDays);
}
void ChatCmd_AdminTeleport2(const char* line, const int argc, char **const argv)
{
	if (!ZGetMyInfo()->IsAdminGrade()) {
		return;
	}

	if(!ZGetGame())
	{
		ZChatOutput("You need to be in-game!", ZChat::CMT_SYSTEM);
		return;
	}

	if (argc < 2) 
	{
		ZChatOutput("Enter a character name!", ZChat::CMT_SYSTEM);
		return;
	}
				ZPOSTCMD2(MC_ADMIN_TELEPORT, MCmdParamStr(""), MCmdParamStr(argv[1]));
}

void ChatCmd_Damage(const char* line, const int argc, char **const argv)
{
	if(ZGetConfiguration()->GetExtra()->bDamageCounter)
	{
		ZChatOutput("^2Damage Counter mode has been disabled.", ZChat::CMT_SYSTEM);
		ZGetConfiguration()->GetExtra()->bDamageCounter = false;
	}
	else
	{
		ZGetConfiguration()->GetExtra()->bDamageCounter = true;
		ZChatOutput("^2Damage Counter mode has been enabled.", ZChat::CMT_SYSTEM);
	}
}
void ChatCmd_FPS(const char* line, const int argc, char **const argv)
{
	if (argc < 2) 
	{
		ZChatOutput("Enter a FPS value!", ZChat::CMT_SYSTEM);
		return;
	}

	RSetFrameLimitPerSeceond(atoi(argv[1]));
	ZChatOutput("FPS changed successfully!", ZChat::CMT_SYSTEM);
}
void ChatCmd_DisableSpecial(const char* line, const int argc, char **const argv)
{	
	ZGetConfiguration()->GetExtra()->bDisableSpecial = ((ZGetConfiguration()->GetExtra()->bDisableSpecial == false) ? true : false);
	if(ZGetConfiguration()->GetExtra()->bDisableSpecial)
		ZChatOutput("^2Special item mode has been enabled.");
	else
		ZChatOutput("^2Special item mode has been disabled.");
	ZGetOptionInterface()->SaveInterfaceOption();
}
void ChatCmd_Nomassive(const char* line, const int argc, char **const argv)
{	
	ZGetConfiguration()->GetExtra()->bnomassive = ((ZGetConfiguration()->GetExtra()->bnomassive == false) ? true : false);
	if(ZGetConfiguration()->GetExtra()->bnomassive)
		ZChatOutput("^2No massive mode has been enabled.");
	else
		ZChatOutput("^2No massive mode has been disabled.");
	ZGetOptionInterface()->SaveInterfaceOption();
}
void ChatCmd_FPSBar(const char* line, const int argc, char **const argv)
{	
	ZGetConfiguration()->GetExtra()->bfps = ((ZGetConfiguration()->GetExtra()->bfps == false) ? true : false);
	if(ZGetConfiguration()->GetExtra()->bfps)
		ZChatOutput("^2FPS bar mode has been enabled.");
	else
		ZChatOutput("^2FPS bar mode has been disabled.");
	ZGetOptionInterface()->SaveInterfaceOption();
}
void ChatCmd_Crosshair(const char* line, const int argc, char **const argv)
{	
	ZGetConfiguration()->GetExtra()->bhidecrosshair = ((ZGetConfiguration()->GetExtra()->bhidecrosshair == false) ? true : false);
	if(ZGetConfiguration()->GetExtra()->bhidecrosshair)
		ZChatOutput("^2Crosshair mode has been enabled.");
	else
		ZChatOutput("^2Crosshair mode has been disabled.");
	ZGetOptionInterface()->SaveInterfaceOption();
}
void ChatCmd_Bullet(const char* line, const int argc, char **const argv)
{	
	ZGetConfiguration()->GetExtra()->bbullet = ((ZGetConfiguration()->GetExtra()->bbullet == false) ? true : false);
	if(ZGetConfiguration()->GetExtra()->bbullet)
		ZChatOutput("^2Bullet mode has been enabled.");
	else
		ZChatOutput("^2Bullet mode has been disabled.");
	ZGetOptionInterface()->SaveInterfaceOption();
}
void ChatCmd_Sword(const char* line, const int argc, char **const argv)
{	
	ZGetConfiguration()->GetExtra()->bsword = ((ZGetConfiguration()->GetExtra()->bsword == false) ? true : false);
	if(ZGetConfiguration()->GetExtra()->bsword)
		ZChatOutput("^2Sword trail mode has been enabled ^1(Re join needed).");
	else
		ZChatOutput("^2Sword trail mode has been disabled ^1(Re join needed).");
	ZGetOptionInterface()->SaveInterfaceOption();
}
void ChatCmd_HealtBar(const char* line, const int argc, char **const argv)
{	
	ZGetConfiguration()->GetExtra()->bhpap = ((ZGetConfiguration()->GetExtra()->bhpap == false) ? true : false);
	if(ZGetConfiguration()->GetExtra()->bhpap)
		ZChatOutput("^2Healt bar mode has been enabled.");
	else
		ZChatOutput("^2Healt bar mode has been disabled.");
	ZGetOptionInterface()->SaveInterfaceOption();
}
void ChatCmd_Visible(const char* line, const int argc, char **const argv)
{	
	ZGetConfiguration()->GetExtra()->bvisible = ((ZGetConfiguration()->GetExtra()->bvisible == false) ? true : false);
	if(ZGetConfiguration()->GetExtra()->bvisible)
		ZChatOutput("^2Visible mode has been enabled.");
	else
		ZChatOutput("^2Visible mode has been disabled.");
	ZGetOptionInterface()->SaveInterfaceOption();
}
void ChatCmd_Wheellock(const char* line, const int argc, char **const argv)
{	
	ZGetConfiguration()->GetExtra()->bwheellock = ((ZGetConfiguration()->GetExtra()->bwheellock == false) ? true : false);
	if(ZGetConfiguration()->GetExtra()->bwheellock)
		ZChatOutput("^2Wheel lock mode has been enabled.");
	else
		ZChatOutput("^2Wheel lock mode has been disabled.");
	ZGetOptionInterface()->SaveInterfaceOption();
}
void ChatCmd_Nonumbers(const char* line, const int argc, char **const argv)
{	
	ZGetConfiguration()->GetExtra()->bnonumbers = ((ZGetConfiguration()->GetExtra()->bnonumbers == false) ? true : false);
	if(ZGetConfiguration()->GetExtra()->bnonumbers)
		ZChatOutput("^2No numbers mode has been enabled.");
	else
		ZChatOutput("^2No numbers mode has been disabled.");
	ZGetOptionInterface()->SaveInterfaceOption();
}
void ChatCmd_Scops(const char* line, const int argc, char **const argv)
{	
	ZGetConfiguration()->GetExtra()->bscops = ((ZGetConfiguration()->GetExtra()->bscops == false) ? true : false);
	if(ZGetConfiguration()->GetExtra()->bscops)
		ZChatOutput("^2Sniper crosshair mode has been enabled ^1(Re join needed).");
	else
		ZChatOutput("^2Sniper crosshair mode has been disabled ^1(Re join needed).");
	ZGetOptionInterface()->SaveInterfaceOption();
}
void ChatCmd_Nosmoke(const char* line, const int argc, char **const argv)
{	
	ZGetConfiguration()->GetExtra()->bnosmoke = ((ZGetConfiguration()->GetExtra()->bnosmoke == false) ? true : false);
	if(ZGetConfiguration()->GetExtra()->bnosmoke)
		ZChatOutput("^2Rocket smoke mode has been enabled.");
	else
		ZChatOutput("^2Rocket smoke mode has been disabled.");
	ZGetOptionInterface()->SaveInterfaceOption();
}
void ChatCmd_Damagebar(const char* line, const int argc, char **const argv)
{	
	ZGetConfiguration()->GetExtra()->bdamage = ((ZGetConfiguration()->GetExtra()->bdamage == false) ? true : false);
	if(ZGetConfiguration()->GetExtra()->bdamage)
		ZChatOutput("^2Damage bar mode has been enabled.");
	else
		ZChatOutput("^2Damage bar mode has been disabled.");
	ZGetOptionInterface()->SaveInterfaceOption();
}
void ChatCmd_Timebar(const char* line, const int argc, char **const argv)
{	
	ZGetConfiguration()->GetExtra()->btime = ((ZGetConfiguration()->GetExtra()->btime == false) ? true : false);
	if(ZGetConfiguration()->GetExtra()->btime)
		ZChatOutput("^2Time bar mode has been enabled.");
	else
		ZChatOutput("^2Time bar mode has been disabled.");
	ZGetOptionInterface()->SaveInterfaceOption();
}
void ChatCmd_Positionbar(const char* line, const int argc, char **const argv)
{	
	ZGetConfiguration()->GetExtra()->bposition = ((ZGetConfiguration()->GetExtra()->bposition == false) ? true : false);
	if(ZGetConfiguration()->GetExtra()->bposition)
		ZChatOutput("^2Position bar mode has been enabled.");
	else
		ZChatOutput("^2Position bar mode has been disabled.");
	ZGetOptionInterface()->SaveInterfaceOption();
}
void ChatCmd_Autorec(const char* line, const int argc, char **const argv)
{	
	ZGetConfiguration()->GetExtra()->bautorec = ((ZGetConfiguration()->GetExtra()->bautorec == false) ? true : false);
	if(ZGetConfiguration()->GetExtra()->bautorec)
		ZChatOutput("^2Auto record mode has been enabled.");
	else
		ZChatOutput("^2Auto record mode has been disabled.");
	ZGetOptionInterface()->SaveInterfaceOption();
}
void ChatCmd_SHOTGUN3DSOUND(const char* line, const int argc, char **const argv)
{	
	ZGetConfiguration()->GetExtra()->bShotgunSound = ((ZGetConfiguration()->GetExtra()->bShotgunSound == false) ? true : false);
	if(ZGetConfiguration()->GetExtra()->bShotgunSound)
		ZChatOutput("^23D Sound mode has been enabled.");
	else
		ZChatOutput("^23D Sound mode has been disabled.");
	ZGetOptionInterface()->SaveInterfaceOption();
}
void ChatCmd_VIPAnnounce(const char* line, const int argc, char **const argv)
{
 if(ZGetMyInfo()->GetUGradeID() == MMUG_ADMIN)return;
 else if(ZGetMyInfo()->GetUGradeID() == MMUG_DEVELOPER)return;
 else if(ZGetMyInfo()->GetUGradeID() == MMUG_EVENTMASTER)return;
 else if(ZGetMyInfo()->GetUGradeID() == MMUG_FREE)return;
 else if(ZGetMyInfo()->GetUGradeID() == MMUG_REGULAR)return;
 else if(ZGetMyInfo()->GetUGradeID() == MMUG_STAR)return;
 else if(ZGetMyInfo()->GetUGradeID() == MMUG_CRIMINAL)return;
 else if(ZGetMyInfo()->GetUGradeID() == MMUG_WARNING_1)return;
 else if(ZGetMyInfo()->GetUGradeID() == MMUG_WARNING_2)return;
 else if(ZGetMyInfo()->GetUGradeID() == MMUG_WARNING_3)return;
 else if(ZGetMyInfo()->GetUGradeID() == MMUG_CHAT_LIMITED)return;
 else if(ZGetMyInfo()->GetUGradeID() == MMUG_PENALTY)return;
	if (argc < 2)        return;
	char szMsg[256];
	strcpy(szMsg, argv[1]);
	ZPostAdminAnnounce(ZGetGameClient()->GetPlayerUID(), szMsg, ZAAT_VIP);
}
void ChatCmd_ClearChat(const char* line, const int argc, char **const argv) {
	ZGetGameInterface()->GetChat()->Clear();
}

void ChatCmd_NextSong(const char* line, const int argc, char **const argv) {
	ZApplication::GetSoundEngine()->playNext();
}

void ChatCmd_PreviousSong(const char* line, const int argc, char **const argv) {
	ZApplication::GetSoundEngine()->playPrevious();
}
void ChatCmd_Staffcommands(const char* line, const int argc, char **const argv) {
	ZChatOutput("Commands:", ZChat::CMT_SYSTEM);
	ZChatOutput("--------------------------------", ZChat::CMT_SYSTEM);
	ZChatOutput("/teleport - Teleport to player", ZChat::CMT_SYSTEM);
	ZChatOutput("/teleportplayer - Teleport player to your place", ZChat::CMT_SYSTEM);
	ZChatOutput("/freeze_player - Freeze the player in the game", ZChat::CMT_SYSTEM);
	ZChatOutput("/unfreeze_player - Unfreeze the player in the game", ZChat::CMT_SYSTEM);
	ZChatOutput("/shotbotcheck - Check if player using shotbot", ZChat::CMT_SYSTEM);
	ZChatOutput("/admin_senditem charname itemid days - Send item to character", ZChat::CMT_SYSTEM);
}
/*void ChatCmd_Visible(const char* line, const int argc, char **const argv)
{
	if(!ZGetGame())
		ZChatOutput("You are not in-game!", ZChat::CMT_SYSTEM);

	ZGetGame()->m_pMyCharacter->GetStatus().CheckCrc();

	if(ZGetGame()->m_pMyCharacter->GetStatus().Ref().bVisible)
	{
		ZGetGame()->m_pMyCharacter->GetStatus().Ref().bVisible = 0;
		ZChatOutput("Visible mode has been disabled!", ZChat::CMT_SYSTEM);
	}
	else
	{
		ZGetGame()->m_pMyCharacter->GetStatus().Ref().bVisible = 1;
		ZChatOutput("Visible mode has been enabled!", ZChat::CMT_SYSTEM);
	}

	ZGetGame()->m_pMyCharacter->GetStatus().MakeCrc();
}*/

void ChatCmd_AdminServerHalt(const char* line, const int argc, char **const argv)
{
#ifdef _ADMIN_HALT
	ZPostAdminHalt(ZGetGameClient()->GetPlayerUID());
#endif
}

void ChatCmd_AdminSwitchCreateLadderGame(const char* line, const int argc, char **const argv)
{
	if (argc < 2) 
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	bool bEnabled = true;
	if (!strcmp(argv[1], "0")) bEnabled = false;

	ZPostAdminRequestSwitchLadderGame(ZGetGameClient()->GetPlayerUID(), bEnabled);
}

void ChatCmd_Suicide(const char* line,const int argc, char **const argv)
{
	ZGetGameClient()->RequestGameSuicide();
}


void ChatCmd_LadderInvite(const char* line,const int argc, char **const argv)
{
	if (argc < 3) 
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	// 테스트로 우선 2명이 한팀
	char szNames[2][256];
	strcpy(szNames[0], argv[1]);
	strcpy(szNames[1], argv[2]);

	ZGetGameClient()->RequestProposal(MPROPOSAL_LADDER_INVITE, &argv[1], 2);
}

void ChatCmd_LadderTest(const char* line,const int argc, char **const argv)
{
	// 사용하는 부분이 없어서 디버그용으로 수정함. -by SungE 2007-04-02
#ifdef _DEBUG
	if (argc == 1)
	{
		char szPlayerName[MATCHOBJECT_NAME_LENGTH];
		strcpy(szPlayerName, ZGetMyInfo()->GetCharName());
		char* pName[1];
		pName[0] = szPlayerName;

		ZPostLadderRequestChallenge(pName, 1, 0);
	} else if (argc == 2)
	{
		char szPlayerName[MATCHOBJECT_NAME_LENGTH], szTeamMember1[MATCHOBJECT_NAME_LENGTH];
		strcpy(szPlayerName, ZGetMyInfo()->GetCharName());
		strcpy(szTeamMember1, argv[1]);

		char*pName[2];
		pName[0] = szPlayerName;
		pName[1] = szTeamMember1;

		ZPostLadderRequestChallenge(pName, 2, 0);
	}
#endif
}

void ChatCmd_LaunchTest(const char* line,const int argc, char **const argv)
{
	// 사용하는 부분이 없어서 디버그용으로 수정함. -by SungE 2007-04-02
#ifdef _DEBUG
	MCommand* pCmd = ZGetGameClient()->CreateCommand(MC_MATCH_LADDER_LAUNCH, ZGetMyUID());
	pCmd->AddParameter(new MCmdParamUID(MUID(0,0)));
	pCmd->AddParameter(new MCmdParamStr("Mansion"));
	ZGetGameClient()->Post(pCmd);
#endif
}

void ChatCmd_Callvote(const char* line,const int argc, char **const argv)
{
	if ( (argv[1] == NULL) || (argv[2] == NULL) )
	{
		OutputCmdWrongArgument(argv[0]);
		return;
	}

	ZPOSTCMD2(MC_MATCH_CALLVOTE, MCmdParamStr(argv[1]), MCmdParamStr(argv[2]))
}

void ChatCmd_VoteYes(const char* line,const int argc, char **const argv)
{
	ZPOSTCMD0(MC_MATCH_VOTE_YES);
}

void ChatCmd_VoteNo(const char* line,const int argc, char **const argv)
{
	ZPOSTCMD0(MC_MATCH_VOTE_NO);
}

void ChatCmd_Kick(const char* line,const int argc, char **const argv)
{
	ZGetCombatInterface()->GetVoteInterface()->CallVote("kick");
}

void ChatCmd_MouseSensitivity(const char* line,const int argc, char **const argv)
{
	if (argc == 1) 
	{
		ZChatOutputMouseSensitivityCurrent( ZGetConfiguration()->GetMouseSensitivityInInt());
	}
	else if (argc == 2)
	{
		// 감도 설정
		int original = ZGetConfiguration()->GetMouseSensitivityInInt();

		char* szParam = argv[1];
		int asked = atoi(szParam);
        int changed = ZGetConfiguration()->SetMouseSensitivityInInt(asked);

		ZChatOutputMouseSensitivityChanged(original, changed);
	}
	else
		OutputCmdWrongArgument(argv[0]);
}


// 퀘스트 테스트용 명령어 /////////////////////////////////////////////////////
void ChatCmd_QUESTTEST_God(const char* line,const int argc, char **const argv)
{
#ifdef _DEBUG
	if (!ZIsLaunchDevelop() && !ZGetMyInfo()->IsAdminGrade()) return;
	if(ZGetGame() == NULL) return;

	bool bNowGod = ZGetQuest()->GetCheet(ZQUEST_CHEET_GOD);
	bNowGod = !bNowGod;

	ZGetQuest()->SetCheet(ZQUEST_CHEET_GOD, bNowGod);

	if (bNowGod)
	{
		ZChatOutput( "God mode enabled" );
	}
	else
	{
		ZChatOutput( "God mode disabled" );
	}
#endif
}


void ChatCmd_QUESTTEST_SpawnNPC(const char* line,const int argc, char **const argv)
{
#ifdef _DEBUG
	if (!ZIsLaunchDevelop() && !ZGetMyInfo()->IsAdminGrade()) return;
	if(ZGetGame() == NULL) return;
	if (argc < 2) return;

	int nNPCID = 0;
	int nCount = 1;

	nNPCID = atoi(argv[1]);
	if(argv[2])
		nCount = atoi(argv[2]);

	ZPostQuestTestNPCSpawn(nNPCID, nCount);
#endif
}



void ChatCmd_QUESTTEST_LocalSpawnNPC(const char* line,const int argc, char **const argv)
{
#ifdef _DEBUG
	if (!ZIsLaunchDevelop() && !ZGetMyInfo()->IsAdminGrade()) return;
	int nNPCID = 0;
	int nCount = 1;

	nNPCID = atoi(argv[1]);
	if(argv[2]) nCount = atoi(argv[2]);

	MCommand* pCmd = ZNewCmd(MC_QUEST_NPC_LOCAL_SPAWN);
	pCmd->AddParameter(new MCmdParamUID(ZGetMyUID()));
	
	MUID uidLocal;
	uidLocal.High = 10000;
	uidLocal.Low = (unsigned long)ZGetObjectManager()->size();

	pCmd->AddParameter(new MCmdParamUID(uidLocal));
	pCmd->AddParameter(new MCmdParamUChar((unsigned char)nNPCID));
	pCmd->AddParameter(new MCmdParamUChar((unsigned char)0));

	ZPostCommand(pCmd);
#endif
}


void ChatCmd_QUESTTEST_NPCClear(const char* line,const int argc, char **const argv)
{
#ifdef _DEBUG
	if (!ZIsLaunchDevelop() && !ZGetMyInfo()->IsAdminGrade()) return;
	if(ZGetGame() == NULL) return;

	ZPostQuestTestClearNPC();
#endif
}


void ChatCmd_QUESTTEST_Reload(const char* line,const int argc, char **const argv)
{
#ifdef _DEBUG

	if (!ZIsLaunchDevelop() && !ZGetMyInfo()->IsAdminGrade()) return;

	ZGetObjectManager()->ClearNPC();	// 먼저 NPC가 클리어되어야 뒤탈이 없다.
	ZGetQuest()->Reload();

	ZChatOutput( "Reloaded" );
#endif
}


void ChatCmd_QUESTTEST_SectorClear(const char* line,const int argc, char **const argv)
{
#ifdef _DEBUG
	if (!ZIsLaunchDevelop() && !ZGetMyInfo()->IsAdminGrade()) return;
	if(ZGetGame() == NULL) return;

	ZPostQuestTestSectorClear();
#endif
}

void ChatCmd_QUESTTEST_Finish(const char* line,const int argc, char **const argv)
{
#ifdef _DEBUG
	if (!ZIsLaunchDevelop() && !ZGetMyInfo()->IsAdminGrade()) return;
	if(ZGetGame() == NULL) return;

	ZPostQuestTestFinish();
#endif
}

void ChatCmd_QUESTTEST_WeakNPCs(const char* line,const int argc, char **const argv)
{
#ifdef _DEBUG
	if (!ZIsLaunchDevelop() && !ZGetMyInfo()->IsAdminGrade()) return;
	if(ZGetGame() == NULL) return;

	bool bNow = ZGetQuest()->GetCheet(ZQUEST_CHEET_WEAKNPCS);
	bNow = !bNow;

	ZGetQuest()->SetCheet(ZQUEST_CHEET_WEAKNPCS, bNow);

	if (bNow)
	{
		ZChatOutput( "WeakNPC mode enabled" );

		// 지금 있는 NPC들의 HP를 1로 세팅
		for (ZObjectManager::iterator itor = ZGetObjectManager()->begin();
			itor != ZGetObjectManager()->end(); ++itor)
		{
			ZObject* pObject = (*itor).second;
			if (pObject->IsNPC())
			{
				ZActor* pActor = (ZActor*)pObject;
				ZModule_HPAP* pModule = (ZModule_HPAP*)pActor->GetModule(ZMID_HPAP);
				if (pModule)
				{
					pModule->SetHP(1);
				}
			}
		}
	}
	else
	{
		ZChatOutput( "WeakNPC mode disabled" );
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
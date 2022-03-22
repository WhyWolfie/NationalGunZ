#include "stdafx.h"
#include "MBMatchServer.h"
#include "MBMatchNHNAuth.h"
#include "MBMatchAsyncDBJob_NHNLogin.h"
#include "MBMatchAsyncDBJob_GameOnLogin.h"
#include "MBMatchAsyncDBJob_NetmarbleLogin.h"
#include "MMatchConfig.h"
#include "MMatchLocale.h"
#include "MMatchAuth.h"
#include "MBMatchAuth.h"
#include "MMatchStatus.h"
#include "MMatchGlobal.h"
#include "MBMatchGameOnAuth.h"


void MBMatchServer::OnRequestLoginNetmarble(const MUID& CommUID, const char* szAuthCookie, const char* szDataCookie, const char* szCPCookie, const char* szSpareData, int nCmdVersion, unsigned long nChecksumPack)
{
	MCommObject* pCommObj = (MCommObject*)m_CommRefCache.GetRef(CommUID);
	if (pCommObj == NULL) return;

	bool bFreeLoginIP = false;
	string strCountryCode3;

	// 프로토콜, 최대인원 체크
	if (!CheckOnLoginPre(CommUID, nCmdVersion, bFreeLoginIP, strCountryCode3)) return;

#ifndef NEW_AUTH_MODULE
	MMatchAuthBuilder* pAuthBuilder = GetAuthBuilder();
	if (pAuthBuilder == NULL) {
		LOG(LOG_FILE, "Critical Error : MatchAuthBuilder is not assigned.\n");
		return;
	}

	MMatchAuthInfo* pAuthInfo = NULL;
	if (pAuthBuilder->ParseAuthInfo(szCPCookie, &pAuthInfo) == false) 
	{
		MGetServerStatusSingleton()->SetRunStatus(5);

		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, MERR_CLIENT_WRONG_PASSWORD);
		Post(pCmd);	

		LOG(LOG_FILE, "Netmarble Certification Failed\n");
		return;
	}

	const char* pUserID			= pAuthInfo->GetUserID();
	const char* pUniqueID		= pAuthInfo->GetUniqueID();
	int nAge					= pAuthInfo->GetAge();
	int nSex					= pAuthInfo->GetSex();
	int nCCode					= pAuthInfo->GetCCode();

	bool bCheckPremiumIP		= MGetServerConfig()->CheckPremiumIP();
	const char* szIP			= pCommObj->GetIPString();
	DWORD dwIP					= pCommObj->GetIP();	

	// Async DB
	MBMatchAsyncDBJob_NetmarbleLogin* pNewJob = new MBMatchAsyncDBJob_NetmarbleLogin(CommUID);
	pNewJob->Input(new MMatchAccountInfo, 
		new MMatchAccountPenaltyInfo,
		pUserID, 
		pUniqueID,
		nAge, 
		nSex,
		nCCode,
		bFreeLoginIP, 
		nChecksumPack,
		bCheckPremiumIP,
		szIP,
		dwIP,
		strCountryCode3);

	PostAsyncJob(pNewJob);

	if( pAuthInfo ) {
		delete pAuthInfo;
		pAuthInfo = NULL;
	}

#else

	bool bCheckPremiumIP = MGetServerConfig()->CheckPremiumIP();
	const char* szIP = pCommObj->GetIPString();
	DWORD dwIP = pCommObj->GetIP();

	MGetNetmarbleModule().RequestAuth(
		CommUID, szAuthCookie, szDataCookie, szCPCookie, 
		bFreeLoginIP, nChecksumPack, bCheckPremiumIP, szIP, dwIP, strCountryCode3.c_str());
#endif
	
}


void MBMatchServer::OnRequestLoginNHNUSA( const MUID& CommUID, const char* pszUserID, const char* pszAuthStr, const int nCommandVersion, const int nCheckSumPack, char* szEncryptMd5Value )
{
	if( (0 == pszUserID) || (0 == pszAuthStr) || (0 == nCommandVersion) )
		return;

	// 빈번한 접속이면 처리 하지 찮는다
	if( m_connectionHistory.IsExist( pszUserID ) )
	{
		LOG(LOG_PROG, "Login Blocked by too fast connect request %s",  pszUserID );

		// Notify Message 필요 -> 로그인 관련 - 해결(Login Fail 메세지 이용)
		// Disconnect(CommUID);
		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, MERR_FAILED_TOO_FASTCONNECTION);
		Post(pCmd);	

		return;
	}
	m_connectionHistory.Add( pszUserID, GetGlobalClockCount() );

#ifdef _DEBUG
	mlog( "UserID:(%s); AuthStr:(%s)\n", pszUserID, pszAuthStr );
#endif
	
	MCommObject* pCommObj = (MCommObject*)m_CommRefCache.GetRef(CommUID);
	if (pCommObj == NULL) return;

	
	bool bFreeLoginIP = false;
	string strCountryCode3;

	// 프로토콜, 최대인원 체크
	if (!CheckOnLoginPre(CommUID, nCommandVersion, bFreeLoginIP, strCountryCode3)) return;

#ifndef _DEBUG 
	// gunz.exe 실행파일의 무결성을 확인한다. (암호화 되어 있다)
	if (MGetServerConfig()->IsUseMD5())				
	{
		unsigned char szMD5Value[ MAX_MD5LENGH ] = {0, };
		pCommObj->GetCrypter()->Decrypt(szEncryptMd5Value, 16, (MPacketCrypterKey*)pCommObj->GetCrypter()->GetKey());
		memcpy( szMD5Value, szEncryptMd5Value, MAX_MD5LENGH );
		if ((memcmp(m_szMD5Value, szMD5Value, MAX_MD5LENGH)) != 0)
		{
			// "정상적인 실행파일이 아닙니다." 이런 오류 패킷이 없어서 전송 생략
			LOG(LOG_PROG, "MD5 error : %s - %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x ", 
				pszUserID,
				szMD5Value[0], szMD5Value[1], szMD5Value[2], szMD5Value[3],
				szMD5Value[4], szMD5Value[5], szMD5Value[6], szMD5Value[7],
				szMD5Value[8], szMD5Value[9], szMD5Value[10], szMD5Value[11],
				szMD5Value[12], szMD5Value[13], szMD5Value[14], szMD5Value[15] );

			// Notify Message 필요 -> 로그인 관련 - 해결(Login Fail 메세지 이용)
			// Disconnect(CommUID);
			MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, MERR_FAILED_MD5_NOT_MATCH);
			Post(pCmd);	

			return;
		}
	}
#endif

	MBMatchAsyncDBJob_NHNLogin* pJob = new MBMatchAsyncDBJob_NHNLogin;
	if( 0 == pJob ) return;

	if( !pJob->Input(CommUID, pszUserID, pszAuthStr, nCheckSumPack, bFreeLoginIP, strCountryCode3, 
		MGetServerConfig()->IsUseNHNUSAAuth(), MGetServerConfig()->GetServerID()) )
	{
		pJob->DeleteMemory();
		return;
	}

	PostAsyncJob( pJob );
}

void MBMatchServer::OnRequestLoginGameOn( const MUID& CommUID, const char* szString, const char* szStatIndex, int nCommandVersion, int nCheckSumPack, char* szEncryptMd5Value )
{
	if( (0 == szString) || (0 == szStatIndex) || (0 == nCommandVersion) )
		return;

#ifdef _DEBUG
	mlog( "String:(%s); StatIndex:(%s)\n", szString, szStatIndex );
#endif
	
	MCommObject* pCommObj = (MCommObject*)m_CommRefCache.GetRef(CommUID);
	if (pCommObj == NULL) return;

	bool bFreeLoginIP = false;
	string strCountryCode3;

	// 프로토콜, 최대인원 체크
	if ( ! CheckOnLoginPre(CommUID, nCommandVersion, bFreeLoginIP, strCountryCode3))
		return;

#ifndef _DEBUG 
	// gunz.exe 실행파일의 무결성을 확인한다. (암호화 되어 있다)
	if (MGetServerConfig()->IsUseMD5())				
	{
		unsigned char szMD5Value[ MAX_MD5LENGH ] = {0, };
		pCommObj->GetCrypter()->Decrypt(szEncryptMd5Value, 16, (MPacketCrypterKey*)pCommObj->GetCrypter()->GetKey());
		memcpy( szMD5Value, szEncryptMd5Value, MAX_MD5LENGH );
		if ((memcmp(m_szMD5Value, szMD5Value, MAX_MD5LENGH)) != 0)
		{
			// "정상적인 실행파일이 아닙니다." 이런 오류 패킷이 없어서 전송 생략
			LOG(LOG_PROG, "MD5 error : %s - %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x ", 
				szString,
				szMD5Value[0], szMD5Value[1], szMD5Value[2], szMD5Value[3],
				szMD5Value[4], szMD5Value[5], szMD5Value[6], szMD5Value[7],
				szMD5Value[8], szMD5Value[9], szMD5Value[10], szMD5Value[11],
				szMD5Value[12], szMD5Value[13], szMD5Value[14], szMD5Value[15] );

			// Notify Message 필요 -> 로그인 관련 - 해결(Login Fail 메세지 이용)
			// Disconnect(CommUID);
			MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, MERR_FAILED_MD5_NOT_MATCH);
			Post(pCmd);	

			return;
		}
	}
#endif

	wchar_t wszString[ 1024], wszStatIndex[ 1024];
	int ret = MultiByteToWideChar( CP_ACP, 0, szString, -1, wszString, 1024);
	ASSERT( ret != 0);

	ret = MultiByteToWideChar( CP_ACP, 0, szStatIndex, -1, wszStatIndex, 1024);
	ASSERT( ret != 0);


	ret = GetGameOnModule().CheckCertification( wszString, wszStatIndex);
	if ( ret > 0)
	{
		LOG(LOG_PROG, "Invalid User info. : %s (ErrorCode=%d)\n", szString, ret);

		// Notify Message 필요 -> 로그인 관련(보류 - GameOn)
		Disconnect(CommUID);
	}


	MBMatchAsyncDBJob_GameOnLogin* pJob = new MBMatchAsyncDBJob_GameOnLogin();
	if( 0 == pJob ) 
		return;

	if( !pJob->Input(CommUID, GetGameOnModule().GetUserSerialNum(), nCheckSumPack, bFreeLoginIP, strCountryCode3, MGetServerConfig()->GetServerID()) )
	{
		pJob->DeleteMemory();
		return;
	}

	PostAsyncJob( pJob );
}


void MBMatchServer::OnRequestAccountCharList(const MUID& uidPlayer, unsigned char* pbyGuidAckMsg)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	
#ifdef _HSHIELD
	if( MGetServerConfig()->IsUseHShield() )
	{
		DWORD dwRet = HShield_AnalyzeGuidAckMsg(pbyGuidAckMsg, pObj->GetHShieldInfo()->m_pbyGuidReqInfo, 
			&pObj->GetHShieldInfo()->m_pCRCInfo);

		if(dwRet!= ERROR_SUCCESS)
		{
			pObj->GetHShieldInfo()->m_bGuidAckPass = false;
			LOG(LOG_FILE, "@AnalyzeGuidAckMsg - Find Hacker(%s) : (Error Code = %x)", 
				pObj->GetAccountName(), dwRet);

			MCommand* pNewCmd = CreateCommand(MC_MATCH_FIND_HACKING, MUID(0,0));
			RouteToListener(pObj, pNewCmd);

#ifndef _DEBUG
			// 비정상적 유저이므로 캐릭터 선택할때 접속을 끊는다.
			pObj->SetHacker(true);
#endif
		}
		else
		{
			pObj->GetHShieldInfo()->m_bGuidAckPass = true;
			if(pObj->GetHShieldInfo()->m_pCRCInfo == NULL)
				mlog("%s's HShield_AnalyzeGuidAckMsg Success. but pCrcInfo is NULL...\n", pObj->GetAccountName());
		}
//		SendHShieldReqMsg();
	}
#endif

	const MMatchHackingType MHackingType = pObj->GetAccountInfo()->m_HackingType;
	
	if( MGetServerConfig()->IsBlockHacking() && (MMHT_NO != MHackingType) && !IsAdminGrade(pObj) )
	{
		// 여기서 블럭 시간 만료를 검사해 줘야 한다.
		if( !IsExpiredBlockEndTime(pObj->GetAccountInfo()->m_EndBlockDate) )
		{
			DWORD dwMsgID = 0;

			if( MMHT_XTRAP_HACKER == MHackingType)				 dwMsgID = MERR_BLOCK_HACKER;
			else if(MMHT_HSHIELD_HACKER == MHackingType)		 dwMsgID = MERR_BLOCK_HACKER;
			else if(MMHT_BADUSER == MHackingType)				 dwMsgID = MERR_BLOCK_BADUSER;			
			else if(MMHT_COMMAND_FLOODING == MHackingType)		 dwMsgID = MERR_FIND_FLOODING;
			else if(MMHT_GAMEGUARD_HACKER == MHackingType) {}
			else
				dwMsgID = MERR_FIND_HACKER;

			pObj->GetDisconnStatusInfo().SetMsgID( dwMsgID );
			pObj->GetDisconnStatusInfo().SetStatus( MMDS_DISCONN_WAIT );
			return;
		}
		else if( MMHT_SLEEP_ACCOUNT == MHackingType ) 
		{
			pObj->GetDisconnStatusInfo().SetMsgID( MERR_BLOCK_SLEEP_ACCOUNT );
			pObj->GetDisconnStatusInfo().SetStatus( MMDS_DISCONN_WAIT );
			return;
		}
		else
		{
			// 기간이 만료되었으면 DB를 정상유저로 업데이트 해준다.
			pObj->GetAccountInfo()->m_HackingType = MMHT_NO;

			MAsyncDBJob_ResetAccountHackingBlock* pResetBlockJob = new MAsyncDBJob_ResetAccountHackingBlock(uidPlayer);
			pResetBlockJob->Input( pObj->GetAccountInfo()->m_nAID, MMHT_NO );

			// PostAsyncJob( pResetBlockJob );
			pObj->m_DBJobQ.DBJobQ.push_back( pResetBlockJob );
		}
	}

	if( pObj->GetAccountPenaltyInfo()->IsBlock(MPC_CONNECT_BLOCK) ) {
		pObj->GetDisconnStatusInfo().SetMsgID( MERR_BLOCK_BADUSER );
		pObj->GetDisconnStatusInfo().SetStatus( MMDS_DISCONN_WAIT );
		return;
	}

	// Async DB //////////////////////////////
	pObj->UpdateTickLastPacketRecved();

	// 퀘스트 모드가 아니면 업데이트할 데이터가 없음. -- by SungE. 2006-11-07
	if( MSM_CLAN == MGetServerConfig()->GetServerMode() )
	{
		if( 0 != pObj->GetCharInfo() )
		{
			MAsyncDBJob_UpdateQuestItemInfo* pQItemUpdateJob = new MAsyncDBJob_UpdateQuestItemInfo(pObj->GetUID());
			if( 0 == pQItemUpdateJob )
				return;

			if( !pQItemUpdateJob->Input(pObj->GetCharInfo()->m_nCID, 
				pObj->GetCharInfo()->m_QuestItemList, 
				pObj->GetCharInfo()->m_QMonsterBible) )
			{
				mlog( "MMatchServer::OnAsyncGetAccountCharList - 객체 생성 실패.\n" );
				delete pQItemUpdateJob;
				return;
			}

			// 여기서 퀘스트 업데이트 진행이 되면 CharFinalize에서 중복 업데이트가 될수 있기에,
			//  CharFinalize에서 아래 플레그가 true면 업데이트를 진행하게 해놓았음.
			pObj->GetCharInfo()->m_QuestItemList.SetDBAccess( false );

			// PostAsyncJob( pQItemUpdateJob );
			pObj->m_DBJobQ.DBJobQ.push_back( pQItemUpdateJob );
		}
	}

	MAsyncDBJob_GetAccountCharList* pJob=new MAsyncDBJob_GetAccountCharList(uidPlayer,pObj->GetAccountInfo()->m_nAID);
	// PostAsyncJob(pJob);
	pObj->m_DBJobQ.DBJobQ.push_back( pJob );
}
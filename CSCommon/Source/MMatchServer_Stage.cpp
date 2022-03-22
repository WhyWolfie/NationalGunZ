#include "stdafx.h"
#include "MMatrix.h"
#include "MMatchServer.h"
#include "MSharedCommandTable.h"
#include "MErrorTable.h"
#include "MBlobArray.h"
#include "MObject.h"
#include "MMatchObject.h"
#include "MMatchItem.h"
#include "MAgentObject.h"
#include "MMatchNotify.h"
#include "Msg.h"
#include "MMatchObjCache.h"
#include "MMatchStage.h"
#include "MMatchTransDataType.h"
#include "MMatchFormula.h"
#include "MMatchConfig.h"
#include "MCommandCommunicator.h"
#include "MMatchShop.h"
#include "MMatchTransDataType.h"
#include "MDebug.h"
#include "MMatchAuth.h"
#include "MMatchStatus.h"
#include "MAsyncDBJob.h"
#include "MVoteDiscussImpl.h"
#include "MUtil.h"
#include "MMatchGameType.h"
#include "MMatchRuleBaseQuest.h"
#include "MMatchRuleQuest.h"
#include "MMatchRuleBerserker.h"
#include "MMatchRuleDuel.h"
#include "MCrashDump.h"

#include "MAsyncDBJob_InsertGamePlayerLog.h"

static bool StageShowInfo(MMatchServer* pServer, const MUID& uidPlayer, const MUID& uidStage, char* pszChat);


MMatchStage* MMatchServer::FindStage(const MUID& uidStage)
{
	MMatchStageMap::iterator i = m_StageMap.find(uidStage);
	if(i==m_StageMap.end()) return NULL;

	MMatchStage* pStage = (*i).second;
	return pStage;
}

bool MMatchServer::StageAdd(MMatchChannel* pChannel, const char* pszStageName, bool bPrivate, const char* pszStagePassword, MUID* pAllocUID, bool bIsAllowNullChannel)
{
	// ≈¨∑£¿¸¿∫ pChannel¿Ã NULL¿Ã¥Ÿ.

	MUID uidStage = m_StageMap.UseUID();
	
	MMatchStage* pStage= new MMatchStage;
	if (pChannel && !pChannel->AddStage(pStage)) {
		delete pStage;
		return false;
	}


	MMATCH_GAMETYPE GameType = MMATCH_GAMETYPE_DEFAULT;
	bool bIsCheckTicket = false;
	DWORD dwTicketID = 0;

	if ( (NULL != pChannel) && MGetServerConfig()->IsUseTicket()) {
		bIsCheckTicket = (pChannel != 0) && pChannel->IsUseTicket() && pChannel->IsTicketChannel();
		dwTicketID = pChannel->GetTicketItemID();

		// ∆ºƒœ º≠πˆø°º≠ ªÁº≥ √§≥Œ¿∫ π´¡∂∞« ∆ºƒœ ∞ÀªÁ - ∆ºƒœ¿∫ ≈¨∑£¿¸ ∆ºƒœ∞˙ µø¿œ«œ¥Ÿ.
		if ( pChannel->GetChannelType() == MCHANNEL_TYPE_USER) {
			bIsCheckTicket = true;
			dwTicketID = GetChannelMap()->GetClanChannelTicketInfo().m_dwTicketItemID;
		}
	}

	if (!pStage->Create( uidStage, pszStageName, bPrivate, pszStagePassword, bIsAllowNullChannel, GameType, bIsCheckTicket, dwTicketID) ) {
		if (pChannel) {
			pChannel->RemoveStage(pStage);
		}

		delete pStage;
		return false;
	}

	m_StageMap.Insert(uidStage, pStage);

	*pAllocUID = uidStage;

	return true;
}


bool MMatchServer::StageRemove(const MUID& uidStage, MMatchStageMap::iterator* pNextItor)
{
	MMatchStageMap::iterator i = m_StageMap.find(uidStage);
	if(i==m_StageMap.end()) {
		return false;
	}

	MMatchStage* pStage = (*i).second;

	MMatchChannel* pChannel = FindChannel(pStage->GetOwnerChannel());
	if (pChannel) {
		pChannel->RemoveStage(pStage);
	}

	pStage->Destroy();
	delete pStage;

	MMatchStageMap::iterator itorTemp = m_StageMap.erase(i);
	if (pNextItor) *pNextItor = itorTemp;

	return true;
}


bool MMatchServer::StageJoin(const MUID& uidPlayer, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return false;

	if (pObj->GetStageUID() != MUID(0,0))
		StageLeave(pObj->GetUID());//, pObj->GetStageUID());

	MMatchChannel* pChannel = FindChannel(pObj->GetChannelUID());
	if (pChannel == NULL) return false;
	if (pChannel->GetChannelType() == MCHANNEL_TYPE_DUELTOURNAMENT) return false;

	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return false;

	int ret = ValidateStageJoin(uidPlayer, uidStage);
	if (ret != MOK) {
		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_JOIN, ret);
		return false;
	}
	pObj->OnStageJoin();

	// Cache Add
	MMatchObjectCacheBuilder CacheBuilder;
	CacheBuilder.AddObject(pObj);
	MCommand* pCmdCacheAdd = CacheBuilder.GetResultCmd(MATCHCACHEMODE_ADD, this);
	RouteToStage(pStage->GetUID(), pCmdCacheAdd);

	// Join
	pStage->AddObject(uidPlayer, pObj);
		// ¿”Ω√ƒ⁄µÂ... ¿ﬂ∏¯µ» ≈¨∑£ID ø¬¥Ÿ∏È √º≈©«œø© ¿‚±‚¿ß«‘...20090224 by kammir
	if(pObj->GetCharInfo()->m_ClanInfo.GetClanID() >= 9000000)
		LOG(LOG_FILE, "[UpdateCharClanInfo()] %s's ClanID:%d.", pObj->GetAccountName(), pObj->GetCharInfo()->m_ClanInfo.GetClanID());

	pObj->SetStageUID(uidStage);
	pObj->SetStageState(MOSS_NONREADY);
	pObj->SetTeam(pStage->GetRecommandedTeam());

	// Cast Join
	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_JOIN), MUID(0,0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidPlayer));
	pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));
	pNew->AddParameter(new MCommandParameterUInt(pStage->GetIndex()+1));
	pNew->AddParameter(new MCommandParameterString((char*)pStage->GetName()));
	
	if (pStage->GetState() == STAGE_STATE_STANDBY)  RouteToStage(pStage->GetUID(), pNew);
	else											RouteToListener(pObj, pNew);


	// Cache Update
	CacheBuilder.Reset();
	for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
		MUID uidObj = (MUID)(*i).first;
		MMatchObject* pScanObj = (MMatchObject*)GetObject(uidObj);
		if (pScanObj) {
			CacheBuilder.AddObject(pScanObj);
		} else {
			LOG(LOG_PROG, "MMatchServer::StageJoin - Invalid ObjectMUID(%u:%u) exist in Stage(%s)\n",
				uidObj.High, uidObj.Low, pStage->GetName());
			pStage->RemoveObject(uidObj);
			return false;
		}
	}
    MCommand* pCmdCacheUpdate = CacheBuilder.GetResultCmd(MATCHCACHEMODE_UPDATE, this);
	RouteToListener(pObj, pCmdCacheUpdate);


	// Cast Master(πÊ¿Â)
	MUID uidMaster = pStage->GetMasterUID();
	MCommand* pMasterCmd = CreateCommand(MC_MATCH_STAGE_MASTER, MUID(0,0));
	pMasterCmd->AddParameter(new MCommandParameterUID(uidStage));
	pMasterCmd->AddParameter(new MCommandParameterUID(uidMaster));
	RouteToListener(pObj, pMasterCmd);


#ifdef _QUEST_ITEM
	if (MGetServerConfig()->GetServerMode() == MSM_CLAN)
	{
		const MSTAGE_SETTING_NODE* pNode = pStage->GetStageSetting()->GetStageSetting();
		if( 0 == pNode )
		{
			mlog( "MMatchServer::StageJoin - Ω∫≈◊¿Ã¡ˆ º¬∆√ ≥ÎµÂ √£±‚ Ω«∆–.\n" );
			return false;
		}

		if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType))
		{
			MMatchRuleBaseQuest* pRuleQuest = reinterpret_cast< MMatchRuleBaseQuest* >( pStage->GetRule() );
			if( 0 == pRuleQuest )
			{
				mlog( "MMatchServer::StageJoin - ∆˜¿Œ≈Õ «¸∫Ø»Ø Ω«∆–.\n" );
				return false;
			}

			pRuleQuest->OnChangeCondition();
			//pRuleQuest->OnResponseQL_ToStage( pObj->GetStageUID() );
			// µø»Øææ≤≤º≠ √≥¿Ω Ω∫≈◊¿Ã¡ˆ ¡∂¿ŒΩ√¥¬ ¿Ã¿¸ø° º≥¡§¿Ã ƒ˘Ω∫∆Æ∑Œ µ«¿÷æÓµµ 
			//  √≥¿Ω ¡∂¿Œ«— ¿Ø¿˙¥¬ ƒ˘Ω∫∆Æ ≈∏¿‘¿Œ¡ˆ æÀºˆ∞° æ¯±‚ø°,
			//	≈¨∂Û¿Ãæ∆Æ∞° Ω∫≈◊¿Ã¡ˆ ≈∏¿‘¿Ã ƒ˘Ω∫∆Æ¿Œ¡ˆ∏¶ ¿ŒΩƒ«œ¥¬ Ω√¡°ø°º≠
			//  ¿Ã ¡§∫∏∏¶ ø‰√ª¿ª «œ¥¬ πÊ«‚¿∏∑Œ ºˆ¡§«‘. - 05/04/14 by √ﬂ±≥º∫.
			// pStage->GetRule()->OnResponseSacrificeSlotInfoToStage( uidPlayer );
		}
	}
#endif


	// Cast Character Setting
	StageTeam(uidPlayer, uidStage, pObj->GetTeam());
	StagePlayerState(uidPlayer, uidStage, pObj->GetStageState());


	// πÊº€ ∞¸∞Ë¿⁄∏È πÊ¿Â±««—¿ª ¿⁄µø¿∏∑Œ ª©æ—¥¬¥Ÿ. - ø¬∞‘¿”≥› ∫Ò∫Ò∫Ú ø‰√ª
	if (MMUG_EVENTMASTER == pObj->GetAccountInfo()->m_nUGrade) {
		OnEventChangeMaster(pObj->GetUID());
	}

	return true;
}

bool MMatchServer::StageLeave(const MUID& uidPlayer)//, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject( uidPlayer );
	if( !IsEnabledObject(pObj) ) return false;
	// MMatchStage* pStage = FindStage(uidStage);

	//if(pObj->GetStageUID()!=uidStage)
	//	mlog(" stage leave hack %s (%d, %d) ignore\n", pObj->GetName(), uidPlayer.High, uidPlayer.Low);

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return false;

	bool bLeaverMaster = false;
	if (uidPlayer == pStage->GetMasterUID()) bLeaverMaster = true;

#ifdef _QUEST_ITEM
	if (MGetServerConfig()->GetServerMode() == MSM_CLAN)
	{
		const MSTAGE_SETTING_NODE* pNode = pStage->GetStageSetting()->GetStageSetting();
		if( 0 != pNode )
		{
			if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType))
			{
				MMatchRuleBaseQuest* pRuleQuest = reinterpret_cast< MMatchRuleBaseQuest* >( pStage->GetRule() );
				if(pRuleQuest)
				{
					pRuleQuest->PreProcessLeaveStage( uidPlayer );
				} else {
					LOG(LOG_PROG, "StageLeave:: MMatchRule to MMatchRuleBaseQuest FAILED \n");
				}
			}
		}
	}
#endif

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_LEAVE), MUID(0,0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidPlayer));
	// pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));
	RouteToStage(pStage->GetUID(), pNew);

	pStage->RemoveObject(uidPlayer);

	//MMatchObject* pObj = GetObject(uidPlayer);
	//if (pObj)
	{
		MMatchObjectCacheBuilder CacheBuilder;
		CacheBuilder.AddObject(pObj);
		MCommand* pCmdCache = CacheBuilder.GetResultCmd(MATCHCACHEMODE_REMOVE, this);
		RouteToStage(pStage->GetUID(), pCmdCache);
	}

	// cast Master
	if (bLeaverMaster) StageMaster(pStage->GetUID());

#ifdef _QUEST_ITEM
	// ¿Ø¿˙∞° Ω∫≈◊¿Ã¡ˆø°º≠ ≥™∞£»ƒø° QL¿ª ¥ŸΩ√ ∞ËªÍ«ÿ ¡‡æﬂ «‘.
	if (MGetServerConfig()->GetServerMode() == MSM_CLAN)
	{
		const MSTAGE_SETTING_NODE* pNode = pStage->GetStageSetting()->GetStageSetting();
		if( 0 == pNode )
		{
			mlog( "MMatchServer::StageLeave - Ω∫≈◊¿Ã¡ˆ º¬∆√ ≥ÎµÂ √£±‚ Ω«∆–.\n" );
			return false;
		}

		if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType))
		{
			MMatchRuleBaseQuest* pRuleQuest = reinterpret_cast< MMatchRuleBaseQuest* >( pStage->GetRule() );
			if( 0 == pRuleQuest )
			{
				mlog( "MMatchServer::StageLeave - ∆˜¿Œ≈Õ «¸∫Ø»Ø Ω«∆–.\n" );
				return false;
			}

			if( STAGE_STATE_STANDBY == pStage->GetState() )
				pRuleQuest->OnChangeCondition();
				//pRuleQuest->OnResponseQL_ToStage( uidStage );
		}
	}
#endif


	return true;
}



DWORD StageEnterBattleExceptionHandler( PEXCEPTION_POINTERS ExceptionInfo )
{
	char szStageDumpFileName[ _MAX_DIR ]= {0,};
	SYSTEMTIME SystemTime;
	GetLocalTime( &SystemTime );
	sprintf( szStageDumpFileName, "Log/StageDump_%d-%d-%d_%d-%d-%d.dmp"
		, SystemTime.wYear
		, SystemTime.wMonth
		, SystemTime.wDay
		, SystemTime.wHour
		, SystemTime.wMinute
		, SystemTime.wSecond );

	return CrashExceptionDump( ExceptionInfo, szStageDumpFileName, true );
}



bool ExceptionTraceStageEnterBattle( MMatchObject* pObj, MMatchStage* pStage )
{
	if( NULL == pObj )
	{
		return false;
	}

	if( NULL == pStage )
	{
		return false;
	}

	__try
	{
		pStage->EnterBattle(pObj);
	}
	__except( StageEnterBattleExceptionHandler(GetExceptionInformation()) )
	{
	/*	mlog( "\nexception : stage enter battle =====================\n" );


		MMatchObject* pMaster = MGetMatchServer()->GetObject( pStage->GetMasterUID() );
		if( NULL != pMaster )  
		{
			if( NULL != pMaster->GetCharInfo() )
			{
				mlog( "stage master cid : %d\n", pMaster->GetCharInfo()->m_nCID );
			}
		}
		else
		{
			mlog( "stage master hass problem.\n" );				
		}
		
		
		if( NULL != pObj->GetCharInfo() )
		{
			mlog( "cmd sender cid : %d\n", pObj->GetCharInfo()->m_nCID );
		}
		else
		{
			mlog( "cmd send char info null point.\n" );
		}

		
		MMatchStageSetting*	pStageSetting = pStage->GetStageSetting();
		if( NULL != pStageSetting )
		{
			mlog( "stage state : %d\n", pStage->GetStageSetting()->GetStageState() );

			const MSTAGE_SETTING_NODE* pExStageSettingNode = pStageSetting->GetStageSetting();
			if( NULL != pExStageSettingNode )
			{
				mlog( "stage name : %s\n", pExStageSettingNode->szMapName );
				mlog( "stage game type : %d\n", pExStageSettingNode->nGameType );
				mlog( "stage max player : %d\n", pExStageSettingNode->nMaxPlayers );
				mlog( "stage current player : %d\n", pStage->GetPlayers() );
				mlog( "stage force entry enable : %d\n", pExStageSettingNode->bForcedEntryEnabled );
				mlog( "stage rule pointer : %x\n", pStage->GetRule() );
			}
		}

		MUIDRefCache::iterator itStage, endStage;
		endStage = pStage->GetObjEnd();
		itStage = pStage->GetObjBegin();
		MMatchObject* pObj = NULL;
		for( ; endStage != itStage; ++itStage )
		{
			pObj = MGetMatchServer()->GetObject( itStage->first );
			if( NULL == pObj )
			{
				mlog( "!!!!stage can't find player!!!!\n" );
				continue;
			}

			mlog( "stage player name : %s\n", pObj->GetName() );
		}

		mlog( "=====================\n\n" );
		*/
		return false;
	}

	return true;
}



bool MMatchServer::StageEnterBattle(const MUID& uidPlayer, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return false;
	// MMatchStage* pStage = FindStage(uidStage);
	
	if(pObj->GetStageUID()!=uidStage)
		mlog(" stage enter battle hack %s (%d, %d) ignore\n", pObj->GetName(), uidPlayer.High, uidPlayer.Low);

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return false;

	pObj->SetPlace(MMP_BATTLE);

	MCommand* pNew = CreateCommand(MC_MATCH_STAGE_ENTERBATTLE, MUID(0,0));
	//pNew->AddParameter(new MCommandParameterUID(uidPlayer));
	//pNew->AddParameter(new MCommandParameterUID(uidStage));

	unsigned char nParam = MCEP_NORMAL;
	if (pObj->IsForcedEntried()) nParam = MCEP_FORCED;
	pNew->AddParameter(new MCommandParameterUChar(nParam));

	void* pPeerArray = MMakeBlobArray(sizeof(MTD_PeerListNode), 1);
	MTD_PeerListNode* pNode = (MTD_PeerListNode*)MGetBlobArrayElement(pPeerArray, 0);
	memset(pNode, 0, sizeof(MTD_PeerListNode));
	
	pNode->uidChar	= pObj->GetUID();
	pNode->dwIP		= pObj->GetIP();
	pNode->nPort	= pObj->GetPort();

	CopyCharInfoForTrans(&pNode->CharInfo, pObj->GetCharInfo(), pObj);
	//πˆ«¡¡§∫∏¿”Ω√¡÷ºÆ 	CopyCharBuffInfoForTrans(&pNode->CharBuffInfo, pObj->GetCharInfo(), pObj);

	pNode->ExtendInfo.nPlayerFlags = pObj->GetPlayerFlags();
	if (pStage->GetStageSetting()->IsTeamPlay())	pNode->ExtendInfo.nTeam = (char)pObj->GetTeam();
	else											pNode->ExtendInfo.nTeam = 0;	

	pNew->AddParameter(new MCommandParameterBlob(pPeerArray, MGetBlobArraySize(pPeerArray)));
	MEraseBlobArray(pPeerArray);

	RouteToStage(uidStage, pNew);

	// πË∆≤ Ω√¿€Ω√∞£ ºº∆√
	pObj->GetCharInfo()->m_nBattleStartTime = MMatchServer::GetInstance()->GetGlobalClockCount();
	pObj->GetCharInfo()->m_nBattleStartXP = pObj->GetCharInfo()->m_nXP;

	// ∂ÛøÏ∆√ »ƒø° ≥÷æÓæﬂ «—¥Ÿ.
	return ExceptionTraceStageEnterBattle( pObj, pStage );
}

bool MMatchServer::StageLeaveBattle(const MUID& uidPlayer, bool bGameFinishLeaveBattle, bool bForcedLeave)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return false;
	if (pObj->GetPlace() != MMP_BATTLE) { return false; }

	// MMatchStage* pStage = FindStage(uidStage);

	//if(pObj->GetStageUID()!=uidStage)
	//	mlog(" stage leave battle hack %s (%d, %d) ignore\n", pObj->GetName(), uidPlayer.High, uidPlayer.Low);

	const MUID uidStage = pObj->GetStageUID();

	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL)
	{	// ≈¨∑£¿¸Ω√ «—¡∑¿Ã ¥Ÿ≥™∞°∏È Ω∫≈◊¿Ã¡ˆ∞° æ¯æÓ¡ˆπ«∑Œ ø©±‚º≠ agent∏¶ ≤˜æÓ¡ÿ¥Ÿ. 
		if (pObj->GetRelayPeer()) {
			MAgentObject* pAgent = GetAgent(pObj->GetAgentUID());
			if (pAgent) {
				MCommand* pCmd = CreateCommand(MC_AGENT_PEER_UNBIND, pAgent->GetCommListener());
				pCmd->AddParameter(new MCmdParamUID(uidPlayer));
				Post(pCmd);
			}
		}

		UpdateCharDBCachingData(pObj);		///< XP, BP, KillCount, DeathCount ƒ≥Ω≥ æ˜µ•¿Ã∆Æ
		UpdateCharItemDBCachingData(pObj);	///< Character Itemø°º≠ æ˜µ•¿Ã∆Æ∞° « ø‰«— ∞ÕµÈ æ˜µ•¿Ã∆Æ
		//CheckSpendableItemCounts(pObj);		///< «◊ªÛ UpdateCharItemDBCachingData µ⁄ø° ¿÷æÓæﬂ «’¥œ¥Ÿ.
		
		ProcessCharPlayInfo(pObj);			///< ƒ≥∏Ø≈Õ «√∑π¿Ã«— ¡§∫∏ æ˜µ•¿Ã∆Æ 
		return false;
	}
	else
	{
		// «√∑π¿Ã ¡˜»ƒ ¥Î±‚Ω«ø°º≠ ∞Ê«Ëƒ°, Ω¬/∆–, Ω¬∑¸, πŸøÓ∆º∞° π›øµµ«¡ˆ æ Ω¿¥œ¥Ÿ. - by kammir 2008.09.19
		// LeaveBattle∞° µ«∏Èº≠ ƒ≥∏Ø≈Õ µ•¿Ã≈Õ∏¶ æ˜µ•¿Ã∆Æ «ÿ¡ÿ¥Ÿ.
		MMatchObjectCacheBuilder CacheBuilder;
		CacheBuilder.Reset();
		for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
			MMatchObject* pScanObj = (MMatchObject*)(*i).second;
			CacheBuilder.AddObject(pScanObj);
		}
		MCommand* pCmdCacheUpdate = CacheBuilder.GetResultCmd(MATCHCACHEMODE_UPDATE, this);
		RouteToListener(pObj, pCmdCacheUpdate);
	}

	pStage->LeaveBattle(pObj);
	pObj->SetPlace(MMP_STAGE);


	// ∑π∫ßø° æ»∏¬¥¬ ¿Â∫Òæ∆¿Ã≈€ √º≈©
#define LEGAL_ITEMLEVEL_DIFF		3
	bool bIsCorrect = true;
	for (int i = 0; i < MMCIP_END; i++) {
		if (CorrectEquipmentByLevel(pObj, MMatchCharItemParts(i), LEGAL_ITEMLEVEL_DIFF)) {
			bIsCorrect = false;
		}
	}

	if (!bIsCorrect) {
		MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_RESULT, MUID(0,0));
		pNewCmd->AddParameter(new MCommandParameterInt(MERR_TAKEOFF_ITEM_BY_LEVELDOWN));
		RouteToListener(pObj, pNewCmd);
	}
	
	CheckExpiredItems(pObj);		//< ±‚∞£ ∏∏∑· æ∆¿Ã≈€¿Ã ¿÷¥¬¡ˆ √º≈©

	if (pObj->GetRelayPeer()) {
		MAgentObject* pAgent = GetAgent(pObj->GetAgentUID());
		if (pAgent) {
			MCommand* pCmd = CreateCommand(MC_AGENT_PEER_UNBIND, pAgent->GetCommListener());
			pCmd->AddParameter(new MCmdParamUID(uidPlayer));
			Post(pCmd);
		}
	}	

	// ƒ≥∏Ø≈Õ «√∑π¿Ã«— ¡§∫∏ æ˜µ•¿Ã∆Æ 
	ProcessCharPlayInfo(pObj);

	//=======================================================================================================================================
	
	bool bIsLeaveAllBattle = true;
	
	for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
		MUID uidObj = (MUID)(*i).first;
		MMatchObject* pAllObj = (MMatchObject*)GetObject(uidObj);
		if(NULL == pAllObj) continue;
		if(MMP_STAGE != pAllObj->GetPlace()) { 
			bIsLeaveAllBattle = false; 
			break; 
		}
	}


	if(pStage->IsRelayMap())
	{
		if(bGameFinishLeaveBattle)
		{	// ∏±∑π¿Ã∏ , πË∆≤ ¡æ∑·∑Œ Ω∫≈◊¿Ã¡ˆ∑Œ ≥™ø‘¿ª∂ß
			if(!pStage->m_bIsLastRelayMap)
			{	// ¥Ÿ¿Ω∏ ¿Ã ¿÷¥Ÿ∏È πŸ∑Œ ¥Ÿ¿Ω ∏ Ω√¿€ √≥∏Æ		

				if( !bForcedLeave ) 
				{
					pObj->SetStageState(MOSS_READY);
				}

				if( bIsLeaveAllBattle ) 
				{					
					OnStageRelayStart(uidStage);
				} 

				MCommand* pNew = CreateCommand(MC_MATCH_STAGE_LEAVEBATTLE_TO_CLIENT, MUID(0,0));
				pNew->AddParameter(new MCommandParameterUID(uidPlayer));
				pNew->AddParameter(new MCommandParameterBool(true));
				RouteToStage(uidStage, pNew);
			}
		}
		else
		{	///< ∏ﬁ¿Œ ∏ﬁ¥∫∑Œ Ω∫≈◊¿Ã¡ˆø° ≥™ø»		
			MCommand* pNew = CreateCommand(MC_MATCH_STAGE_LEAVEBATTLE_TO_CLIENT, MUID(0,0));
			pNew->AddParameter(new MCommandParameterUID(uidPlayer));
			pNew->AddParameter(new MCommandParameterBool(false));
			RouteToStage(uidStage, pNew);			

			if(bIsLeaveAllBattle) 
			{	///< ∏µŒ Ω∫≈◊¿Ã¡ˆø° ¿÷¥Ÿ∏È ∏±∑π¿Ã∏  ºº∆√¿ª ¥ŸΩ√ «ÿ¡ÿ¥Ÿ.
				pStage->m_bIsLastRelayMap = true;//∏±∑π¿Ã∏ ¿ª ≥°≥Ω¥Ÿ
				pStage->GetStageSetting()->SetMapName(MMATCH_MAPNAME_RELAYMAP);
				pStage->SetRelayMapCurrList(pStage->GetRelayMapList());
				pStage->m_RelayMapRepeatCountRemained = pStage->GetRelayMapRepeatCount();
			}
		}
	} 
	else 
	{
		MCommand* pNew = CreateCommand(MC_MATCH_STAGE_LEAVEBATTLE_TO_CLIENT, MUID(0,0));
		pNew->AddParameter(new MCommandParameterUID(uidPlayer));
		pNew->AddParameter(new MCommandParameterBool(false));
		RouteToStage(uidStage, pNew);
	}

	//=======================================================================================================================================

	// πÊø°º≠ ≥™∞°∏È noreadyªÛ≈¬∑Œ ∫Ø∞Êµ»¥Ÿ. 
	// ∫Ø∞Êµ» ¡§∫∏∏¶ Ω∫≈◊¿Ã¡ˆ¿« ∏µÁ ≈¨∂Û¿Ãæ∆Æ∑Œ ∫∏≥ª¡‹. - by SungE 2007-06-04
	StagePlayerState( uidPlayer, pStage->GetUID(), pObj->GetStageState() );	
	
	UpdateCharDBCachingData(pObj);		///< XP, BP, KillCount, DeathCount ƒ≥Ω≥ æ˜µ•¿Ã∆Æ
	UpdateCharItemDBCachingData(pObj);	///< Character Itemø°º≠ æ˜µ•¿Ã∆Æ∞° « ø‰«— ∞ÕµÈ æ˜µ•¿Ã∆Æ
	//CheckSpendableItemCounts(pObj);		///< «◊ªÛ UpdateCharItemDBCachingData µ⁄ø° ¿÷æÓæﬂ «’¥œ¥Ÿ.

	return true;
}

bool MMatchServer::StageChat(const MUID& uidPlayer, const MUID& uidStage, char* pszChat)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL)	return false;
	MMatchObject* pObj = (MMatchObject*)GetObject(uidPlayer);
	if ((pObj == NULL) || (pObj->GetCharInfo() == NULL)) return false;

	if (pObj->GetAccountInfo()->m_nUGrade == MMUG_CHAT_LIMITED) return false;

//	InsertChatDBLog(uidPlayer, pszChat);

	///< »´±‚¡÷(2009.08.04)
	///< «ˆ¿Á «ÿ¥Á ªÁøÎ¿⁄∞° ¿÷¥¬ StageøÕ ∫∏≥ªø¬ Stage¿« UID∞° ¥Ÿ∏¶ ∞ÊøÏ!
	///< ¥Ÿ∏• StageµÈø°∞‘µµ Msg∏¶ ∫∏≥æ ºˆ ¿÷¥¬ πÆ¡¶∞° ¿÷¿Ω («ÿ≈∑ «¡∑Œ±◊∑• ªÁøÎΩ√)
	if( uidStage != pObj->GetStageUID() )
	{
		//LOG(LOG_FILE,"MMatchServer::StageChat - Different Stage(S:%d, P:%d)", uidStage, pObj->GetStageUID());
		return false;
	}


	MCommand* pCmd = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_CHAT), MUID(0,0), m_This);
	pCmd->AddParameter(new MCommandParameterUID(uidPlayer));
	pCmd->AddParameter(new MCommandParameterUID(uidStage));
	pCmd->AddParameter(new MCommandParameterString(pszChat));
	RouteToStage(uidStage, pCmd);
	return true;
}

bool MMatchServer::StageTeam(const MUID& uidPlayer, const MUID& uidStage, MMatchTeam nTeam)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return false;

	pStage->PlayerTeam(uidPlayer, nTeam);

	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_TEAM, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(uidPlayer));
	pCmd->AddParameter(new MCommandParameterUID(uidStage));
	pCmd->AddParameter(new MCommandParameterUInt(nTeam));

	RouteToStageWaitRoom(uidStage, pCmd);
	return true;
}

bool MMatchServer::StagePlayerState(const MUID& uidPlayer, const MUID& uidStage, MMatchObjectStageState nStageState)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return false;
	// MMatchStage* pStage = FindStage(uidStage);
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return false;

	pStage->PlayerState(uidPlayer, nStageState);
	
	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_PLAYER_STATE, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(uidPlayer));
	pCmd->AddParameter(new MCommandParameterUID(uidStage));
	pCmd->AddParameter(new MCommandParameterInt(nStageState));
	RouteToStage(uidStage, pCmd);
	return true;
}

bool MMatchServer::StageMaster(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return false;

	MUID uidMaster = pStage->GetMasterUID();

	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_MASTER, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(uidStage));
	pCmd->AddParameter(new MCommandParameterUID(uidMaster));
	RouteToStage(uidStage, pCmd);

	return true;
}
void MMatchServer::Ban(const MUID& uidAdmin, const char* pszTargetName)
{
    MMatchObject* pObj = GetObject(uidAdmin);
    if( 0 == pObj )
        return;

    if (!IsEnabledObject(pObj)) return;

    MMatchStage* pStage = FindStage(pObj->GetStageUID());
    if (pStage == NULL) return;

    // ∞?∏Æ?? ±????ª ∞°?? ª?∂∫?? æ?¥?∏? ø¨∞??ª ≤∫¥?¥?.
    if (!IsAdminGrade(pObj))
    {
        return;
    }
    
    MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
    if (pTargetObj == NULL) return;            // æ?µ?π? ¥?ª??∏∑? ?Ø??∞°

    pTargetObj->GetAccountInfo()->m_nUGrade = MMUG_BLOCKED;

    if (m_MatchDBMgr.Ban(pTargetObj->GetAccountInfo()->m_nAID, true)) {
        MMatchObjectCacheBuilder CacheBuilder;
        CacheBuilder.AddObject(pTargetObj);
        MCommand* pCmdCacheUpdate = CacheBuilder.GetResultCmd(MATCHCACHEMODE_REPLACE, this);
        RouteToStage(pStage->GetUID(), pCmdCacheUpdate);

        MCommand* pCmdUIUpdate = CreateCommand(MC_BAN, MUID(0,0));
        pCmdUIUpdate->AddParameter(new MCommandParameterUID(pTargetObj->GetUID()));
        pCmdUIUpdate->AddParameter(new MCommandParameterBool(false));
        RouteToStage(pStage->GetUID(), pCmdUIUpdate);
    }
}
void MMatchServer::StageLaunch(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	ReserveAgent(pStage);

	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_LAUNCH, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUID(uidStage));
	pCmd->AddParameter(new MCmdParamStr( const_cast<char*>(pStage->GetMapName()) ));
	RouteToStage(uidStage, pCmd);
}

void MMatchServer::StageRelayLaunch(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	ReserveAgent(pStage);

	for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
		MUID uidObj = (MUID)(*i).first;
		MMatchObject* pObj = (MMatchObject*)GetObject(uidObj);
		if (pObj) {
			if( pObj->GetStageState() == MOSS_READY) {
				MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_RELAY_LAUNCH, MUID(0,0));
				pCmd->AddParameter(new MCmdParamUID(uidStage));
				pCmd->AddParameter(new MCmdParamStr(const_cast<char*>(pStage->GetMapName())));
				pCmd->AddParameter(new MCmdParamBool(false));
				RouteToListener(pObj, pCmd);
			} else {
				MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_RELAY_LAUNCH, MUID(0,0));
				pCmd->AddParameter(new MCmdParamUID(uidStage));
				pCmd->AddParameter(new MCmdParamStr(const_cast<char*>(pStage->GetMapName())));
				pCmd->AddParameter(new MCmdParamBool(true));
				RouteToListener(pObj, pCmd);
			}
		} else {
			LOG(LOG_PROG, "WARNING(StageRelayLaunch) : Not Existing Obj(%u:%u)\n", uidObj.High, uidObj.Low);
			i=pStage->RemoveObject(uidObj);
			LogObjectCommandHistory(uidObj);
		}
	}
}

void MMatchServer::StageFinishGame(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	bool bIsRelayMapUnFinish = true;

	if(pStage->IsRelayMap())
	{ // ∏±∑π¿Ã ∏ ¿œ∂ßø°¥¬ πË∆≤¿ª ¥ŸΩ√ Ω√¿€«ÿ¡ÿ¥Ÿ. 
		if((int)pStage->m_vecRelayMapsRemained.size() <= 0)
		{	// ≥≤¿∫ ∏ ¿Ã æ¯¿ª∂ß
			int nRepeatCount = (int)pStage->m_RelayMapRepeatCountRemained - 1;
			if(nRepeatCount < 0)
			{
				bIsRelayMapUnFinish = false;

				pStage->m_bIsLastRelayMap = true;//∏±∑π¿Ã∏ ¿ª ≥°≥Ω¥Ÿ				
				nRepeatCount = 0;
				pStage->GetStageSetting()->SetMapName(MMATCH_MAPNAME_RELAYMAP);	//"RelayMap" ºº∆√
			}
			pStage->m_RelayMapRepeatCountRemained = (RELAY_MAP_REPEAT_COUNT)nRepeatCount;
			pStage->SetRelayMapCurrList(pStage->GetRelayMapList());
		}

		if(!pStage->m_bIsLastRelayMap) {
			// √≥¿Ω Ω√¿€Ω√, Flag∏¶ OnΩ√ƒ—¡ÿ¥Ÿ. 
			if( pStage->IsStartRelayMap() == false ) {
				pStage->SetIsStartRelayMap(true);
			}			

			if((int)pStage->m_vecRelayMapsRemained.size() > 0) { // ¥Ÿ¿Ω∏ ¿Ã ¿÷¥Ÿ∏È
				int nRelayMapIndex = 0;

				if(pStage->GetRelayMapType() == RELAY_MAP_TURN) {	//< ≥≤¿∫ ∞Õ¡ﬂø°º≠ √π π¯¬∞∫Œ≈Õ Ω√¿€(∞°µ∂º∫)
					nRelayMapIndex = 0; 
				} else if(pStage->GetRelayMapType() == RELAY_MAP_RANDOM) {
					nRelayMapIndex = rand() % (int)pStage->m_vecRelayMapsRemained.size();
				}

				if(nRelayMapIndex >= MAX_RELAYMAP_LIST_COUNT) { //< ∏  ±∏º∫¿Ã 20∞≥
					mlog("StageFinishGame RelayMap Fail RelayMapList MIsCorrect MaxCount[%d] \n", (int)nRelayMapIndex);
					return;
				}

				char* szMapName = (char*)MGetMapDescMgr()->GetMapName(pStage->m_vecRelayMapsRemained[nRelayMapIndex].nMapID);
				if (!szMapName)
				{
					mlog("RelayMapBattleStart Fail MapID[%d] \n", (int)pStage->m_vecRelayMapsRemained[nRelayMapIndex].nMapID);
					return;
				}

				pStage->GetStageSetting()->SetMapName(szMapName);

				// Ω««‡«— ∏±∑π¿Ã∏ ¿∫ ªË¡¶«ÿ¡ÿ¥Ÿ.
				vector<RelayMap>::iterator itor = pStage->m_vecRelayMapsRemained.begin();
				for(int i=0 ; nRelayMapIndex > i ; ++itor, ++i);// «ÿ¥Á ¿Œµ¶Ω∫±Ó¡ˆ ¿Ãµø
				pStage->m_vecRelayMapsRemained.erase(itor);
			} 
			else {
				mlog("MMatchServer::StageFinishGame::IsRelayMap() - m_vecRelayMapsRemained.size() == 0\n");
			}
		} else {
			pStage->SetIsStartRelayMap(false);
			bIsRelayMapUnFinish = false; // ∏±∑π¿Ã∏  ¡¯«‡¿Ã ≥°≥µ¿Ω
		}
	}

	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_FINISH_GAME, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(uidStage));
	pCmd->AddParameter(new MCommandParameterBool(bIsRelayMapUnFinish));
	RouteToStage(uidStage, pCmd);

	return;
}

MCommand* MMatchServer::CreateCmdResponseStageSetting(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return NULL;

	MCommand* pCmd = CreateCommand(MC_MATCH_RESPONSE_STAGESETTING, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(pStage->GetUID()));

	MMatchStageSetting* pSetting = pStage->GetStageSetting();

	// Param 1 : Stage Settings
	void* pStageSettingArray = MMakeBlobArray(sizeof(MSTAGE_SETTING_NODE), 1);
	MSTAGE_SETTING_NODE* pNode = (MSTAGE_SETTING_NODE*)MGetBlobArrayElement(pStageSettingArray, 0);
	CopyMemory(pNode, pSetting->GetStageSetting(), sizeof(MSTAGE_SETTING_NODE));
	pCmd->AddParameter(new MCommandParameterBlob(pStageSettingArray, MGetBlobArraySize(pStageSettingArray)));
	MEraseBlobArray(pStageSettingArray);

	// Param 2 : Char Settings
	int nCharCount = (int)pStage->GetObjCount();
	void* pCharArray = MMakeBlobArray(sizeof(MSTAGE_CHAR_SETTING_NODE), nCharCount);
	int nIndex=0;
	for (MUIDRefCache::iterator itor=pStage->GetObjBegin(); itor!=pStage->GetObjEnd(); itor++) {
		MSTAGE_CHAR_SETTING_NODE* pCharNode = (MSTAGE_CHAR_SETTING_NODE*)MGetBlobArrayElement(pCharArray, nIndex++);
		MMatchObject* pObj = (MMatchObject*)(*itor).second;
		pCharNode->uidChar = pObj->GetUID();
		pCharNode->nTeam = pObj->GetTeam();
		pCharNode->nState = pObj->GetStageState();
	}
	pCmd->AddParameter(new MCommandParameterBlob(pCharArray, MGetBlobArraySize(pCharArray)));
	MEraseBlobArray(pCharArray);

	// Param 3 : Stage State
	pCmd->AddParameter(new MCommandParameterInt((int)pStage->GetState()));

	// Param 4 : Stage Master
	pCmd->AddParameter(new MCommandParameterUID(pStage->GetMasterUID()));

	return pCmd;
}



void MMatchServer::OnStageCreate(const MUID& uidChar, char* pszStageName, bool bPrivate, char* pszStagePassword)
{
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;

	MMatchChannel* pChannel = FindChannel(pObj->GetChannelUID());
	if (pChannel == NULL) return;

	if ((MGetServerConfig()->GetServerMode() == MSM_CLAN) && (pChannel->GetChannelType() == MCHANNEL_TYPE_CLAN)
		&& (pChannel->GetChannelType() == MCHANNEL_TYPE_DUELTOURNAMENT)) {
		return;
	}
	
	MUID uidStage;

	if (!StageAdd(pChannel, pszStageName, bPrivate, pszStagePassword, &uidStage))
	{
		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_CREATE, MERR_CANNOT_CREATE_STAGE);
		return;
	}
	StageJoin(uidChar, uidStage);

	MMatchStage* pStage = FindStage(uidStage);
	if (pStage)
		pStage->SetFirstMasterName(pObj->GetCharInfo()->m_szName);
}


//void MMatchServer::OnStageJoin(const MUID& uidChar, const MUID& uidStage)
//{
//	MMatchObject* pObj = GetObject(uidChar);
//	if (pObj == NULL) return;
//
//	MMatchStage* pStage = NULL;
//
//	if (uidStage == MUID(0,0)) {
//		return;
//	} else {
//		pStage = FindStage(uidStage);
//	}
//
//	if (pStage == NULL) {
//		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_JOIN, MERR_STAGE_NOT_EXIST);
//		return;
//	}
//
//	if ((IsAdminGrade(pObj) == false) && pStage->IsPrivate())
//	{
//		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_JOIN, MERR_CANNOT_JOIN_STAGE_BY_PASSWORD);
//		return;
//	}
//
//	StageJoin(uidChar, pStage->GetUID());
//}

void MMatchServer::OnPrivateStageJoin(const MUID& uidPlayer, const MUID& uidStage, char* pszPassword)
{
	if (strlen(pszPassword) > STAGEPASSWD_LENGTH) return;

	MMatchStage* pStage = NULL;

	if (uidStage == MUID(0,0)) 
	{
		return;
	} 
	else 
	{
		pStage = FindStage(uidStage);
	}

	if (pStage == NULL) 
	{
		MMatchObject* pObj = GetObject(uidPlayer);
		if (pObj != NULL)
		{
			RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_JOIN, MERR_STAGE_NOT_EXIST);
		}

		return;
	}

	// øµ¿⁄≥™ ∞≥πﬂ¿⁄∏È π´Ω√..

	bool bSkipPassword = false;

	MMatchObject* pObj = GetObject(uidPlayer);

	if ((pObj == NULL) || (pObj->GetCharInfo() == NULL)) 
		return;

	MMatchUserGradeID ugid = pObj->GetAccountInfo()->m_nUGrade;

	if (ugid == MMUG_DEVELOPER || ugid == MMUG_ADMIN) 
		bSkipPassword = true;

	// ∫Òπ–πÊ¿Ã æ∆¥œ∞≈≥™ ∆–Ω∫øˆµÂ∞° ∆≤∏Æ∏È ∆–Ω∫øˆµÂ∞° ∆≤∑»¥Ÿ∞Ì ¿¿¥‰«—¥Ÿ.
	if(bSkipPassword==false) {
		if ((!pStage->IsPrivate()) || (strcmp(pStage->GetPassword(), pszPassword)))
		{
			MMatchObject* pObj = GetObject(uidPlayer);
			if (pObj != NULL)
			{
				RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_JOIN, MERR_CANNOT_JOIN_STAGE_BY_PASSWORD);
			}

			return;
		}
	}

	StageJoin(uidPlayer, pStage->GetUID());
}

void MMatchServer::OnStageFollow(const MUID& uidPlayer, const char* pszTargetName)
{
	MMatchObject* pPlayerObj = GetObject(uidPlayer);
	if (pPlayerObj == NULL) return;

	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
	if (pTargetObj == NULL) return;

	// ¿⁄±‚ ¿⁄Ω≈¿ª µ˚∂Û ∞°∑¡∞Ì «ﬂ¿ª∞ÊøÏ ∞ÀªÁ.
	if (pPlayerObj->GetUID() == pTargetObj->GetUID()) return;

	// Ω∫≈◊¿Ã∆Æ∞° ¿ﬂ∏¯µ«æÓ ¿÷¥¬¡ˆ ∞ÀªÁ.
	if (!pPlayerObj->CheckEnableAction(MMatchObject::MMOA_STAGE_FOLLOW)) return;


	// º≠∑Œ ¥Ÿ∏• √§≥Œ¿Œ¡ˆ ∞ÀªÁ.
	if (pPlayerObj->GetChannelUID() != pTargetObj->GetChannelUID()) {

#ifdef _VOTESETTING
		RouteResponseToListener( pPlayerObj, MC_MATCH_RESPONSE_STAGE_FOLLOW, MERR_CANNOT_FOLLOW );
#endif
		return;
	}

	if ((IsAdminGrade(pTargetObj) == true)) {
		NotifyMessage(pPlayerObj->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		return;
	}

	MMatchStage* pStage = FindStage(pTargetObj->GetStageUID());
	if (pStage == NULL) return;

	// ≈¨∑£¿¸∞‘¿”¿∫ µ˚∂Û∞• ºˆ æ¯¿Ω
	if (pStage->GetStageType() != MST_NORMAL) return;

	if (pStage->IsPrivate()==false) {
		if ((pStage->GetStageSetting()->GetForcedEntry()==false) && pStage->GetState() != STAGE_STATE_STANDBY) {
			// Deny Join

#ifdef _VOTESETTING
			RouteResponseToListener( pPlayerObj, MC_MATCH_RESPONSE_STAGE_FOLLOW, MERR_CANNOT_FOLLOW );
#endif
		} else {
			StageJoin(uidPlayer, pTargetObj->GetStageUID());
		}
	}
	else {
		// µ˚∂Û∞°∑¡¥¬ πÊ¿Ã ∫Òπ–π¯»£∏¶ « ø‰∑Œ «“∞ÊøÏ¥¬ µ˚∂Û∞•ºˆ æ¯¿Ω.
		//RouteResponseToListener( pPlayerObj, MC_MATCH_RESPONSE_STAGE_FOLLOW, MERR_CANNOT_FOLLOW_BY_PASSWORD );

		// «ÿ¥ÁπÊ¿Ã ∫Òπ–πÊ¿Ã∏È ∫Òπ–π¯»£∏¶ ø‰±∏«—¥Ÿ.
		MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_REQUIRE_PASSWORD), MUID(0,0), m_This);
		pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));
		pNew->AddParameter(new MCommandParameterString((char*)pStage->GetName()));
		RouteToListener(pPlayerObj, pNew);
	}
}

void MMatchServer::OnStageLeave(const MUID& uidPlayer)//, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject( uidPlayer );
	if( !IsEnabledObject(pObj) ) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	if( !IsEnabledObject(GetObject(uidPlayer)) )
	{
		return;
	}

	StageLeave(uidPlayer);// , uidStage);
}

void MMatchServer::OnStageRequestPlayerList(const MUID& uidPlayer, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	// MMatchStage* pStage = FindStage(uidStage);
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	// πÊ¿Œø¯ ∏Ò∑œ
	MMatchObjectCacheBuilder CacheBuilder;
	CacheBuilder.Reset();
	for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
		MMatchObject* pScanObj = (MMatchObject*)(*i).second;
		CacheBuilder.AddObject(pScanObj);
	}
    MCommand* pCmdCacheUpdate = CacheBuilder.GetResultCmd(MATCHCACHEMODE_UPDATE, this);
	RouteToListener(pObj, pCmdCacheUpdate);

	// Cast Master(πÊ¿Â)
	MUID uidMaster = pStage->GetMasterUID();
	MCommand* pMasterCmd = CreateCommand(MC_MATCH_STAGE_MASTER, MUID(0,0));
	pMasterCmd->AddParameter(new MCommandParameterUID(uidStage));
	pMasterCmd->AddParameter(new MCommandParameterUID(uidMaster));
	RouteToListener(pObj, pMasterCmd);

	// Cast Character Setting
	StageTeam(uidPlayer, uidStage, pObj->GetTeam());
	StagePlayerState(uidPlayer, uidStage, pObj->GetStageState());
}

void MMatchServer::OnStageEnterBattle(const MUID& uidPlayer, const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	StageEnterBattle(uidPlayer, uidStage);
}

void MMatchServer::OnStageLeaveBattle(const MUID& uidPlayer, bool bGameFinishLeaveBattle)//, const MUID& uidStage)
{
	if( !IsEnabledObject(GetObject(uidPlayer)) )
	{
		return;
	}

	StageLeaveBattle(uidPlayer, bGameFinishLeaveBattle, false);//, uidStage);
}


#include "CMLexicalAnalyzer.h"
// ∞≠≈ ¿”Ω√ƒ⁄µÂ
bool StageKick(MMatchServer* pServer, const MUID& uidPlayer, const MUID& uidStage, char* pszChat)
{
	MMatchObject* pChar = pServer->GetObject(uidPlayer);
	if (pChar == NULL)	return false;
	MMatchStage* pStage = pServer->FindStage(uidStage);
	if (pStage == NULL) return false;
	if (uidPlayer != pStage->GetMasterUID()) return false;

	bool bResult = false;
	CMLexicalAnalyzer lex;
	lex.Create(pszChat);

	if (lex.GetCount() >= 1) {
		char* pszCmd = lex.GetByStr(0);
		if (pszCmd) {
			if (stricmp(pszCmd, "/kick") == 0) {
				if (lex.GetCount() >= 2) {
					char* pszTarget = lex.GetByStr(1);
					if (pszTarget) {
						for (MUIDRefCache::iterator itor = pStage->GetObjBegin(); 
							itor != pStage->GetObjEnd(); ++itor)
						{
							MMatchObject* pTarget = (MMatchObject*)((*itor).second);
							if (stricmp(pszTarget, pTarget->GetName()) == 0) {
								if (pTarget->GetPlace() != MMP_BATTLE) {
									pServer->StageLeave(pTarget->GetUID());//, uidStage);
									bResult = true;
								}
								break;
							}
						}
					}
				}
			}	// Kick
		}
	}

	lex.Destroy();
	return bResult;
}

// πÊ¿Â»Æ¿Œ ¿”Ω√ƒ⁄µÂ
bool StageShowInfo(MMatchServer* pServer, const MUID& uidPlayer, const MUID& uidStage, char* pszChat)
{
	MMatchObject* pChar = pServer->GetObject(uidPlayer);
	if (pChar == NULL)	return false;
	MMatchStage* pStage = pServer->FindStage(uidStage);
	if (pStage == NULL) return false;
	if (uidPlayer != pStage->GetMasterUID()) return false;

	bool bResult = false;
	CMLexicalAnalyzer lex;
	lex.Create(pszChat);

	if (lex.GetCount() >= 1) {
		char* pszCmd = lex.GetByStr(0);
		if (pszCmd) {
			if (stricmp(pszCmd, "/showinfo") == 0) {
				char szMsg[256]="";
				sprintf(szMsg, "FirstMaster : (%s)", pStage->GetFirstMasterName());
				pServer->Announce(pChar, szMsg);
				bResult = true;
			}	// ShowInfo
		}
	}

	lex.Destroy();
	return bResult;
}
void MMatchServer::OnStageChat(const MUID& uidPlayer, const MUID& uidStage, char* pszChat)
{
	// RAONHAJE : ∞≠≈ ¿”Ω√ƒ⁄µÂ
	if (pszChat[0] == '/') {
		if (StageKick(this, uidPlayer, uidStage, pszChat))
			return;
		if (StageShowInfo(this, uidPlayer, uidStage, pszChat))
			return;
	}

	StageChat(uidPlayer, uidStage, pszChat);
}

void MMatchServer::OnStageStart(const MUID& uidPlayer, const MUID& uidStage, int nCountdown)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	if (pStage->GetMasterUID() != uidPlayer) return;

	if (pStage->StartGame(MGetServerConfig()->IsUseResourceCRC32CacheCheck()) == true) {
		StageRelayMapBattleStart(uidPlayer, uidStage);

		MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_START), MUID(0,0), m_This);
		pNew->AddParameter(new MCommandParameterUID(uidPlayer));
		pNew->AddParameter(new MCommandParameterUID(uidStage));
		pNew->AddParameter(new MCommandParameterInt(min(nCountdown,3)));
		RouteToStage(uidStage, pNew);

		// µ∫Òø° ∑Œ±◊∏¶ ≥≤±‰¥Ÿ.
		SaveGameLog(uidStage);
	}
}

void MMatchServer::OnStageRelayStart(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	
	if (pStage->StartRelayGame(MGetServerConfig()->IsUseResourceCRC32CacheCheck()) == true) {
		// µ∫Òø° ∑Œ±◊∏¶ ≥≤±‰¥Ÿ.
		SaveGameLog(uidStage);
	}
}

void MMatchServer::OnStartStageList(const MUID& uidComm)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	pObj->SetStageListTransfer(true);
}

void MMatchServer::OnStopStageList(const MUID& uidComm)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	pObj->SetStageListTransfer(false);
}

void MMatchServer::OnStagePlayerState(const MUID& uidPlayer, const MUID& uidStage, MMatchObjectStageState nStageState)
{
	StagePlayerState(uidPlayer, uidStage, nStageState);
}


void MMatchServer::OnStageTeam(const MUID& uidPlayer, const MUID& uidStage, MMatchTeam nTeam)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	MMatchObject* pChar = GetObject(uidPlayer);
	if (pChar == NULL) return;

	StageTeam(uidPlayer, uidStage, nTeam);
}

void MMatchServer::OnStageMap(const MUID& uidStage, char* pszMapName)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	if (pStage->GetState() != STAGE_STATE_STANDBY) return;	// ¥Î±‚ªÛ≈¬ø°º≠∏∏ πŸ≤‹ºˆ ¿÷¥Ÿ
	if (strlen(pszMapName) < 2) return;

	pStage->SetMapName( pszMapName );
	pStage->SetIsRelayMap(strcmp(MMATCH_MAPNAME_RELAYMAP, pszMapName) == 0);
	
	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_MAP), MUID(0,0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidStage));
	pNew->AddParameter(new MCommandParameterString(pStage->GetMapName()));

	if ( MGetGameTypeMgr()->IsQuestDerived( pStage->GetStageSetting()->GetGameType()))
	{
		MMatchRuleBaseQuest* pQuest = reinterpret_cast< MMatchRuleBaseQuest* >( pStage->GetRule() );
		pQuest->RefreshStageGameInfo();
	}

    RouteToStage(uidStage, pNew);
}

void MMatchServer::StageRelayMapBattleStart(const MUID& uidPlayer, const MUID& uidStage)
{// ∏±∑π¿Ã∏  º±≈√«œ∞Ì ∞‘¿” Ω√¿€ πˆ∆∞ ¥©∏£∏È ¥Ÿ¿Ω¿ª ºˆ«‡«—¥Ÿ
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	if (pStage->GetMasterUID() != uidPlayer) return;
	if(!pStage->IsRelayMap()) return;
	
	// ∞‘¿” √≥¿ΩΩ√¿€Ω√ √ ±‚»≠ «ÿ¡÷±‚
	pStage->InitCurrRelayMap();

	if (pStage->m_vecRelayMapsRemained.empty()) return;

	if((int)pStage->m_vecRelayMapsRemained.size() > MAX_RELAYMAP_LIST_COUNT)
	{// ∏  ±∏º∫¿Ã 20∞≥ √ ∞˙«œ∏È ø°∑Ø
		mlog("RelayMapBattleStart Fail RelayMapList MIsCorrect OverCount[%d] \n", (int)pStage->m_vecRelayMapsRemained.size());
		return;
	}

	if (pStage->m_vecRelayMapsRemained.size() != pStage->GetRelayMapListCount())
	{
		mlog("m_vecRelayMapsRemained[%d] != GetRelayMapListCount[%d]\n", (int)pStage->m_vecRelayMapsRemained.size(), pStage->GetRelayMapListCount());
		return;
	}

	// √≥¿Ω Ω««‡«“ ∏ ¿ª ¡§«—¥Ÿ
	int nRelayMapIndex = 0;
	if(pStage->GetRelayMapType() == RELAY_MAP_TURN )
		nRelayMapIndex = 0; // ≥≤¿∫∞Õ¡ﬂø°º≠ √≥¿Ωπ¯¬∞ ∫Œ≈Õ Ω√¿€(∞°µ∂º∫)
	else if(pStage->GetRelayMapType() == RELAY_MAP_RANDOM)
		nRelayMapIndex = rand() % int(pStage->m_vecRelayMapsRemained.size());

	if(MMATCH_MAP_RELAYMAP == pStage->m_vecRelayMapsRemained[nRelayMapIndex].nMapID)
	{
		mlog("RelayMapBattleStart Fail Type[%d], RoundCount[Curr:%d][%d], ListCount[Curr:%d][%d] \n",  
			pStage->GetRelayMapType(), pStage->m_RelayMapRepeatCountRemained, pStage->GetRelayMapRepeatCount(), (int)pStage->m_vecRelayMapsRemained.size(), pStage->GetRelayMapListCount());
		return;
	}

	char* szMapName = (char*)MGetMapDescMgr()->GetMapName(pStage->m_vecRelayMapsRemained[nRelayMapIndex].nMapID);
	if (!szMapName)
	{
		mlog("RelayMapBattleStart Fail MapID[%d] \n", (int)pStage->m_vecRelayMapsRemained[nRelayMapIndex].nMapID);
		return;
	}

	pStage->GetStageSetting()->SetMapName(szMapName);

	// Ω««‡«— ∏±∑π¿Ã∏ ¿∫ ªË¡¶«ÿ¡ÿ¥Ÿ.
	vector<RelayMap>::iterator itor = pStage->m_vecRelayMapsRemained.begin();
	for(int i=0 ; nRelayMapIndex > i ; ++itor, ++i);// «ÿ¥Á ¿Œµ¶Ω∫±Ó¡ˆ ¿Ãµø
	pStage->m_vecRelayMapsRemained.erase(itor);
}

void MMatchServer::OnStageRelayMapElementUpdate(const MUID& uidStage, int nType, int nRepeatCount)
{
	MMatchStage* pStage = FindStage(uidStage);
	pStage->SetRelayMapType((RELAY_MAP_TYPE)nType);
	pStage->SetRelayMapRepeatCount((RELAY_MAP_REPEAT_COUNT)nRepeatCount);

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_RELAY_MAP_ELEMENT_UPDATE), MUID(0,0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidStage));
	pNew->AddParameter(new MCommandParameterInt((int)pStage->GetRelayMapType()));
	pNew->AddParameter(new MCommandParameterInt((int)pStage->GetRelayMapRepeatCount()));
	RouteToStage(uidStage, pNew);
}

void MMatchServer::OnStageRelayMapListUpdate(const MUID& uidStage, int nRelayMapType, int nRelayMapRepeatCount, void* pRelayMapListBlob)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	if(!pStage->IsRelayMap()) return;
	if (pStage->GetState() != STAGE_STATE_STANDBY) return;	// ¥Î±‚ªÛ≈¬ø°º≠∏∏ πŸ≤‹ºˆ ¿÷¥Ÿ

	// ∏±∑π¿Ã∏  ¡§∫∏∏¶ º≠πˆ¬  Ω∫≈◊¿Ã¡ˆ∏¶ ∞ªΩ≈
	RelayMap relayMapList[MAX_RELAYMAP_LIST_COUNT];
	for (int i = 0; i < MAX_RELAYMAP_LIST_COUNT; i++)
		relayMapList[i].nMapID = -1;
	int nRelayMapListCount = MGetBlobArrayCount(pRelayMapListBlob);
	if(nRelayMapListCount > MAX_RELAYMAP_LIST_COUNT)
		nRelayMapListCount = MAX_RELAYMAP_LIST_COUNT;
	for (int i = 0; i < nRelayMapListCount; i++)
	{
		MTD_RelayMap* pRelayMap = (MTD_RelayMap*)MGetBlobArrayElement(pRelayMapListBlob, i);
		if(!MGetMapDescMgr()->MIsCorrectMap(pRelayMap->nMapID))
		{
			mlog("OnStageRelayMapListUpdate Fail MIsCorrectMap ID[%d] \n", (int)pRelayMap->nMapID);
			break;
		}
		relayMapList[i].nMapID = pRelayMap->nMapID;
	}

	pStage->SetRelayMapType((RELAY_MAP_TYPE)nRelayMapType);
	pStage->SetRelayMapRepeatCount((RELAY_MAP_REPEAT_COUNT)nRelayMapRepeatCount);
	pStage->SetRelayMapList(relayMapList);
	pStage->InitCurrRelayMap();


	// ∫Ì∑∞ ∏∏µÈ±‚, ∏ ∏ÆΩ∫∆Æ ºº∆√
	void* pRelayMapListBlob = MMakeBlobArray(sizeof(MTD_RelayMap), pStage->GetRelayMapListCount());
	RelayMap RelayMapList[MAX_RELAYMAP_LIST_COUNT];
	memcpy(RelayMapList, pStage->GetRelayMapList(), sizeof(RelayMap)*MAX_RELAYMAP_LIST_COUNT);
	for (int i = 0; i < pStage->GetRelayMapListCount(); i++)
	{
		MTD_RelayMap* pRelayMapList = (MTD_RelayMap*)MGetBlobArrayElement(pRelayMapListBlob, i);
		pRelayMapList->nMapID = RelayMapList[i].nMapID;
	}

	// πÊ¿Â¿Ã ∫∏≥Ω ∏±∑π¿Ã∏  ¡§∫∏∏¶ πÊø¯µÈø°∞‘ ∫∏≥ø
	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_RELAY_MAP_INFO_UPDATE), MUID(0,0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidStage));
	pNew->AddParameter(new MCommandParameterInt((int)pStage->GetRelayMapType()));
	pNew->AddParameter(new MCommandParameterInt((int)pStage->GetRelayMapRepeatCount()));
	pNew->AddParameter(new MCommandParameterBlob(pRelayMapListBlob, MGetBlobArraySize(pRelayMapListBlob)));
	RouteToStage(uidStage, pNew);
}
void MMatchServer::OnStageRelayMapListInfo(const MUID& uidStage, const MUID& uidChar)
{
	MMatchStage* pStage = FindStage(uidStage);
	if(pStage == NULL) return;
	if(!pStage->IsRelayMap()) return;
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;
	// ¥Î±‚ªÛ≈¬¿œ∂ß πÊ¿Â¿∫ √≥∏Æ æ»«ÿ¡‹(∏±∑π¿Ã∏  ¿€º∫¡ﬂ¿œºˆµµ ¿÷¿Ω)
	if(pStage->GetState() == STAGE_STATE_STANDBY && pStage->GetMasterUID() == uidChar) return;	

	// ∫Ì∑∞ ∏∏µÈ±‚, ∏ ∏ÆΩ∫∆Æ ºº∆√
	void* pRelayMapListBlob = MMakeBlobArray(sizeof(MTD_RelayMap), pStage->GetRelayMapListCount());
	RelayMap RelayMapList[MAX_RELAYMAP_LIST_COUNT];
	memcpy(RelayMapList, pStage->GetRelayMapList(), sizeof(RelayMap)*MAX_RELAYMAP_LIST_COUNT);
	for (int i = 0; i < pStage->GetRelayMapListCount(); i++)
	{
		MTD_RelayMap* pRelayMapList = (MTD_RelayMap*)MGetBlobArrayElement(pRelayMapListBlob, i);
		pRelayMapList->nMapID = RelayMapList[i].nMapID;
	}
	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_RELAY_MAP_INFO_UPDATE), MUID(0,0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidStage));
	pNew->AddParameter(new MCommandParameterInt((int)pStage->GetRelayMapType()));
	pNew->AddParameter(new MCommandParameterInt((int)pStage->GetRelayMapRepeatCount()));
	pNew->AddParameter(new MCommandParameterBlob(pRelayMapListBlob, MGetBlobArraySize(pRelayMapListBlob)));
	MEraseBlobArray(pRelayMapListBlob);

	RouteToListener(pObj, pNew); // πÊ¿Â¿Ã ∏±∑π¿Ã∏  º≥¡§¡ﬂø° æ˜µ•¿Ã∆Æµ» º≥¡§¿∏∑Œ ∫Ø∞Ê µ…ºˆ∞° ¿÷¿Ω
}

void MMatchServer::OnStageSetting(const MUID& uidPlayer, const MUID& uidStage, void* pStageBlob, int nStageCount)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	if (pStage->GetState() != STAGE_STATE_STANDBY) return;	// ¥Î±‚ªÛ≈¬ø°º≠∏∏ πŸ≤‹ºˆ ¿÷¥Ÿ
	if (nStageCount <= 0) return;

	// validate
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) {
		mlog(" stage setting invalid object (%d, %d) ignore\n", uidPlayer.High, uidPlayer.Low);
		return;
	}

	if( pObj->GetStageUID()!=uidStage ||  nStageCount!=1 ||
		MGetBlobArraySize(pStageBlob) != (sizeof(MSTAGE_SETTING_NODE)+sizeof(int)*2) )
	{
		mlog(" stage setting hack %s (%d, %d) ignore\n", pObj->GetName(), uidPlayer.High, uidPlayer.Low);
		LogObjectCommandHistory( uidPlayer );
		return;
	}

	// πÊ¿Â¿Ã∞≈≥™ øÓøµ¿⁄∞° æ∆¥—µ• ºº∆√¿ª πŸ≤Ÿ∏È ±◊≥… ∏Æ≈œ
	if (pStage->GetMasterUID() != uidPlayer)
	{
		MMatchObject* pObjMaster = GetObject(uidPlayer);
		if (!IsAdminGrade(pObjMaster)) return;
	}


	MSTAGE_SETTING_NODE* pNode = (MSTAGE_SETTING_NODE*)MGetBlobArrayElement(pStageBlob, 0);

	// let's refactor
	if( (pNode->nGameType < MMATCH_GAMETYPE_DEATHMATCH_SOLO) || (pNode->nGameType >= MMATCH_GAMETYPE_MAX)) {
		mlog(" stage setting game mode hack %s (%d, %d) ignore\n", pObj->GetName(), uidPlayer.High, uidPlayer.Low);
		LogObjectCommandHistory( uidPlayer );

		// µ∫Òø° ≥≤±‚¿⁄.
//		pObj->SetInvalidStageSettingDisconnectWaitInfo();
		pObj->DisconnectHacker( MMHT_INVALIDSTAGESETTING );

		return;
	}

	// º≠πŸ¿Ãπ˙¿Ã ∫Ò»∞º∫ ºº∆√¿Œµ• º≠πŸ¿Ãπ˙ ø‰√ªΩ√
	if( MGetServerConfig()->IsEnabledSurvivalMode()==false && pNode->nGameType==MMATCH_GAMETYPE_SURVIVAL) {
		mlog(" stage setting game mode hack %s (%d, %d) ignore\n", pObj->GetName(), uidPlayer.High, uidPlayer.Low);
		LogObjectCommandHistory( uidPlayer );
		pObj->DisconnectHacker( MMHT_INVALIDSTAGESETTING );
		return;
	}

	// ±‚∫ª¿˚¿∏∑Œ √÷¥Î ¿Œø¯¿Ã STAGE_BASIC_MAX_PLAYERCOUNT¿Ã ≥—¿∏∏È STAGE_BASIC_MAX_PLAYERCOUNT∑Œ ∏¬√Á¡‹.
	// ≥≤¿∫ ¿€æ˜¿ª ¡¯«‡«œ∏È¿∫ ∞¢ ∞‘¿”ø° ∏¬¥¬ ¿Œø¯¿∏∑Œ º¬∆√¿ª «‘. - by SungE 2007-05-14
	if( STAGE_MAX_PLAYERCOUNT < pNode->nMaxPlayers )
		pNode->nMaxPlayers = STAGE_MAX_PLAYERCOUNT;

	// ¿Ã ¿ÃªÛ¿« ∂ÛøÓµÂ º¬∆√¿∫ ∫“∞°¥… «œ¥Ÿ. π´¡∂∞« ∫∏¡§«—¥Ÿ. - By SungE 2007-11-07
	if( STAGE__MAX_ROUND < pNode->nRoundMax )
		pNode->nRoundMax = STAGE__MAX_ROUND;

	MMatchStageSetting* pSetting = pStage->GetStageSetting();
	MMatchChannel* pChannel = FindChannel(pStage->GetOwnerChannel());

	bool bCheckChannelRule = true;

	if (QuestTestServer())
	{
		if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType))
		{
			bCheckChannelRule = false;
		}
	}

	if ((pChannel) && (bCheckChannelRule))
	{
		// ºº∆√«“ ºˆ ¿÷¥¬ ∏ , ∞‘¿”≈∏¿‘¿Œ¡ˆ √º≈©
		MChannelRule* pRule = MGetChannelRuleMgr()->GetRule(pChannel->GetRuleType());
		if (pRule)
		{
			if (!pRule->CheckGameType(pNode->nGameType))
			{
				pNode->nGameType = MMATCH_GAMETYPE_DEATHMATCH_SOLO;
			}

			bool bDuelMode = false;
			bool bCTFMode = false;//xxx
			if ( pNode->nGameType == MMATCH_GAMETYPE_DUEL)
				bDuelMode = true;

			if ( pNode->nGameType == MMATCH_GAMETYPE_CTF)//xxx
				bCTFMode = true;

//XXX
			//CTF MAP
			if (!pRule->CheckCTFMap(pNode->nMapIndex) && bCTFMode)
			{
				strcpy(pNode->szMapName, MGetMapDescMgr()->GetMapName(MMATCH_MAP_MANSION));
				pNode->nMapIndex = 0;
			}
			if (!pRule->CheckMap(pNode->nMapIndex, bDuelMode) && !bCTFMode) //XXX
			{
				strcpy(pNode->szMapName, MGetMapDescMgr()->GetMapName(MMATCH_MAP_MANSION));
				pNode->nMapIndex = 0;
			}
			else if(!bCTFMode)//xxx
			{
				strcpy(pNode->szMapName, pSetting->GetMapName());
				pNode->nMapIndex = pSetting->GetMapIndex();
			}
		}
	}

	MMATCH_GAMETYPE nLastGameType = pSetting->GetGameType();

	// ƒ˘Ω∫∆Æ ∏µÂ¿Ã∏È π´¡∂∞« ≥≠¿‘∫“∞°, √÷¥Î¿Œø¯ 4∏Ì¿∏∑Œ ºº∆√«—¥Ÿ.
	if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType))
	{
		if (pNode->bForcedEntryEnabled == true) pNode->bForcedEntryEnabled = false;
		pNode->nMaxPlayers = STAGE_QUEST_MAX_PLAYER;
		pNode->nLimitTime = STAGESETTING_LIMITTIME_UNLIMITED;


		// ƒ˘Ω∫∆Æ º≠πˆ∞° æ∆¥—µ• ƒ˘Ω∫∆Æ ∞‘¿”¿Ã∏È º÷∑Œµ•Ω∫∏≈ƒ°∑Œ πŸ≤€¥Ÿ.
		if (!QuestTestServer())
		{
			pNode->nGameType = MMATCH_GAMETYPE_DEATHMATCH_SOLO;
		}
	}

	// ƒ˘Ω∫∆Æ ∏µÂø¥¥Ÿ∞° ¥Ÿ∏• ∏µÂ∞° µ«∏È '≥≠¿‘∫“∞°'∏¶ «„øÎ¿∏∑Œ ∫Ø∞Ê
	if (MGetGameTypeMgr()->IsQuestDerived( nLastGameType ) == true &&
		MGetGameTypeMgr()->IsQuestDerived( pNode->nGameType ) == false)
		pNode->bForcedEntryEnabled = true;

	if (!MGetGameTypeMgr()->IsTeamGame(pNode->nGameType))
	{
		pNode->bAutoTeamBalancing = true;
	}

	// ∏±∑π¿Ã∏  ºº∆√
	pStage->SetIsRelayMap(strcmp(MMATCH_MAPNAME_RELAYMAP, pNode->szMapName) == 0);
	pStage->SetIsStartRelayMap(false);

	if(!pStage->IsRelayMap())
	{	// ∏±∑π¿Ã∏ ¿Ã æ∆¥œ∏È ±‚∫ª¿∏∑Œ √ ±‚»≠ «ÿ¡ÿ¥Ÿ.
		pNode->bIsRelayMap = pStage->IsRelayMap();
		pNode->bIsStartRelayMap = pStage->IsStartRelayMap();
		for (int i=0; i<MAX_RELAYMAP_LIST_COUNT; ++i)
			pNode->MapList[i].nMapID = -1;
		pNode->nRelayMapListCount = 0;
		pNode->nRelayMapType = RELAY_MAP_TURN;
		pNode->nRelayMapRepeatCount = RELAY_MAP_3REPEAT;
	}


	pSetting->UpdateStageSetting(pNode);
	pStage->ChangeRule(pNode->nGameType);


	MCommand* pCmd = CreateCmdResponseStageSetting(uidStage);
	RouteToStage(uidStage, pCmd);


	// ∞‘¿” ∏µÂ∞° ∫Ø∞Êµ«æ˙¿ª∞ÊøÏ
	if (nLastGameType != pSetting->GetGameType())
	{
		char szNewMap[ MAPNAME_LENGTH ] = {0};

		if (MGetGameTypeMgr()->IsQuestDerived( nLastGameType ) == false &&
			MGetGameTypeMgr()->IsQuestDerived( pSetting->GetGameType() ) == true)
		{
//			OnStageMap(uidStage, GetQuest()->GetSurvivalMapInfo(MSURVIVAL_MAP(0))->szName);
//			OnStageMap(uidStage, pSetting->GetMapName());
			OnStageMap(uidStage, MMATCH_DEFAULT_STAGESETTING_MAPNAME);

			MMatchRuleBaseQuest* pQuest = reinterpret_cast< MMatchRuleBaseQuest* >( pStage->GetRule());
			pQuest->RefreshStageGameInfo();
		}
		else if ( (nLastGameType != MMATCH_GAMETYPE_DUEL) && ( pSetting->GetGameType() == MMATCH_GAMETYPE_DUEL))
		{
			strcpy( szNewMap, MGetMapDescMgr()->GetMapName( MMATCH_MAP_HALL));
			OnStageMap(uidStage, szNewMap);
		}
		else if ( ((nLastGameType == MMATCH_GAMETYPE_QUEST) || (nLastGameType == MMATCH_GAMETYPE_SURVIVAL) || (nLastGameType == MMATCH_GAMETYPE_DUEL)) &&
			      ((pSetting->GetGameType() != MMATCH_GAMETYPE_QUEST) && (pSetting->GetGameType() != MMATCH_GAMETYPE_SURVIVAL) && ( pSetting->GetGameType() != MMATCH_GAMETYPE_DUEL)))
		{
			strcpy( szNewMap, MGetMapDescMgr()->GetMapName( MMATCH_MAP_MANSION));
			OnStageMap(uidStage, szNewMap);
		}
	}
}

void MMatchServer::OnRequestStageSetting(const MUID& uidComm, const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	MCommand* pCmd = CreateCmdResponseStageSetting(uidStage);
	pCmd->m_Receiver = uidComm;
	Post(pCmd);

	// ∏  º±≈√¿Ã ∏±∑π¿Ã∏ ¿Ã∏È √≥∏Æ«ÿ¡ÿ¥Ÿ.
	OnStageRelayMapListInfo(uidStage, uidComm);

	MMatchObject* pChar = GetObject(uidComm);
	if (pChar && (MMUG_EVENTMASTER == pChar->GetAccountInfo()->m_nUGrade)) 	{
		// ¿Ã∫•∆Æ ∏∂Ω∫≈Õø°∞‘ √≥¿Ω πÊ∏∏µÈæ˙¥¯ ªÁ∂˜¿ª æÀ∑¡¡ÿ¥Ÿ
		StageShowInfo(this, uidComm, uidStage, "/showinfo");
	}
}

void MMatchServer::OnRequestPeerList(const MUID& uidChar, const MUID& uidStage)
{
	ResponsePeerList(uidChar, uidStage);
}

void MMatchServer::OnRequestGameInfo(const MUID& uidChar, const MUID& uidStage)
{
	ResponseGameInfo(uidChar, uidStage);
}

void MMatchServer::ResponseGameInfo(const MUID& uidChar, const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage); if (pStage == NULL) return;
	MMatchObject* pObj = GetObject(uidChar); if (pObj == NULL) return;
	if (pStage->GetRule() == NULL) return;

	MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_GAME_INFO, MUID(0,0));
	pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));

	// ∞‘¿”¡§∫∏
	void* pGameInfoArray = MMakeBlobArray(sizeof(MTD_GameInfo), 1);
	MTD_GameInfo* pGameItem = (MTD_GameInfo*)MGetBlobArrayElement(pGameInfoArray, 0);
	memset(pGameItem, 0, sizeof(MTD_GameInfo));
	
	if (pStage->GetStageSetting()->IsTeamPlay())
	{
		pGameItem->nRedTeamScore = static_cast<char>(pStage->GetTeamScore(MMT_RED));
		pGameItem->nBlueTeamScore = static_cast<char>(pStage->GetTeamScore(MMT_BLUE));

		pGameItem->nRedTeamKills = static_cast<short>(pStage->GetTeamKills(MMT_RED));
		pGameItem->nBlueTeamKills = static_cast<short>(pStage->GetTeamKills(MMT_BLUE));
	}

	pNew->AddParameter(new MCommandParameterBlob(pGameInfoArray, MGetBlobArraySize(pGameInfoArray)));
	MEraseBlobArray(pGameInfoArray);

	// ∑Í¡§∫∏
	void* pRuleInfoArray = NULL;
	if (pStage->GetRule())
		pRuleInfoArray = pStage->GetRule()->CreateRuleInfoBlob();
	if (pRuleInfoArray == NULL)
		pRuleInfoArray = MMakeBlobArray(0, 0);
	pNew->AddParameter(new MCommandParameterBlob(pRuleInfoArray, MGetBlobArraySize(pRuleInfoArray)));
	MEraseBlobArray(pRuleInfoArray);

	// Battleø° µÈæÓ∞£ ªÁ∂˜∏∏ List∏¶ ∏∏µÁ¥Ÿ.
	int nPlayerCount = pStage->GetObjInBattleCount();

	void* pPlayerItemArray = MMakeBlobArray(sizeof(MTD_GameInfoPlayerItem), nPlayerCount);
	int nIndex=0;
	for (MUIDRefCache::iterator itor=pStage->GetObjBegin(); itor!=pStage->GetObjEnd(); itor++) 
	{
		MMatchObject* pObj = (MMatchObject*)(*itor).second;
		if (pObj->GetEnterBattle() == false) continue;

		MTD_GameInfoPlayerItem* pPlayerItem = (MTD_GameInfoPlayerItem*)MGetBlobArrayElement(pPlayerItemArray, nIndex++);
		pPlayerItem->uidPlayer = pObj->GetUID();
		pPlayerItem->bAlive = pObj->CheckAlive();
		pPlayerItem->nKillCount = pObj->GetAllRoundKillCount();
		pPlayerItem->nDeathCount = pObj->GetAllRoundDeathCount();
	}
	pNew->AddParameter(new MCommandParameterBlob(pPlayerItemArray, MGetBlobArraySize(pPlayerItemArray)));
	MEraseBlobArray(pPlayerItemArray);

	RouteToListener(pObj, pNew);
}

void MMatchServer::OnMatchLoadingComplete(const MUID& uidPlayer, int nPercent)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	MCommand* pCmd = CreateCommand(MC_MATCH_LOADING_COMPLETE, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUID(uidPlayer));
	pCmd->AddParameter(new MCmdParamInt(nPercent));
	RouteToStage(pObj->GetStageUID(), pCmd);	
}


void MMatchServer::OnGameRoundState(const MUID& uidStage, int nState, int nRound)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	pStage->RoundStateFromClient(uidStage, nState, nRound);
}


void MMatchServer::OnDuelSetObserver(const MUID& uidChar)
{
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;

	MCommand* pCmd = CreateCommand(MC_MATCH_SET_OBSERVER, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUID(uidChar));
	RouteToBattle(pObj->GetStageUID(), pCmd);
}

void MMatchServer::OnRequestSpawn(const MUID& uidChar, const MVector& pos, const MVector& dir)
{
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;

	// Do Not Spawn when AdminHiding
	if (IsAdminGrade(pObj) && pObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide)) return;


	// ∏∂¡ˆ∏∑ ¡◊æ˙¥¯ Ω√∞£∞˙ ªı∑Œ ∏ÆΩ∫∆˘¿ª ø‰√ª«— Ω√∞£ ªÁ¿Ãø° 2√  ¿ÃªÛ¿« Ω√∞£¿Ã ¿÷æ˙¥¬¡ˆ ∞ÀªÁ«—¥Ÿ.
	DWORD dwTime = timeGetTime() - pObj->GetLastSpawnTime();	
	if ( dwTime < RESPAWN_DELAYTIME_AFTER_DYING_MIN) return;
	pObj->SetLastSpawnTime(timeGetTime());

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;
	if ( (pStage->GetRule()->GetRoundState() != MMATCH_ROUNDSTATE_PREPARE) && (!pObj->IsEnabledRespawnDeathTime(GetTickTime())) )
		 return;

	MMatchRule* pRule = pStage->GetRule();					// ¿Ã∑± Ωƒ¿« ƒ⁄µÂ¥¬ ∏∂¿Ωø° æ»µÈ¡ˆ∏∏ -_-; ∞‘¿”≈∏¿‘ ∫∏∞Ì øπø‹√≥∏Æ.
	MMATCH_GAMETYPE gameType = pRule->GetGameType();		// ¥Ÿ∏• πÊπ˝ ¿÷≥™ø‰.
	if (gameType == MMATCH_GAMETYPE_DUEL)
	{
		MMatchRuleDuel* pDuel = (MMatchRuleDuel*)pRule;		// RTTI æ»Ω·º≠ dynamic cast¥¬ ∆–Ω∫.. øπø‹√≥∏Æµµ ¬•¡ı≥™∞Ì -,.-
		if (uidChar != pDuel->uidChampion && uidChar != pDuel->uidChallenger)
		{
			OnDuelSetObserver(uidChar);
			return;
		}
	}

	pObj->ResetCustomItemUseCount();
	pObj->SetAlive(true);

	MCommand* pCmd = CreateCommand(MC_MATCH_GAME_RESPONSE_SPAWN, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUID(uidChar));
	pCmd->AddParameter(new MCmdParamShortVector(pos.x, pos.y, pos.z));
	pCmd->AddParameter(new MCmdParamShortVector(DirElementToShort(dir.x), DirElementToShort(dir.y), DirElementToShort(dir.z)));
	RouteToBattle(pObj->GetStageUID(), pCmd);
}

void MMatchServer::OnGameRequestTimeSync(const MUID& uidComm, unsigned long nLocalTimeStamp)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	MMatchTimeSyncInfo* pSync = pObj->GetSyncInfo();
	pSync->Update(GetGlobalClockCount());

	MCommand* pCmd = CreateCommand(MC_MATCH_GAME_RESPONSE_TIMESYNC, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUInt(nLocalTimeStamp));
	pCmd->AddParameter(new MCmdParamUInt(GetGlobalClockCount()));
	RouteToListener(pObj, pCmd);
}

void MMatchServer::OnGameReportTimeSync(const MUID& uidComm, unsigned long nLocalTimeStamp, unsigned int nDataChecksum)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	pObj->UpdateTickLastPacketRecved();	// Last Packet Timestamp

	if (pObj->GetEnterBattle() == false)
		return;

	//// SpeedHack Test ////
	MMatchTimeSyncInfo* pSync = pObj->GetSyncInfo();
	int nSyncDiff = nLocalTimeStamp - pSync->GetLastSyncClock();
	float fError = static_cast<float>(nSyncDiff) / static_cast<float>(MATCH_CYCLE_CHECK_SPEEDHACK);
	if (fError > 2.f) {	
		pSync->AddFoulCount();
		if (pSync->GetFoulCount() >= 3) {	// 3ø¨º” Ω∫««µÂ«Ÿ ∞À√‚ - 3¡¯æ∆øÙ

			#ifndef _DEBUG		// µπˆ±◊«“∂ß¥¬ ª©≥ıæ“¿Ω
				NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GAME_SPEEDHACK);
				StageLeave(pObj->GetUID());//, pObj->GetStageUID());
				Disconnect(pObj->GetUID());
			#endif
			mlog("SPEEDHACK : User='%s', SyncRatio=%f (TimeDiff=%d) \n", 
				pObj->GetName(), fError, nSyncDiff);
			pSync->ResetFoulCount();
		}
	} else {
		pSync->ResetFoulCount();
	}
	pSync->Update(GetGlobalClockCount());

	//// MemoryHack Test ////
	if (nDataChecksum > 0) {	// º≠πˆ∞° Client MemoryChecksum ∏∏£π«∑Œ ¿œ¥‹ ≈¨∂Û¿Ãæ∆Æ∞° Ω≈∞Ì«œ¥¬¿«πÃ∑Œ ªÁøÎ«—¥Ÿ.
		NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GAME_MEMORYHACK);
		StageLeave(pObj->GetUID());//, pObj->GetStageUID());
		Disconnect(pObj->GetUID());
		mlog("MEMORYHACK : User='%s', MemoryChecksum=%u \n", pObj->GetName(), nDataChecksum);
	}
}

void MMatchServer::OnUpdateFinishedRound(const MUID& uidStage, const MUID& uidChar, 
						   void* pPeerInfo, void* pKillInfo)
{

}

void MMatchServer::OnRequestForcedEntry(const MUID& uidStage, const MUID& uidChar)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	MMatchObject* pObj = GetObject(uidChar);	
	if (pObj == NULL) return;

	pObj->SetForcedEntry(true);

	RouteResponseToListener(pObj, MC_MATCH_STAGE_RESPONSE_FORCED_ENTRY, MOK);
}

void MMatchServer::OnRequestSuicide(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	pStage->ReserveSuicide( uidPlayer, MGetMatchServer()->GetGlobalClockCount() );

	// OnGameKill(uidPlayer, uidPlayer);

	//MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SUICIDE, MUID(0,0));
	//int nResult = MOK;
	//pNew->AddParameter(new MCommandParameterInt(nResult));
	//pNew->AddParameter(new MCommandParameterUID(uidPlayer));
	//RouteToBattle(pObj->GetStageUID(), pNew);
}

void MMatchServer::OnRequestObtainWorldItem(const MUID& uidPlayer, const int nItemUID)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	pStage->ObtainWorldItem(pObj, nItemUID);
}

void MMatchServer::OnRequestFlagCap(const MUID& uidPlayer, const int nItemID)//XXX
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	if(pStage->GetStageSetting())
	{
		if(pStage->GetStageSetting()->GetGameType() == MMATCH_GAMETYPE_CTF)
		{
			MMatchRuleTeamCTF* pRule = (MMatchRuleTeamCTF*)pStage->GetRule();
			if(pRule)
			{
			pRule->OnObtainWorldItem( pObj, nItemID, NULL );
			}
		}
	}
}

void MMatchServer::OnRequestSpawnWorldItem(const MUID& uidPlayer, const int nItemID, const float x, const float y, const float z, float fDropDelayTime)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	if( !pObj->IsHaveCustomItem() )
		return;

	/*
	¬¯øÎ«œ∞Ì ¿÷¥¬ ¬ ¿ª æÀ ºˆ æ¯¿∏π«∑Œ µŒ¬ ¿« «’¿∏∑Œ √÷¥Î ªÁøÎ«“ ºˆ ¿÷¥¬ ºˆ∑Æ¿ª ±∏«—»ƒ
	±◊ «—µµ æ»ø°º≠ ªÁøÎ«“ ºˆ ¿÷µµ∑œ «—¥Ÿ.
	*/
	if( pObj->IncreaseCustomItemUseCount() )
	{
		pStage->RequestSpawnWorldItem(pObj, nItemID, x, y, z, fDropDelayTime);
	}
}

void MMatchServer::OnNotifyThrowTrapItem(const MUID& uidPlayer, const int nItemID)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	if (!pObj->IsEquipCustomItem(nItemID))
		return;

	pStage->OnNotifyThrowTrapItem(uidPlayer, nItemID);
}

void MMatchServer::OnNotifyActivatedTrapItem(const MUID& uidPlayer, const int nItemID, const MVector3& pos)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	pStage->OnNotifyActivatedTrapItem(uidPlayer, nItemID, pos);
}

float MMatchServer::GetDuelVictoryMultiflier(int nVictorty)
{
	return 1.0f;
}

float MMatchServer::GetDuelPlayersMultiflier(int nPlayerCount)
{
	return 1.0f;
}

void MMatchServer::CalcExpOnGameKill(MMatchStage* pStage, MMatchObject* pAttacker, MMatchObject* pVictim, 
					   int* poutAttackerExp, int* poutVictimExp)
{
	bool bSuicide = false;		// ¿⁄ªÏ
	if (pAttacker == pVictim) bSuicide = true;		

	MMATCH_GAMETYPE nGameType = pStage->GetStageSetting()->GetGameType();
	float fGameExpRatio = MGetGameTypeMgr()->GetInfo(nGameType)->fGameExpRatio;

	if(pStage->GetStageSetting()->GetStageSetting()->bTraining == true)
	{
		*poutAttackerExp = 0;
		*poutVictimExp = 0;
		return;
	}

	// ∞‘¿”≈∏¿‘¿Ã Training¿Ã∏È πŸ∑Œ 0∏Æ≈œ
	if (nGameType == MMATCH_GAMETYPE_TRAINING)
	{
		*poutAttackerExp = 0;
		*poutVictimExp = 0;
		return;
	}
	// ∞‘¿”≈∏¿‘¿Ã πˆº≠ƒø¿œ ∞ÊøÏ
	else if (nGameType == MMATCH_GAMETYPE_BERSERKER)
	{
		MMatchRuleBerserker* pRuleBerserker = (MMatchRuleBerserker*)pStage->GetRule();

		if (pRuleBerserker->GetBerserker() == pAttacker->GetUID())
		{
			if (pAttacker != pVictim)
			{
				// πˆº≠ƒø¥¬ ∞Ê«Ëƒ°∏¶ 80%∏∏ »πµÊ«—¥Ÿ.
				fGameExpRatio = fGameExpRatio * 0.8f;
			}
			else
			{
				// πˆº≠ƒø¥¬ ¿⁄ªÏ ∂«¥¬ ««∞° ¡ŸæÓ ¡◊¥¬∞ÊøÏ º’Ω« ∞Ê«Ëƒ°¥¬ æ¯µµ∑œ «—¥Ÿ.
				fGameExpRatio = 0.0f;
			}
		}
	}
	else if (nGameType == MMATCH_GAMETYPE_DUEL)
	{
		MMatchRuleDuel* pRuleDuel = (MMatchRuleDuel*)pStage->GetRule();
		if (pVictim->GetUID() == pRuleDuel->uidChallenger)
		{
			fGameExpRatio *= GetDuelVictoryMultiflier(pRuleDuel->GetVictory());
		}
		else
		{
			fGameExpRatio *= GetDuelVictoryMultiflier(pRuleDuel->GetVictory()) * GetDuelPlayersMultiflier(pStage->GetPlayers());

		}
//		if (pRuleDuel->GetVictory() <= 1)
//		{
//			fGameExpRatio = fGameExpRatio * GetDuelPlayersMultiflier(pStage->GetPlayers()) * GetDuelVictoryMultiflier()
//		}
	}
	else if (nGameType == MMATCH_GAMETYPE_CTF)//XXX
	{
			MMatchRuleTeamCTF* pRuleCTF = (MMatchRuleTeamCTF*)pStage->GetRule();
			if (pAttacker != pVictim)
			{
				// πˆº≠ƒø¥¬ ∞Ê«Ëƒ°∏¶ 80%∏∏ »πµÊ«—¥Ÿ.
				if(!(pAttacker->GetTeam() == pVictim->GetTeam()) && (pVictim->GetUID() == pRuleCTF->GetBlueCarrier() || pVictim->GetUID() == pRuleCTF->GetRedCarrier()))
				fGameExpRatio = fGameExpRatio * 2;
			}
			else
			{
				// πˆº≠ƒø¥¬ ¿⁄ªÏ ∂«¥¬ ««∞° ¡ŸæÓ ¡◊¥¬∞ÊøÏ º’Ω« ∞Ê«Ëƒ°¥¬ æ¯µµ∑œ «—¥Ÿ.
				fGameExpRatio = 0.0f;
			}
	}

	// ∏ , ∞‘¿”≈∏¿‘ø° ¥Î«— ∞Ê«Ëƒ° ∫Ò¿≤ ¿˚øÎ
	int nMapIndex = pStage->GetStageSetting()->GetMapIndex();
	if ((nMapIndex >=0) && (nMapIndex < MMATCH_MAP_COUNT))
	{
		float fMapExpRatio = MGetMapDescMgr()->GetExpRatio(nMapIndex);
		fGameExpRatio = fGameExpRatio * fMapExpRatio;
	}

	int nAttackerLevel = pAttacker->GetCharInfo()->m_nLevel;
	int nVictimLevel = pVictim->GetCharInfo()->m_nLevel;

	// ∞Ê«Ëƒ° ∞ËªÍ
	int nAttackerExp = (int)(MMatchFormula::GetGettingExp(nAttackerLevel, nVictimLevel) * fGameExpRatio);
	int nVictimExp = (int)(MMatchFormula::CalcPanaltyEXP(nAttackerLevel, nVictimLevel) * fGameExpRatio);


	// ≈¨∑£¿¸¿œ ∞ÊøÏ¥¬ »πµÊ ∞Ê«Ëƒ°∞° 1.5πË, º’Ω«∞Ê«Ëƒ° æ¯¿Ω
	if ((MGetServerConfig()->GetServerMode() == MSM_CLAN) && (pStage->GetStageType() == MST_LADDER))
	{
		nAttackerExp = (int)((float)nAttackerExp * 1.5f);
		nVictimExp = 0;
	}

	// ∞Ìºˆ√§≥Œ, √ ∞Ìºˆ√§≥Œ¿œ ∞ÊøÏø°¥¬ ∞Êƒ°¥ŸøÓ æ¯¿Ω(¿⁄ªÏ¡¶ø‹)
	MMatchChannel* pOwnerChannel = FindChannel(pStage->GetOwnerChannel());
	if ((pOwnerChannel) && (!bSuicide))
	{
		if ((pOwnerChannel->GetRuleType() == MCHANNEL_RULE_MASTERY) || 
			(pOwnerChannel->GetRuleType() == MCHANNEL_RULE_ELITE) ||
			(pOwnerChannel->GetRuleType() == MCHANNEL_RULE_CHAMPION))
		{
			nVictimExp=0;
		}
	}

	// ¡◊¿∫ªÁ∂˜¿Ã øÓøµ¿⁄, ∞≥πﬂ¿⁄¿œ ∞ÊøÏ ∞Ê«Ëƒ° µŒπË
	if ((pVictim->GetAccountInfo()->m_nUGrade == MMUG_ADMIN) || 
		(pVictim->GetAccountInfo()->m_nUGrade == MMUG_DEVELOPER))
	{
		nAttackerExp = nAttackerExp * 2;
	}
	// ¡◊¿ŒªÁ∂˜¿Ã øÓøµ¿⁄, ∞≥πﬂ¿⁄¿œ ∞ÊøÏ ∞Êƒ°¥ŸøÓ æ¯¿Ω
	if ((!bSuicide) &&
		((pAttacker->GetAccountInfo()->m_nUGrade == MMUG_ADMIN) || 
		(pAttacker->GetAccountInfo()->m_nUGrade == MMUG_DEVELOPER)))
	{
		nVictimExp = 0;
	}

	// ¿⁄ªÏ¿œ ∞ÊøÏ ∞Ê«Ëƒ° º’Ω«¿Ã µŒπË
	if (bSuicide) 
	{
		nVictimExp = (int)(MMatchFormula::GetSuicidePanaltyEXP(nVictimLevel) * fGameExpRatio);
		nAttackerExp = 0;
	}

	// ∆¿≈≥¿Œ∞ÊøÏ ∞Ê«Ëƒ° ¡¶∑Œ
	if ((pStage->GetStageSetting()->IsTeamPlay()) && (pAttacker->GetTeam() == pVictim->GetTeam()))
	{
		nAttackerExp = 0;
	}


	// ∆¿¿¸¿œ ∞ÊøÏ ∞Ê«Ëƒ° πË∫–
	if (pStage->IsApplyTeamBonus())
	{
		int nTeamBonus = 0;
		if (pStage->GetRule() != NULL)
		{
			int nNewAttackerExp = nAttackerExp;
			pStage->GetRule()->CalcTeamBonus(pAttacker, pVictim, nAttackerExp, &nNewAttackerExp, &nTeamBonus);
			nAttackerExp = nNewAttackerExp;
		}

		// ∆¿ ∞Ê«Ëƒ° ¿˚∏≥
		pStage->AddTeamBonus(nTeamBonus, MMatchTeam(pAttacker->GetTeam()));
	}

	// xp ∫∏≥ Ω∫ ¿˚øÎ(≥›∏∂∫Ì PCπÊ, ∞Ê«Ëƒ° π›¡ˆ)
	int nAttackerExpBonus = 0;
	if (nAttackerExp != 0)
	{
		//const float ta = float(atoi("15")) / 100.0f;
		//mlog( "test float : %f\n", ta * 100.0f );

		//MMatchItemBonusType nBonusType			= GetStageBonusType(pStage->GetStageSetting());
		//const double		dAttackerExp		= static_cast< double >( nAttackerExp );
		//const double		dXPBonusRatio		= static_cast< double >( MMatchFormula::CalcXPBonusRatio(pAttacker, nBonusType) );
		//const double		dAttackerExpBouns	= dAttackerExp * dXPBonusRatio;
		//const double		dSumAttackerExp		= dAttackerExp + dAttackerExpBouns;
		//
		//
		//nAttackerExpBonus = static_cast< int >( dAttackerExpBouns + 0.00001); 

		MMatchItemBonusType nBonusType = GetStageBonusType(pStage->GetStageSetting());
		const float fAttackerExpBonusRatio = MMatchFormula::CalcXPBonusRatio(pAttacker, nBonusType);
		 //∫Œµøº“ºˆ¡° ø¿¬˜∂ßπÆø° ∞ËªÍø° øµ«‚¿ª ¡÷¡ˆ æ ¥¬ π¸¿ßø°º≠ ∫∏¡§¿ª «ÿ¡ÿ¥Ÿ.
		// ∏∏æ‡ ¿Ã∫Œ∫–ø°º≠ ¥ŸΩ√ πÆ¡¶∞° πﬂª˝«—¥Ÿ∏È ∫∏¡§¿Ã æ∆¥— ∫£¿ÃΩ∫∫Œ≈Õ ºˆ¡§ ¿€æ˜¿ª «ÿ ¡‡æﬂ «—¥Ÿ.
		 nAttackerExpBonus = (int)(nAttackerExp * (fAttackerExpBonusRatio + 0.00001f));
	}

	*poutAttackerExp = nAttackerExp + nAttackerExpBonus;

	*poutVictimExp = nVictimExp;
}


const int MMatchServer::CalcBPonGameKill( MMatchStage* pStage, MMatchObject* pAttacker, const int nAttackerLevel, const int nVictimLevel )
{
	if( (0 == pStage) || (0 == pAttacker) ) 
		return -1;

	const int	nAddedBP		= static_cast< int >( MMatchFormula::GetGettingBounty(nAttackerLevel, nVictimLevel) );
	const float fBPBonusRatio	= MMatchFormula::CalcBPBounsRatio( pAttacker, GetStageBonusType(pStage->GetStageSetting()) );
	const int	nBPBonus		= static_cast< int >( nAddedBP * fBPBonusRatio );

	return nAddedBP + nBPBonus;
}




void MMatchServer::ProcessPlayerXPBP(MMatchStage* pStage, MMatchObject* pPlayer, int nAddedXP, int nAddedBP)
{
	if (pStage == NULL) return;
	if (!IsEnabledObject(pPlayer)) return;

	/*
		∞Ê«Ëƒ° ∞ËªÍ
		ƒ≥∏Ø≈Õø° ∞Ê«Ëƒ° ¿˚øÎ
		∑π∫ß ∞ËªÍ
		DBƒ≥ΩÃ æ˜µ•¿Ã∆Æ
		∑π∫ßæ˜,¥ŸøÓ ∏ﬁºº¡ˆ ¿¸º€
	*/

	MUID uidStage = pPlayer->GetStageUID();
	int nPlayerLevel = pPlayer->GetCharInfo()->m_nLevel;

	// ƒ≥∏Ø≈Õ XP æ˜µ•¿Ã∆Æ
	pPlayer->GetCharInfo()->IncXP(nAddedXP);

	// ∑π∫ß ∞ËªÍ
	int nNewPlayerLevel = -1;
	if ((pPlayer->GetCharInfo()->m_nLevel < MAX_LEVEL) &&
		(pPlayer->GetCharInfo()->m_nXP >= MMatchFormula::GetNeedExp(nPlayerLevel)))
	{
		nNewPlayerLevel = MMatchFormula::GetLevelFromExp(pPlayer->GetCharInfo()->m_nXP);
		if (nNewPlayerLevel != pPlayer->GetCharInfo()->m_nLevel) pPlayer->GetCharInfo()->m_nLevel = nNewPlayerLevel;
	}

	// πŸøÓ∆º √ﬂ∞°«ÿ¡ÿ¥Ÿ
	pPlayer->GetCharInfo()->IncBP(nAddedBP);


	// DB ƒ≥Ω≥ æ˜µ•¿Ã∆Æ
	if (pPlayer->GetCharInfo()->GetDBCachingData()->IsRequestUpdate()) {
		UpdateCharDBCachingData(pPlayer);		///< XP, BP, KillCount, DeathCount ƒ≥Ω≥ æ˜µ•¿Ã∆Æ
	}

	// ∏∏æ‡ ∑π∫ß¿Ã πŸ≤Ó∏È µ˚∑Œ ∑π∫ßæ˜«—¥Ÿ.
	if ((nNewPlayerLevel >= 0) && (nNewPlayerLevel != nPlayerLevel))
	{
		// ∑π∫ß¿Ã πŸ≤Ó∏È πŸ∑Œ ƒ≥Ω≥ æ˜µ•¿Ã∆Æ«—¥Ÿ
		UpdateCharDBCachingData(pPlayer);

		pPlayer->GetCharInfo()->m_nLevel = nNewPlayerLevel;
		if (!m_MatchDBMgr.UpdateCharLevel(pPlayer->GetCharInfo()->m_nCID, 
										  nNewPlayerLevel, 
										  pPlayer->GetCharInfo()->m_nBP,
										  pPlayer->GetCharInfo()->m_nTotalKillCount, 
										  pPlayer->GetCharInfo()->m_nTotalDeathCount,
										  pPlayer->GetCharInfo()->m_nTotalPlayTimeSec,
										  true))
		{
			mlog("DB UpdateCharLevel Error : %s\n", pPlayer->GetCharInfo()->m_szName);
		}
	}

	// ∑π∫ßæ˜, ∑π∫ß ¥ŸøÓ ∏ﬁºº¡ˆ ∫∏≥ª±‚
	if (nNewPlayerLevel > 0)
	{
		if (nNewPlayerLevel > nPlayerLevel)
		{
			MCommand* pCmd = CreateCommand(MC_MATCH_GAME_LEVEL_UP, MUID(0,0));
			pCmd->AddParameter(new MCommandParameterUID(pPlayer->GetUID()));
			pCmd->AddParameter(new MCommandParameterInt(nNewPlayerLevel));
			RouteToBattle(uidStage, pCmd);	
		}
		else if (nNewPlayerLevel < nPlayerLevel)
		{
			MCommand* pCmd = CreateCommand(MC_MATCH_GAME_LEVEL_DOWN, MUID(0,0));
			pCmd->AddParameter(new MCommandParameterUID(pPlayer->GetUID()));
			pCmd->AddParameter(new MCommandParameterInt(nNewPlayerLevel));
			RouteToBattle(uidStage, pCmd);	
		}
	}
}

// ∆¿ ∫∏≥ Ω∫ ¿˚øÎ
void MMatchServer::ApplyObjectTeamBonus(MMatchObject* pObject, int nAddedExp)
{
	if (!IsEnabledObject(pObject)) return;
	if (nAddedExp <= 0)
	{
		//_ASSERT(0);
		return;
	}
	
	bool bIsLevelUp = false;

	// ∫∏≥ Ω∫ ¿˚øÎ
	if (nAddedExp != 0)
	{
		int nExpBonus = (int)(nAddedExp * MMatchFormula::CalcXPBonusRatio(pObject, MIBT_TEAM));
		nAddedExp += nExpBonus;
	}




	// ƒ≥∏Ø≈Õ XP æ˜µ•¿Ã∆Æ
	pObject->GetCharInfo()->IncXP(nAddedExp);

	// ∑π∫ß ∞ËªÍ
	int nNewLevel = -1;
	int nCurrLevel = pObject->GetCharInfo()->m_nLevel;

	if (nNewLevel > nCurrLevel) bIsLevelUp = true;

	if ((pObject->GetCharInfo()->m_nLevel < MAX_LEVEL) &&
		(pObject->GetCharInfo()->m_nXP >= MMatchFormula::GetNeedExp(nCurrLevel)))
	{
		nNewLevel = MMatchFormula::GetLevelFromExp(pObject->GetCharInfo()->m_nXP);
		if (nNewLevel != nCurrLevel) pObject->GetCharInfo()->m_nLevel = nNewLevel;
	}

	// DB ƒ≥Ω≥ æ˜µ•¿Ã∆Æ
	if (pObject->GetCharInfo()->GetDBCachingData()->IsRequestUpdate())
	{
		UpdateCharDBCachingData(pObject);
	}

	// ∏∏æ‡ ∑π∫ß¿Ã πŸ≤Ó∏È πŸ∑Œ ∑π∫ßæ˜«—¥Ÿ.
	if ((nNewLevel >= 0) && (nNewLevel != nCurrLevel))
	{
		// ∑π∫ß¿Ã πŸ≤Ó∏È πŸ∑Œ ƒ≥Ω≥ æ˜µ•¿Ã∆Æ«—¥Ÿ
		UpdateCharDBCachingData(pObject);

		pObject->GetCharInfo()->m_nLevel = nNewLevel;
		nCurrLevel = nNewLevel;

		if (!m_MatchDBMgr.UpdateCharLevel(pObject->GetCharInfo()->m_nCID, 
			                              nNewLevel,
										  pObject->GetCharInfo()->m_nBP,
										  pObject->GetCharInfo()->m_nTotalKillCount,
										  pObject->GetCharInfo()->m_nTotalDeathCount,
										  pObject->GetCharInfo()->m_nTotalPlayTimeSec,
										  bIsLevelUp
										  ))
		{
			mlog("DB UpdateCharLevel Error : %s\n", pObject->GetCharInfo()->m_szName);
		}
	}


	MUID uidStage = pObject->GetStageUID();

	unsigned long int nExpArg;
	unsigned long int nChrExp;
	int nPercent;

	nChrExp = pObject->GetCharInfo()->m_nXP;
	nPercent = MMatchFormula::GetLevelPercent(nChrExp, nCurrLevel);
	// ªÛ¿ß 2πŸ¿Ã∆Æ¥¬ ∞Ê«Ëƒ°, «œ¿ß 2πŸ¿Ã∆Æ¥¬ ∞Ê«Ëƒ°¿« ∆€ºæ∆Æ¿Ã¥Ÿ.
	nExpArg = MakeExpTransData(nAddedExp, nPercent);


	MCommand* pCmd = CreateCommand(MC_MATCH_GAME_TEAMBONUS, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(pObject->GetUID()));
	pCmd->AddParameter(new MCommandParameterUInt(nExpArg));
	RouteToBattle(uidStage, pCmd);	


	// ∑π∫ßæ˜ ∏ﬁºº¡ˆ ∫∏≥ª±‚
	if ((nNewLevel >= 0) && (nNewLevel > nCurrLevel))
	{
		MCommand* pCmd = CreateCommand(MC_MATCH_GAME_LEVEL_UP, MUID(0,0));
		pCmd->AddParameter(new MCommandParameterUID(pObject->GetUID()));
		pCmd->AddParameter(new MCommandParameterInt(nNewLevel));
		RouteToBattle(uidStage, pCmd);	
	}
}

// «√∑π¿Ã ¡ﬂ ƒ≥∏Ø≈Õ ¡§∫∏ æ˜µ•¿Ã∆Æ
void MMatchServer::ProcessCharPlayInfo(MMatchObject* pPlayer)
{
	if (!IsEnabledObject(pPlayer)) return;

	/*
	ø¯«“∂ß∏∂¥Ÿ ƒ≥∏Ø≈Õ ¡§∫∏∏¶ æ˜µ•¿Ã∆Æ ¿˚øÎ
	∞Ê«Ëƒ° ∞ËªÍ
	ƒ≥∏Ø≈Õø° ∞Ê«Ëƒ° ¿˚øÎ
	∑π∫ß ∞ËªÍ
	∑π∫ßæ˜,¥ŸøÓ ∏ﬁºº¡ˆ ¿¸º€
	πŸøÓ∆º √ﬂ∞°«ÿ¡ÿ¥Ÿ
	¡¢º”Ω√∞£, ∞‘¿” ¡¯«‡Ω√∞£, «√∑π¿Ã Ω√∞£
	*/

	MUID uidStage = pPlayer->GetStageUID();
	int nPlayerLevel = pPlayer->GetCharInfo()->m_nLevel;

	// ∑π∫ß ∞ËªÍ
	int nNewPlayerLevel = -1;
	if ((pPlayer->GetCharInfo()->m_nLevel < MAX_LEVEL) &&
		(pPlayer->GetCharInfo()->m_nXP >= MMatchFormula::GetNeedExp(nPlayerLevel)))
	{
		nNewPlayerLevel = MMatchFormula::GetLevelFromExp(pPlayer->GetCharInfo()->m_nXP);
		if (nNewPlayerLevel != pPlayer->GetCharInfo()->m_nLevel) pPlayer->GetCharInfo()->m_nLevel = nNewPlayerLevel;
	}
	// ∏∏æ‡ ∑π∫ß¿Ã πŸ≤Ó∏È µ˚∑Œ ∑π∫ßæ˜«—¥Ÿ.
	if ((nNewPlayerLevel >= 0) && (nNewPlayerLevel != nPlayerLevel))
		pPlayer->GetCharInfo()->m_nLevel = nNewPlayerLevel;

	// ¡¢º”Ω√∞£, ∞‘¿” ¡¯«‡Ω√∞£, «√∑π¿Ã Ω√∞£
	unsigned long int nNowTime = MMatchServer::GetInstance()->GetGlobalClockCount();
	unsigned long int nBattlePlayingTimeSec = 0;
	if(pPlayer->GetCharInfo()->m_nBattleStartTime != 0)
	{
		nBattlePlayingTimeSec = MGetTimeDistance(pPlayer->GetCharInfo()->m_nBattleStartTime, nNowTime) / 1000;	// πË∆≤¿ª ¡¯«‡«— Ω√∞£
		
		/*
		// æ∆π´ √≥∏Æµµ «œ¡ˆ æ ¥¬µ•, ø÷ ∑Œ±◊¥¬ ≥≤±‚¥¬∞«∞°ø‰? ¿œ¥‹ ¡÷ºÆ √≥∏Æ«’¥œ¥Ÿ. - carrot318
		if(nBattlePlayingTimeSec > 60*60)
		{// ¿ÃªÛ¿˚¿∏∑Œ ∞™¿Ã ºº∆√µ≈∏È ∑Œ±◊∏¶ ≥≤±‰¥Ÿ.
			CTime theTime = CTime::GetCurrentTime();
			CString szTime = theTime.Format( "[%c] " );

			// ∞‘¿” ∏µÂ
			char buf[64]={0,};
			MMatchStage* pStage = FindStage(uidStage);

			if( pStage != NULL )
			{
				switch((int)pStage->GetStageSetting()->GetGameType())
				{
				case MMATCH_GAMETYPE_DEATHMATCH_SOLO:	{sprintf(buf, "DEATHMATCH_SOLO");	} break;		///< ∞≥¿Œ µ•æ≤∏≈ƒ°
				case MMATCH_GAMETYPE_DEATHMATCH_TEAM:	{sprintf(buf, "DEATHMATCH_TEAM");	} break;		///< ∆¿ µ•æ≤∏≈ƒ°
				case MMATCH_GAMETYPE_GLADIATOR_SOLO:	{sprintf(buf, "GLADIATOR_SOLO");	} break;		///< ∞≥¿Œ ±€∑°µø°¿Ã≈Õ
				case MMATCH_GAMETYPE_GLADIATOR_TEAM:	{sprintf(buf, "GLADIATOR_TEAM");	} break;		///< ∆¿ ±€∑°µø°¿Ã≈Õ
				case MMATCH_GAMETYPE_ASSASSINATE:		{sprintf(buf, "ASSASSINATE");		} break;		///< ∫∏Ω∫¿¸
				case MMATCH_GAMETYPE_TRAINING:			{sprintf(buf, "TRAINING");			} break;		///< ø¨Ω¿

				case MMATCH_GAMETYPE_SURVIVAL:			{sprintf(buf, "SURVIVAL");			} break;		///< º≠πŸ¿Ãπ˙
				case MMATCH_GAMETYPE_QUEST:				{sprintf(buf, "QUEST");				} break;		///< ƒ˘Ω∫∆Æ

				case MMATCH_GAMETYPE_BERSERKER:			{sprintf(buf, "BERSERKER");			} break;		
				case MMATCH_GAMETYPE_DEATHMATCH_TEAM2:	{sprintf(buf, "DEATHMATCH_TEAM2");	} break;		
				case MMATCH_GAMETYPE_DUEL:				{sprintf(buf, "DUEL");				} break;	
				default:								{sprintf(buf, "don't know");		} break;
				}
				mlog("%s BattlePlayT Error GameMode:%s, CID:%d, Name:%s, ServerCurrT:%u, BattleStartT:%u, PlayT:%d, PlayerConnectT:%u \n"
					, szTime, buf, pPlayer->GetCharInfo()->m_nCID, pPlayer->GetCharInfo()->m_szName, nNowTime, pPlayer->GetCharInfo()->m_nBattleStartTime, nBattlePlayingTimeSec, pPlayer->GetCharInfo()->m_nConnTime);
			}
		}
		*/
		//pPlayer->GetCharInfo()->m_nBattleStartTime = 0;
	}
	unsigned long int nLoginTotalTimeSec = MGetTimeDistance(pPlayer->GetCharInfo()->m_nConnTime, nNowTime) / 1000;	// ∞‘¿”¿ª ¡¯«‡«— Ω√∞£

	// ¿ÃªÛ¿˚¿∏∑Œ ∞Ê«Ëƒ°∞° »πµÊ«ﬂ¿∏∏È ∑Œ±◊∏¶ ≥≤∞‹¡ÿ¥Ÿ.
	// æ∆π´ √≥∏Æµµ «œ¡ˆ æ ¥¬µ•, ø÷ ∑Œ±◊¥¬ ≥≤±‚¥¬∞«∞°ø‰? ¿œ¥‹ ¡÷ºÆ √≥∏Æ«’¥œ¥Ÿ. - carrot318
	/*
	long int nBattleEXPGained = pPlayer->GetCharInfo()->m_nXP - pPlayer->GetCharInfo()->m_nBattleStartXP;
	if(nBattleEXPGained < -150000 || 150000 < nBattleEXPGained)
	{
		CTime theTime = CTime::GetCurrentTime();
		CString szTime = theTime.Format( "[%c] " );
		mlog("%s BattleXPGained Error CID:%d, Name:%s, StartXP:%d, EXPGained:%d \n", szTime, pPlayer->GetCharInfo()->m_nCID, pPlayer->GetCharInfo()->m_szName, pPlayer->GetCharInfo()->m_nBattleStartXP, nBattleEXPGained);
	}
	*/

#ifdef LOCALE_NHNUSA
	if (!m_MatchDBMgr.UpdateCharPlayInfo(pPlayer->GetAccountInfo()->m_nAID
										, pPlayer->GetCharInfo()->m_nCID
										, pPlayer->GetCharInfo()->m_nXP
										, pPlayer->GetCharInfo()->m_nLevel
										, nBattlePlayingTimeSec										// πË∆≤ Ω√∞£
										, nLoginTotalTimeSec										// √—∞‘¿”¿ª ¡¯«‡«— Ω√∞£
										, pPlayer->GetCharInfo()->m_nTotalKillCount
										, pPlayer->GetCharInfo()->m_nTotalDeathCount
										, pPlayer->GetCharInfo()->m_nBP
										, false))
	{
		mlog("DB UpdateCharPlayInfo Error : %s\n", pPlayer->GetCharInfo()->m_szName);
	}
#endif

}

void MMatchServer::PostGameDeadOnGameKill(MUID& uidStage, MMatchObject* pAttacker, MMatchObject* pVictim,
									int nAddedAttackerExp, int nSubedVictimExp)
{
	unsigned long int nAttackerArg = 0;
	unsigned long int nVictimArg =0;

	int nRealAttackerLevel = pAttacker->GetCharInfo()->m_nLevel;
	int nRealVictimLevel = pVictim->GetCharInfo()->m_nLevel;

	unsigned long int nChrExp;
	int nPercent;

	nChrExp = pAttacker->GetCharInfo()->m_nXP;
	nPercent = MMatchFormula::GetLevelPercent(nChrExp, nRealAttackerLevel);
	nAttackerArg = MakeExpTransData(nAddedAttackerExp, nPercent);

	nChrExp = pVictim->GetCharInfo()->m_nXP;
	nPercent = MMatchFormula::GetLevelPercent(nChrExp, nRealVictimLevel);
	nVictimArg = MakeExpTransData(nSubedVictimExp, nPercent);

	MCommand* pCmd = CreateCommand(MC_MATCH_GAME_DEAD, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(pAttacker->GetUID()));
	pCmd->AddParameter(new MCommandParameterUInt(nAttackerArg));
	pCmd->AddParameter(new MCommandParameterUID(pVictim->GetUID()));
	pCmd->AddParameter(new MCommandParameterUInt(nVictimArg));
	RouteToBattle(uidStage, pCmd);	
}

void MMatchServer::StageList(const MUID& uidPlayer, int nStageStartIndex, bool bCacheUpdate)
{
	MMatchObject* pChar = GetObject(uidPlayer);
	if (pChar == NULL) return;
	MMatchChannel* pChannel = FindChannel(pChar->GetChannelUID());
	if (pChannel == NULL) return;

	// ≈¨∑£º≠πˆ¿Œµ• ≈¨∑£√§≥Œ¿œ ∞ÊøÏø°¥¬ πÊ ∏ÆΩ∫∆Æ¥ÎΩ≈ ¥Î±‚¡ﬂ ≈¨∑£ ∏ÆΩ∫∆Æ∏¶ ∫∏≥Ω¥Ÿ.
	if ((MGetServerConfig()->GetServerMode() == MSM_CLAN) && (pChannel->GetChannelType() == MCHANNEL_TYPE_CLAN))
	{
		StandbyClanList(uidPlayer, nStageStartIndex, bCacheUpdate);
		return;
	}


	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_LIST), MUID(0,0), m_This);

	int nPrevStageCount = -1, nNextStageCount = -1;
	int nNextStageIndex = pChannel->GetMaxPlayers()-1;


	// 2008.09.16 
	int nRealStageStartIndex = nStageStartIndex;
	int nStageCount = 0;
	for(int i = 0; i < pChannel->GetMaxPlayers(); i++)
	{
		// πÊ¿Ã ∫Òøˆ¿’¿∏∏È √≥∏Ææ»«—¥Ÿ
		if (pChannel->IsEmptyStage(i)) continue;
		// πÊ¿Ã ¿÷¿∏∏È √≥∏Æ
		if(nStageCount < nStageStartIndex) // æ’ø° ≈«ø° √≥∏Æµ» πÊµÈ < «ˆ¿Á ≈«ø°º≠ Ω√¿€«“ πÊ Index
			nStageCount++;
		else
		{
			nRealStageStartIndex = i;
			break;
		}
	}

	int nRealStageCount = 0;
	for (int i = /*nStageStartIndex*/nRealStageStartIndex; i < pChannel->GetMaxPlayers(); i++)
	{
		if (pChannel->IsEmptyStage(i)) continue;

		MMatchStage* pStage = pChannel->GetStage(i);
		if ((pStage == NULL) || (pStage->GetState() == STAGE_STATE_CLOSE)) continue;

		nRealStageCount++;
		if (nRealStageCount >= TRANS_STAGELIST_NODE_COUNT) 
		{
			nNextStageIndex = i;
			break;
		}
	}

	if (!bCacheUpdate)
	{
		nPrevStageCount = pChannel->GetPrevStageCount(nStageStartIndex);
		nNextStageCount = pChannel->GetNextStageCount(nNextStageIndex);
	}

	pNew->AddParameter(new MCommandParameterChar((char)nPrevStageCount));
	pNew->AddParameter(new MCommandParameterChar((char)nNextStageCount));


	void* pStageArray = MMakeBlobArray(sizeof(MTD_StageListNode), nRealStageCount);
	int nArrayIndex=0;

	for (int i = /*nStageStartIndex*/nRealStageStartIndex; i < pChannel->GetMaxPlayers(); i++)
	{
		if (pChannel->IsEmptyStage(i)) continue;
		MMatchStage* pStage = pChannel->GetStage(i);
		if ((pStage == NULL) || (pStage->GetState() == STAGE_STATE_CLOSE)) continue;
		
		if( pStage->GetState() < STAGE_STATE_STANDBY || pStage->GetState() > STAGE_STATE_COUNT )
		{
			/* ≈©∑°Ω¨∑Œ ¿Œ«ÿ πÊæÓƒ⁄µÂ √ﬂ∞°. ≥™¡ﬂø° pChannel->m_pStages πËø≠¿Ã æ∆¥— ∏ ¿Ã≥™ ¥Ÿ∏•∞…∑Œ ∏Æ∆Â≈‰∏µ« ø‰*/
			LOG(LOG_FILE, "there is unavailable stages in %s channel. No:%d \n", pChannel->GetName(), i);
			continue;
		}


		if (nArrayIndex >= nRealStageCount) break;

		MTD_StageListNode* pNode = (MTD_StageListNode*)MGetBlobArrayElement(pStageArray, nArrayIndex++);
		pNode->uidStage = pStage->GetUID();
		strcpy(pNode->szStageName, pStage->GetName());
		pNode->nNo = (unsigned char)(pStage->GetIndex() + 1);	// ªÁøÎ¿⁄ø°∞‘ ∫∏ø©¡÷¥¬ ¿Œµ¶Ω∫¥¬ 1∫Œ≈Õ Ω√¿€«—¥Ÿ
		pNode->nPlayers = (char)pStage->GetPlayers();
		pNode->nMaxPlayers = pStage->GetStageSetting()->GetMaxPlayers();
		pNode->nState = pStage->GetState();
		pNode->nGameType = pStage->GetStageSetting()->GetGameType();
		
		// ∏±∑π¿Ã∏È ∑Œ∫Ò πÊ∏ÆΩ∫∆Æ πË≥ ∏¶ ∏±∑π¿Ã∏ ¿∏∑Œ ¿Ø¡ˆ«ÿ¡ÿ¥Ÿ.
		if(pStage->IsRelayMap()) pNode->nMapIndex = MMATCH_MAP_RELAYMAP;
		else		 			 pNode->nMapIndex = pStage->GetStageSetting()->GetMapIndex();
		
		pNode->nSettingFlag = 0;
		// ≥≠¿‘
		if (pStage->GetStageSetting()->GetForcedEntry())
		{
			pNode->nSettingFlag |= MSTAGENODE_FLAG_FORCEDENTRY_ENABLED;
		}
		// ∫Òπ–πÊ
		if (pStage->IsPrivate())
		{
			pNode->nSettingFlag |= MSTAGENODE_FLAG_PRIVATE;
		}
		// ∑π∫ß¡¶«—
		pNode->nLimitLevel = pStage->GetStageSetting()->GetLimitLevel();
		pNode->nMasterLevel = 0;

		if (pNode->nLimitLevel != 0)
		{
			pNode->nSettingFlag |= MSTAGENODE_FLAG_LIMITLEVEL;

			;
			MMatchObject* pMaster = GetObject(pStage->GetMasterUID());
			if (pMaster)
			{
				if (pMaster->GetCharInfo())
				{
					pNode->nMasterLevel = pMaster->GetCharInfo()->m_nLevel;
				}
			}
		}
	}

	pNew->AddParameter(new MCommandParameterBlob(pStageArray, MGetBlobArraySize(pStageArray)));
	MEraseBlobArray(pStageArray);

	RouteToListener(pChar, pNew);	
}


void MMatchServer::OnStageRequestStageList(const MUID& uidPlayer, const MUID& uidChannel, const int nStageCursor)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) return;

	pObj->SetStageCursor(nStageCursor);
	StageList(pObj->GetUID(), nStageCursor, false);
}


void MMatchServer::OnRequestQuickJoin(const MUID& uidPlayer, void* pQuickJoinBlob)
{
	MTD_QuickJoinParam* pNode = (MTD_QuickJoinParam*)MGetBlobArrayElement(pQuickJoinBlob, 0);
	ResponseQuickJoin(uidPlayer, pNode);
}

void MMatchServer::ResponseQuickJoin(const MUID& uidPlayer, MTD_QuickJoinParam* pQuickJoinParam)
{
	if (pQuickJoinParam == NULL) return;

	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;
	MMatchChannel* pChannel = FindChannel(pObj->GetChannelUID());
	if (pChannel == NULL) return;

	list<MUID>	recommended_stage_list;
	MUID uidRecommendedStage = MUID(0,0);
	int nQuickJoinResult = MOK;


	for (int i = 0; i < pChannel->GetMaxStages(); i++)
	{
		if (pChannel->IsEmptyStage(i)) continue;
		MMatchStage* pStage = pChannel->GetStage(i);
		if ((pStage == NULL) || (pStage->GetState() == STAGE_STATE_CLOSE)) continue;

		int ret = ValidateStageJoin(pObj->GetUID(), pStage->GetUID());
		if (ret == MOK)
		{
			if (pStage->IsPrivate()) continue;

			int nMapIndex = pStage->GetStageSetting()->GetMapIndex();
			int nGameType = pStage->GetStageSetting()->GetGameType();

			if (!CheckBitSet(pQuickJoinParam->nMapEnum, nMapIndex)) continue;
			if (!CheckBitSet(pQuickJoinParam->nModeEnum, nGameType)) continue;

			//if (((1 << nMapIndex) & (pQuickJoinParam->nMapEnum)) == 0) continue;
			//if (((1 << nGameType) & (pQuickJoinParam->nModeEnum)) == 0) continue;

			recommended_stage_list.push_back(pStage->GetUID());
		}
	}

	if (!recommended_stage_list.empty())
	{
		int nSize=(int)recommended_stage_list.size();
		int nIndex = rand() % nSize;

		int nCnt = 0;
		for (list<MUID>::iterator itor = recommended_stage_list.begin(); itor != recommended_stage_list.end(); ++itor)
		{
			if (nIndex == nCnt)
			{
				uidRecommendedStage = (*itor);
				break;
			}
			nCnt++;
		}
	}
	else
	{
		nQuickJoinResult = MERR_CANNOT_NO_STAGE;
	}

	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_RESPONSE_QUICKJOIN, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterInt(nQuickJoinResult));
	pCmd->AddParameter(new MCommandParameterUID(uidRecommendedStage));
	RouteToListener(pObj, pCmd);	
}

static int __cdecl _int_sortfunc(const void* a, const void* b)
{
	return *((int*)a) - *((int*)b);
}


int MMatchServer::GetLadderTeamIDFromDB(const int nTeamTableIndex, const int* pnMemberCIDArray, const int nMemberCount)
{
	if ((nMemberCount <= 0) || (nTeamTableIndex != nMemberCount))
	{
		//_ASSERT(0);
		return 0;
	}

	// cid ø¿∏ß¬˜º¯¿∏∑Œ º“∆√ - dbªÛø° º“∆√µ«æÓ µÈæÓ∞°¿÷¥Ÿ. 
	int* pnSortedCIDs = new int[nMemberCount];
	for (int i = 0; i < nMemberCount; i++)
	{
		pnSortedCIDs[i] = pnMemberCIDArray[i];
	}
	qsort(pnSortedCIDs, nMemberCount, sizeof(int), _int_sortfunc);

	int nTID = 0;
	if (pnSortedCIDs[0] != 0)
	{
		if (!m_MatchDBMgr.GetLadderTeamID(nTeamTableIndex, pnSortedCIDs, nMemberCount, &nTID))
		{
			nTID = 0;
		}
	}
	

	delete[] pnSortedCIDs;

	return nTID;
}

void MMatchServer::SaveLadderTeamPointToDB(const int nTeamTableIndex, const int nWinnerTeamID, const int nLoserTeamID, const bool bIsDrawGame)
{
	// ∆˜¿Œ∆Æ ∞ËªÍ - æ◊º«∏Æ±◊ ¿¸øÎ
	int nWinnerPoint = 0, nLoserPoint = 0, nDrawPoint = 0;

	nLoserPoint = -1;
	switch (nTeamTableIndex)
	{
	case 2:	// 2¥Î2
		{
			nWinnerPoint = 4;
			nDrawPoint = 1;
		}
		break;
	case 3:
		{
			nWinnerPoint = 6;
			nDrawPoint = 1;
		}
		break;
	case 4:
		{
			nWinnerPoint = 10;
			nDrawPoint = 2;
		}
		break;
	}

	if (!m_MatchDBMgr.LadderTeamWinTheGame(nTeamTableIndex, nWinnerTeamID, nLoserTeamID, bIsDrawGame,
		                                   nWinnerPoint, nLoserPoint, nDrawPoint))
	{
		mlog("DB Query(SaveLadderTeamPointToDB) Failed\n");
	}
}


void MMatchServer::OnVoteCallVote(const MUID& uidPlayer, const char* pszDiscuss, const char* pszArg)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	// øÓøµ¿⁄∞° ∞≠≈≈ı«•«œ∏È ∞≠¡¶∑Œ ∞≠≈
	if (IsAdminGrade(pObj)) {
		MMatchStage* pStage = FindStage(pObj->GetStageUID());
		if (pStage)
			pStage->KickBanPlayer(pszArg, false);
		return;
	}

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	char szMsg[256];
	// øÓøµ¿⁄∞° ∞∞¿Ã ∞‘¿”¡ﬂ¿Ã∏È ≈ı«• ∫“∞°¥…
	for (MUIDRefCache::iterator itor = pStage->GetObjBegin(); itor != pStage->GetObjEnd(); itor++) {
		MUID uidObj = (MUID)(*itor).first;
		MMatchObject* pPlayer = (MMatchObject*)GetObject(uidObj);
		if ((pPlayer) && (IsAdminGrade(pPlayer)))
		{
			sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_CANNOT_VOTE);
			Announce(uidPlayer, szMsg);

			return;
		}
	}


	if( pObj->WasCallVote() )
	{
		sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_CANNOT_VOTE);
		Announce(uidPlayer, szMsg);

		return;
	}

	// ≈ı«•∏¶ «ﬂ¥Ÿ¥¬∞… «•Ω√«ÿ≥ı¿Ω.
	pObj->SetVoteState( true );

	if (pStage->GetStageType() == MST_LADDER)
	{
		sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_CANNOT_VOTE_LADERGAME);
		Announce(uidPlayer, szMsg);

		return;
	}

	if (pStage->GetRule() && pStage->GetRule()->GetGameType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
	{
		sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_CANNOT_VOTE);
		Announce(uidPlayer, szMsg);

		return;
	}
#ifdef _VOTESETTING
	// πÊ º≥¡§¡ﬂ ≈ı«•±‚¥…¿ª ∞ÀªÁ«‘.
	if( !pStage->GetStageSetting()->bVoteEnabled ) {
		VoteAbort( uidPlayer );
		return;
	}

	// ¿Ãπ¯ ∞‘¿”ø°º≠ ≈ı«•∏¶ ∞«¿««ﬂ¥¬¡ˆ ∞ÀªÁ.
	if( pStage->WasCallVote() ) {
		VoteAbort( uidPlayer );
		return;
	}
	else {
		pStage->SetVoteState( true );
	}
#endif

	if (pStage->GetVoteMgr()->GetDiscuss())
	{
		sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_VOTE_ALREADY_START);
		Announce(uidPlayer, szMsg);

		return;
	}

	MVoteDiscuss* pDiscuss = MVoteDiscussBuilder::Build(uidPlayer, pStage->GetUID(), pszDiscuss, pszArg);
	if (pDiscuss == NULL) return;

	if (pStage->GetVoteMgr()->CallVote(pDiscuss)) {
		pDiscuss->Vote(uidPlayer, MVOTE_YES);	// πﬂ¿«¿⁄ π´¡∂∞« ¬˘º∫

		MCommand* pCmd = CreateCommand(MC_MATCH_NOTIFY_CALLVOTE, MUID(0,0));
		pCmd->AddParameter(new MCmdParamStr(pszDiscuss));
		pCmd->AddParameter(new MCmdParamStr(pszArg));
		RouteToStage(pStage->GetUID(), pCmd);
		return;
	}
	else
	{
		sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_VOTE_FAILED);
		Announce(uidPlayer, szMsg);

		return;
	}
}

void MMatchServer::OnVoteYes(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	MVoteDiscuss* pDiscuss = pStage->GetVoteMgr()->GetDiscuss();
    if (pDiscuss == NULL) return;

	pDiscuss->Vote(uidPlayer, MVOTE_YES);
}

void MMatchServer::OnVoteNo(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	MVoteDiscuss* pDiscuss = pStage->GetVoteMgr()->GetDiscuss();
    if (pDiscuss == NULL) return;

	pDiscuss->Vote(uidPlayer, MVOTE_NO);
}

void MMatchServer::VoteAbort( const MUID& uidPlayer )
{
#ifndef MERR_CANNOT_VOTE
#define MERR_CANNOT_VOTE 120000
#endif

	MMatchObject* pObj = GetObject( uidPlayer );
	if( 0 == pObj )
		return;

	MCommand* pCmd = CreateCommand( MC_MATCH_VOTE_RESPONSE, MUID(0, 0) );
	if( 0 == pCmd )
		return;

	pCmd->AddParameter( new MCommandParameterInt(MERR_CANNOT_VOTE) );
	RouteToListener( pObj, pCmd );
}



void MMatchServer::OnEventChangeMaster(const MUID& uidAdmin)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if( 0 == pObj )
		return;

	if (!IsEnabledObject(pObj)) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	// ∞¸∏Æ¿⁄ ±««—¿ª ∞°¡¯ ªÁ∂˜¿Ã æ∆¥œ∏È ø¨∞·¿ª ≤˜¥¬¥Ÿ.
	if (!IsAdminGrade(pObj))
	{
//		DisconnectObject(uidAdmin);		
		return;
	}

	if (pStage->GetMasterUID() == uidAdmin)
		return;

	pStage->SetMasterUID(uidAdmin);
	StageMaster(pStage->GetUID());
}

void MMatchServer::OnEventChangePassword(const MUID& uidAdmin, const char* pszPassword)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if( 0 == pObj ) 
		return;

	if (!IsEnabledObject(pObj)) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	// ∞¸∏Æ¿⁄ ±««—¿ª ∞°¡¯ ªÁ∂˜¿Ã æ∆¥œ∏È ø¨∞·¿ª ≤˜¥¬¥Ÿ.
	if (!IsAdminGrade(pObj))
	{
//		DisconnectObject(uidAdmin);		
		return;
	}

	pStage->SetPassword(pszPassword);
	pStage->SetPrivate(true);
}

void MMatchServer::OnEventRequestJjang(const MUID& uidAdmin, const char* pszTargetName)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if( 0 == pObj )
		return;

	if (!IsEnabledObject(pObj)) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	// ∞¸∏Æ¿⁄ ±««—¿ª ∞°¡¯ ªÁ∂˜¿Ã æ∆¥œ∏È π´Ω√
	if (!IsAdminGrade(pObj))
	{
		return;
	}

	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
	if (pTargetObj == NULL) return;
	if (IsAdminGrade(pTargetObj)) return;		// æÓµÂπŒ ¥ÎªÛ¿∏∑Œ ¬Ø∫“∞°
	if (MMUG_STAR == pTargetObj->GetAccountInfo()->m_nUGrade) return;	// ¿ÃπÃ ¬Ø

	pTargetObj->GetAccountInfo()->m_nUGrade = MMUG_STAR;

	if (m_MatchDBMgr.EventJjangUpdate(pTargetObj->GetAccountInfo()->m_nAID, true)) {
		MMatchObjectCacheBuilder CacheBuilder;
		CacheBuilder.AddObject(pTargetObj);
		MCommand* pCmdCacheUpdate = CacheBuilder.GetResultCmd(MATCHCACHEMODE_REPLACE, this);
		RouteToStage(pStage->GetUID(), pCmdCacheUpdate);

		MCommand* pCmdUIUpdate = CreateCommand(MC_EVENT_UPDATE_JJANG, MUID(0,0));
		pCmdUIUpdate->AddParameter(new MCommandParameterUID(pTargetObj->GetUID()));
		pCmdUIUpdate->AddParameter(new MCommandParameterBool(true));
		RouteToStage(pStage->GetUID(), pCmdUIUpdate);
	}
}

void MMatchServer::OnEventRemoveJjang(const MUID& uidAdmin, const char* pszTargetName)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if( 0 == pObj )
		return;

	if (!IsEnabledObject(pObj)) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	// ∞¸∏Æ¿⁄ ±««—¿ª ∞°¡¯ ªÁ∂˜¿Ã æ∆¥œ∏È ø¨∞·¿ª ≤˜¥¬¥Ÿ.
	if (!IsAdminGrade(pObj))
	{
		return;
	}
	
	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
	if (pTargetObj == NULL) return;			// æÓµÂπŒ ¥ÎªÛ¿∏∑Œ ¬Ø∫“∞°

	pTargetObj->GetAccountInfo()->m_nUGrade = MMUG_FREE;

	if (m_MatchDBMgr.EventJjangUpdate(pTargetObj->GetAccountInfo()->m_nAID, false)) {
		MMatchObjectCacheBuilder CacheBuilder;
		CacheBuilder.AddObject(pTargetObj);
		MCommand* pCmdCacheUpdate = CacheBuilder.GetResultCmd(MATCHCACHEMODE_REPLACE, this);
		RouteToStage(pStage->GetUID(), pCmdCacheUpdate);

		MCommand* pCmdUIUpdate = CreateCommand(MC_EVENT_UPDATE_JJANG, MUID(0,0));
		pCmdUIUpdate->AddParameter(new MCommandParameterUID(pTargetObj->GetUID()));
		pCmdUIUpdate->AddParameter(new MCommandParameterBool(false));
		RouteToStage(pStage->GetUID(), pCmdUIUpdate);
	}
}


void MMatchServer::OnStageGo(const MUID& uidPlayer, unsigned int nRoomNo)
{
	MMatchObject* pChar = GetObject(uidPlayer);
	if( 0 == pChar ) return;
	if (!IsEnabledObject(pChar)) return;
	if (pChar->GetPlace() != MMP_LOBBY) return;
	MMatchChannel* pChannel = FindChannel(pChar->GetChannelUID());
	if (pChannel == NULL) return;

	MMatchStage* pStage = pChannel->GetStage(nRoomNo-1);
	if (pStage) {
		MCommand* pNew = CreateCommand(MC_MATCH_REQUEST_STAGE_JOIN, GetUID());
		pNew->SetSenderUID(uidPlayer);	// «√∑π¿ÃæÓ∞° ∫∏≥Ω ∏ﬁΩ√¡ˆ¿Œ ∞Õ√≥∑≥ ¿ß¿Â
		pNew->AddParameter(new MCommandParameterUID(uidPlayer));
		pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));
		Post(pNew);
	}
}



void MMatchServer::OnDuelQueueInfo(const MUID& uidStage, const MTD_DuelQueueInfo& QueueInfo)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_DUEL_QUEUEINFO, MUID(0,0));
	pCmd->AddParameter(new MCmdParamBlob(&QueueInfo, sizeof(MTD_DuelQueueInfo)));
	RouteToBattle(uidStage, pCmd);
}


void MMatchServer::OnQuestSendPing(const MUID& uidStage, unsigned long int t)
{
	MCommand* pCmd = CreateCommand(MC_QUEST_PING, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUInt(t));
	RouteToBattle(uidStage, pCmd);
}

void MMatchServer::SaveGameLog(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	int nMapID		= pStage->GetStageSetting()->GetMapIndex();
	int nGameType	= (int)pStage->GetStageSetting()->GetGameType();
	
	

	// test ∏ µÓ¿∫ ∑Œ±◊ ≥≤±‚¡ˆ æ ¥¬¥Ÿ.
	if ( (MGetMapDescMgr()->MIsCorrectMap(nMapID)) && (MGetGameTypeMgr()->IsCorrectGameType(nGameType)) )
	{
		if (pStage->GetStageType() != MST_LADDER)
		{
			MMatchObject* pMaster = GetObject(pStage->GetMasterUID());

			MAsyncDBJob_InsertGameLog* pJob = new MAsyncDBJob_InsertGameLog(uidStage);
			pJob->Input(pMaster == NULL ? 0 : pMaster->GetCharInfo()->m_nCID,
				MGetMapDescMgr()->GetMapName(nMapID), 
				MGetGameTypeMgr()->GetInfo(MMATCH_GAMETYPE(nGameType))->szGameTypeStr);
			PostAsyncJob(pJob);
		}
	}

}

void MMatchServer::SaveGamePlayerLog(MMatchObject* pObj, unsigned int nStageID)
{	
	if( pObj == NULL ) return;
	if( nStageID == 0 ) return;
	if( pObj->GetCharInfo() == NULL ) return;

	MAsyncDBJob_InsertGamePlayerLog* pJob = new MAsyncDBJob_InsertGamePlayerLog;
	pJob->Input(nStageID, pObj->GetCharInfo()->m_nCID,
		(GetGlobalClockCount() - pObj->GetCharInfo()->m_nBattleStartTime) / 1000,
		pObj->GetCharInfo()->GetCharGamePlayInfo()->nKillCount,
		pObj->GetCharInfo()->GetCharGamePlayInfo()->nDeathCount,
		pObj->GetCharInfo()->GetCharGamePlayInfo()->nXP,
		pObj->GetCharInfo()->GetCharGamePlayInfo()->nBP);
	PostAsyncJob(pJob);
}
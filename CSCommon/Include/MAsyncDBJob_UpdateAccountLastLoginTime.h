#pragma once



#include "MAsyncDBJob.h"



class MAsyncDBJob_UpdateAccountLastLoginTime : public MAsyncJob
{
public :
	MAsyncDBJob_UpdateAccountLastLoginTime(const MUID& uidOwner) : MAsyncJob( MASYNCJOB_UPDATEACCOUNTLASTLOGINTIME, uidOwner )
	{
	}

	~MAsyncDBJob_UpdateAccountLastLoginTime()
	{
	}

	void Input( const DWORD dwAID, unsigned long dwLastVIPWall, int nVIPWalls )
	{
		m_dwAID = dwAID;
		m_dwLastVIPWall = dwLastVIPWall;
		m_nVIPWalls = nVIPWalls;
	}

	void Run( void* pContext )
	{
		MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

		pDBMgr->UpdateAccountLastLoginTime( m_dwAID, m_dwLastVIPWall, m_nVIPWalls );
	}

private :
	DWORD m_dwAID;
	unsigned long m_dwLastVIPWall;
	int m_nVIPWalls;
};
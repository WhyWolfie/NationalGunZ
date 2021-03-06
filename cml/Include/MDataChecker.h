//// Memory Hack을 대비, 중요정보들의 Checksum을 추적한다. ////
#ifndef _MDATACHECKER_H
#define _MDATACHECKER_H

//#pragma once

// 아래 체크섬 클래스는 사용하지 않습니다.... crc32검사로 대체하였습니다.

#include <map>
using namespace std;


#ifndef BYTE
typedef unsigned char BYTE;
#endif

enum MEMORYFUGITIVE_TYPE {
	MT_MEMORYFUGITIVE_NONE,		// MMemoryFugitive 가 아닌 경우 (native type)
	MT_MEMORYFUGITIVE_INT,
	MT_MEMORYFUGITIVE_FLOAT,
	//.. 필요하면 타입을 더 추가
};

class MDataChecker;
class MDataCheckNode {
protected:
	unsigned int	m_nID;	// 디버깅용 식별자
	BYTE*			m_pData;
	unsigned int	m_nLen;
	unsigned int	m_nChecksum;
	unsigned int	m_nLastChecksum;
	MEMORYFUGITIVE_TYPE	m_memFugitiveType;

public:
	MDataCheckNode(BYTE* pData, unsigned int nLen, MEMORYFUGITIVE_TYPE memFugitiveType);
	virtual ~MDataCheckNode();

	unsigned int GetID()			{ return m_nID; }
	unsigned int GetChecksum()		{ return m_nChecksum; }
	unsigned int GetLastChecksum()	{ return m_nLastChecksum; }
	bool UpdateChecksum();	// 업데이트후 전과 같으면 true, 다르면 false
	void Validate()	{ m_nLastChecksum = m_nChecksum; }

friend MDataChecker;
};
class MDataCheckMap : public map<BYTE*, MDataCheckNode*>{};


class MDataChecker {
protected:
	unsigned int	m_nTotalChecksum;
	unsigned int	m_nLastTotalChecksum;

	MDataCheckMap	m_DataCheckMap;

public:
	MDataChecker();
	virtual ~MDataChecker();

	void Clear();
	unsigned int GetChecksum()	{ return m_nTotalChecksum; }
	MDataCheckNode* FindCheck(BYTE* pData);
	MDataCheckNode* AddCheck(BYTE* pData, unsigned int nLen, MEMORYFUGITIVE_TYPE memFugitiveType=MT_MEMORYFUGITIVE_NONE);
	void RenewCheck(BYTE* pData, unsigned int nLen);
	bool UpdateChecksum();
	void BringError();
};

#endif
#ifndef MPolygonMapModel_H
#define MPolygonMapModel_H

#include "CMList.h"
#include "CMPtrList.h"
#include "MAStar.h"
#include <list>
#include <map>

using namespace std;

/*
	¼¼±ª¸??®´? Right-Hand?? ¹???¼??» °¡??´?.

	   ->
	    /\
	   /  \
	  / in \
	 /      \ out
	+--------+
        <-
*/


/*
class MPMPoint;
typedef list<MPMPoint*>		MPNVList;
typedef MPNVList::iterator	MPNVListItor;
*/

class MPMPolygon;
class MPMPortal;
class MPolygonMapModel;

// °?»? °¡´??? Point Node
// ??¸®°??? °¢ ¸?¼­¸®°¡ °?»??? ³?µ?°¡ µ?´?.
class MPMPoint : public MNodeModel{
protected:
friend class MPMPolygon;
friend class MPMPortal;
friend class MPolygonMapModel;
	MPolygonMapModel*		m_pParentMap;
	float				m_fX, m_fY, m_fZ;
	CMPtrList<MPMPoint>	m_VisibleList;		// ?¸??´? ?¡ ¸®½??®
	int					m_nLastChangeCount;	// ??±?±??? ¹?¿µ?? µ? Map?? ChangeCount
	MPMPoint*			m_pPrevPoint;		// Polygon¿¡ ¼????»¶§ ???? ?º???®
	MPMPoint*			m_pNextPoint;		// Polygon¿¡ ¼????»¶§ ???? ?º???®
	CMPtrList<MPMPoint>	m_VisibledList;		// ???¡?? ?¸????´? ´?¸¥ ?¡ ¸®½??®
	void*				m_pTemp;			// ??½? ?º???? ( ?¸??¹??? ¾?´?´?. )

	//void*				m_pRoomIndex;
	list<int>			m_RoomIndexs;
	CMPtrList<MPMPortal>	m_InPortals;

protected:
	bool IsParentVisiblePoint(MPMPoint* p);	// Parent Point(Path¼³?¤??¿¡..)¿¡¼­ p°¡ ?¸??´?°¡ ?ª½??®
	void GenerateVisiblePoint(void);		// ?¸??´? ?¡ ?£±?
	void ResetVisibleList(void);
	void AddVisiblePoint(MPMPoint* pPoint);

public:
	MPMPoint(float x, float y, float z, int nRoomIndex);
	MPMPoint(float x, float y, float z, list<int>& RoomIndexs);
	MPMPoint(void);

	inline float GetX(void){ return m_fX; }
	inline float GetY(void){ return m_fY; }
	inline float GetZ(void){ return m_fZ; }

	MPMPoint* GetPrevPoint(void){ return m_pPrevPoint; }
	MPMPoint* GetNextPoint(void){ return m_pNextPoint; }

	int GetVisiblePointCount(void);
	MPMPoint* GetVisiblePoint(int i);

	void ResetConnectPoint(void);
	bool IsVisible(MPMPoint* pPN);

	virtual int GetSuccessorCount(void);
	virtual MNodeModel* GetSuccessor(int i);

	virtual float GetSuccessorCost(MNodeModel* pSuccessor);
	virtual float GetHeuristicCost(MNodeModel* pNode);

	/// ?? ³?µ?°¡ ¼??? Room??¿¡ °¡?? ???? ??µ¦½?
	int GetMinRoomIndex(void);
	/// ?? ³?µ?°¡ ¼??? Room??¿¡ °¡?? ?« ??µ¦½?
	int GetMaxRoomIndex(void);

	//virtual float GetTotalCostPriority(void){ return m_fCostToGoal; }	// Goal±????? Cost¸¸?» °?·???´?.
};


enum MPMPOLYGONTYPE{
	MPMPT_NORMAL = 0,		// ?¯?¤ ¸?¾??? ¾?´? ??¸®°?
	MPMPT_RECT,				// ?º»?°¢?? ¸?¾??? ??¸®°?, ?¸??´? ??¸®°? ????½?¿¡ ???¡?» ?¦°??? ??´?.
};

// ??¸®°??? ¹???
enum MPMPOLYGONDIR{
	MPMPD_NA = 0,				// Not Available
	MPMPD_CLOCKWISE = 1,		// Clockwise
	MPMPD_COUNTERCLOCKWISE = 2,	// Counter-Clockwise
};

// ??¸®°? ??¾?¹°
// Right-Handed??¹?·?, ¹?´????¸·?´? ??µ¿ °¡´???´?.
class MPMPolygon{
public:
	CMPtrList<MPMPoint>		m_PointList;		// °¢ ¸?¼­¸®
	bool					m_bEnclosed;		// ??¸°°??? ´??? ??´?°¡?
	MPMPOLYGONTYPE			m_nType;			// ?¸??
	bool					m_bInnerStartPoint;	// ¸??¥ ???¡?? ?? ??¸®°? ¾???¿¡ ¼??? ??´? °?¿?.
	bool					m_bInnerEndPoint;	// ¸??¥ ???¡?? ?? ??¸®°? ¾???¿¡ ¼??? ??´? °?¿?.
	MPMPOLYGONDIR			m_nPolygonDir;		// ??¸®°??? ¹??? ( ??¸®°??? ´??? ??´? °?¿?¸¸, m_bEnclosed==true )
protected:
	// Merge Support
	CMPtrList<MPMPoint>		m_TempMergedPointAddList;		// ¸???¸¦ ?§?? ??½? ??¸®°? ¸®½??®
	CMPtrList<MPMPoint>		m_TempMergedPointDupList;		// ¸???¸¦ ?§?? ??½? ??¸®°? ¸®½??®, °??¡´? ?º???®
	int						m_nLatestFindIndex;
	MPMPoint*				m_pMergeTestBegin;
	MPMPoint*				m_pMergeTestEnd;
	//int						m_nMergeBeginIndex, m_nMergeEndIndex;	// ¸????ª½??®?? ½???°? ³¡ ??µ¦½?
protected:
	MPMPOLYGONDIR TestPolygonDir(void);			// ??¸®°? ¹??? ?ª½??®

	void Initialize(void);
public:
	MPMPolygon(void);
	// Rect Type?? Polygon »?¼?
	MPMPolygon(MPMPoint* lt, MPMPoint* rt, MPMPoint* rb, MPMPoint* lb);

	void Add(MPMPoint* pPointNode);
	void Delete(int i);
	void Enclose(bool bEnclosed);

	// ??³? ??»??? ¿¬¼?µ? ¿¡??¸¦ °??¯??´? µ? ??¸®°??» ¸?????´? ????????½?, ¸?????·?´? ??¸®°??? ?º???®¸¦ ??³?¾¿ ??°¡?? ³?°¡´? ¹?½?
	void Merge(MPMPoint* pPointNode);
	void MergeReset(void);	// Merge¿¡ °?·?µ? ??½? µ¥????¸¦ ¸®¼???´?.

	void InsertBefore(int nIndex, MPMPoint* pPointNode);
	void AddAfter(int nIndex, MPMPoint* pPointNode);
	void InsertBefore(int nIndex, CMPtrList<MPMPoint>* pList);
	void AddAfter(int nIndex, CMPtrList<MPMPoint>* pList);

	MPMPoint* Find(float x, float y, float z, MPMPoint* pTestBegin, MPMPoint* pTestEnd);
	int FindIndex(float x, float y, float z, MPMPoint* pTestBegin, MPMPoint* pTestEnd);
	int GetIndex(MPMPoint* pTest);

	// ¾?¶² ?¡?? ?? ??¸®°? ¾?¿¡ °¤?? ??´?°¡?
	bool IsInnerPoint(float x, float y, bool bTestClockwise);
	MPMPOLYGONDIR IsInnerPoint(float* t, float x, float y);

	MPMPoint* GetStartPoint(void);
	MPMPoint* GetEndPoint(void);
	MPMPoint* Get(int i);

	void Add(MPMPolygon* pPO);

	void GetPointNormal(float* x, float* y, int i);
	void Enlarge(float fMargin);

	void RebuildConnection(void);

	bool IsIntersect(MPMPoint** p1, MPMPoint** p2, float* t, float x1, float y1, float x2, float y2);	// p1->p2?
	float GetNearestContactPoint(float* px, float* py, float x, float y, bool bInverse=false);			// °¡?? °¡±?¿? ??¸®°? ¶????§¿¡ ?¡
	//float GetContactPointByLineTest(float* px, float* py, float x, float y, int nTestCount);			// ¶????» nTestCount¸¸?­ ±ª¾???¼­ °¡?? °¡±?¿? ?¡ ?£±?, °¡?? °¡±?¿? ?¡?» ?£±? ¾?·???¸¸... ?¦??µ? ?ª½??® ?½¼?¸¦ ?¸????´?.

	MPMPOLYGONDIR GetDir(void);		// Winding Dir
};

// Point ??°¡½? ??¸®°??» ³¡?» ¸?½?.
enum MPMPOLYGONADDTYPE{
	MPMPAT_CONTINUE = 0,	// °?¼? ??¸®°??? ??¾???´?.
	MPMPAT_END,			// ??¸®°??» ³¡³½´?.
	MPMPAT_ENCLOSE,		// ??¸®°??» ´?°? ³¡³½´?.
};

// PreparePathFinding()?? Return Value
enum MPMRESULT{
	MPMR_OK,
	MPMR_ENDPOINTCHANGED,
};

struct MPOLYGONIDPAIR{
	int a, b;
	inline friend bool operator == (const MPOLYGONIDPAIR& a, const MPOLYGONIDPAIR& b){
		if(a.a==b.a && a.b==b.b) return true;
		return false;
	}
	inline friend bool operator > (const MPOLYGONIDPAIR& a, const MPOLYGONIDPAIR& b){
		if(a.a>b.a) return true;
		if(a.a<b.a) return false;
		if(a.b>b.b) return true;
		return false;
	}
	inline friend bool operator < (const MPOLYGONIDPAIR& a, const MPOLYGONIDPAIR& b){
		if(a.a<b.a) return true;
		if(a.a>b.a) return false;
		if(a.b<b.b) return true;
		return false;
	}
};

typedef map<MPOLYGONIDPAIR, bool>	MPOLYGONIDPAIRMAP;
/*
struct MPOLYGONIDPAIR{
	int a, b;
};
struct ltstr
{
	bool operator()(const MPOLYGONIDPAIR& s1, const MPOLYGONIDPAIR& s2) const
	{
		if(s1.a<s2.b) return true;
		return false;
		//return strcmp(s1, s2) < 0;
	}
};
typedef map<MPOLYGONIDPAIR, bool, ltstr>	MPOLYGONIDPAIRMAP;
*/

class MPMPortal{
//protected:
//friend class MPolygonMapModel;
public:
	MPMPoint*	m_pPos[2];	// Portal Position
	int			m_nPortalIndex;
public:
	MPMPortal(MPMPoint* pPos1, MPMPoint* pPos2, int nPortalIndex);
	virtual ~MPMPortal(void);
	void Enlarge(float fMargin);
};


// ?¤???­µ??? ¾??? ¸? ¸?µ¨
class MPolygonMapModel{
protected:
	MPMPolygon*					m_pCurrPolygon;			// Point°¡ ??°¡µ?´? ???? ??¸®°?
	bool						m_bEndPointInPolygon;	// ³¡ ?¡?? ¾?¶² ??¸®°?¿¡ °¤?? ??´?°¡?
	int							m_nChangeCount;			// ¸??? ?¯°?µ? ?«¿??®(?¯°? µ?´??? ¾? ¼? ??´?.)
	int*						m_pPolygonPathList;
	int							m_nPolygonPathCount;
	MPOLYGONIDPAIRMAP			m_PolygonSharedMap;

public:
	MPMPoint*					m_pStartPoint;			// ½??? ?¡
	MPMPoint*					m_pEndPoint;			// ³¡ ?¡
	CMLinkedList<MPMPoint>		m_PointList;
	CMLinkedList<MPMPolygon>	m_PolygonList;
	CMLinkedList<MPMPortal>		m_PortalList;

protected:
	void Change(void);									// ¸??? ?¯?­ ???¤
	void GetInnerClosedPolygon(CMPtrList<MPMPolygon>* pPolygons, int nPointType, MPMPOLYGONDIR dir);	// nPointType=0 Start, nPointType=1 End Point

	bool TestEndPointInPolygon(void);					// ³¡?¡?? ??¸®°? ¾?¿¡ °¤?? ??´??? ?ª½??®, ¹?¸® ?ª½??®???¸·?½? ½??¡ ?¸??¿¡ ¼?´? ?ª»??» ²???¼? ??´?.
	bool TestStartPointInPolygon(void);					// ½????¡?? ??¸®°? ¾?¿¡ °¤?? ??´??? ?ª½??®, ¹?¸® ?ª½??®???¸·?½? ½??¡ ?¸??¿¡ ¼?´? ?ª»??» ²???¼? ??´?.

	// ??¸®°? ¸®½??®??¿¡ Non-Intersect??´? ???¡ ???µ(sx,sy ½????¡, ex,ey ³¡?¡)
	bool FindNonIntersectPosition(CMPtrList<MPMPolygon>* pPolygons, float* rx, float* ry, float sx, float sy, float ex, float ey);
	// ??¸®°? ¸®½??®??¿¡ Non-Intersect??´? ???¡ ???µ(x,y¿¡¼­ °¡±?¿? ?¡)
	bool FindNonIntersectPosition(CMPtrList<MPMPolygon>* pPolygons, float* rx, float* ry, float x, float y);

	bool TestPointInPolygon(float* pNearestPointX, float* pNearestPointY, float x, float y);		// °¤????´??? ?ª½??®??, °¡?? °¡±?¿? ¿??? ???¡ ¸®??
	bool RecommendPoint(float* pRecommendedPointX, float* pRecommendedPointY, float x, float y, float fCheckStepDistance, float fExponentialIncrease=1.0f);	// ??µ¹???? ¾?´? ???¡?¸·? ???µ
	bool RecommendEndPoint(float* pRecX, float* pRecY);	// ??µ¹???? ¾?´? ???¡?¸·? ???µ(return true¸? ?????¤, µ?¶?¼­.. TestEndPointInPolygon()?» ????¾? ??´?.

	friend class MPMPoint;
	void ClearVisibledList(void);				// MPMPoint::m_VisibledList Clear

	bool IsVisibleEdgePoint(MPMPoint* p1, MPMPoint* p2, int nBeginRoom, int nEndRoom);					// p1->p2?	( Edge¸¸ °?»? )
	int IsBiDirectionVisiblePoint(float* pT1, float* pT2, MPMPoint* p1, MPMPoint* p2);					// 0: ¾??½, 1: p1->p2, 2:p2->p1, 3:p1<->p2

public:
	MPolygonMapModel(void);
	virtual ~MPolygonMapModel(void);

	void Destroy(void);

	MPMPoint* AddPoint(float x, float y, float z, int nRoomIndex, bool bCheckDup=false);							// ??¹??¡ ??°¡ ( Point°¡ °??¥ ¼? ??´? °?¿? ???¹ ?¼?©¸¦ ??´?. )
	MPMPoint* AddPoint(float x, float y, float z, list<int>& RoomIndexs, bool bCheckDup=false);					// ??¹??¡ ??°¡ ( Point°¡ °??¥ ¼? ??´? °?¿? ???¹ ?¼?©¸¦ ??´?. )
	MPMPolygon* AddPolygon(MPMPoint* pNode, MPMPOLYGONADDTYPE nType=MPMPAT_CONTINUE);	// ???? ??¸®°?¿¡ ?¡ ??°¡
	MPMPolygon* AddRect(float x1, float y1, float z1, float x2, float y2, float z2, int pRoomIndexs[4], bool bInverse=false);	// ?º »?°¢?? ?????? ??¸®°? ??°¡

	MPMPortal* AddPortal(MPMPoint* p1, MPMPoint* p2);

	MPMPoint* AddStartPoint(float x, float y, float z, int nRoomIndex);				// ½????¡ ??°¡
	MPMPoint* AddEndPoint(float x, float y, float z, int nRoomIndex);				// ³¡ ?¡ ??°¡
	MPMPoint* GetStartPoint(void);
	MPMPoint* GetEndPoint(void);

	void SetStartPoint(float x, float y, float z, int nRoomIndex);					// ½????¡
	void SetEndPoint(float x, float y, float z, int nRoomIndex);					// ³¡ ?¡

	// Low-Level Addition
	void AddPoint(MPMPoint* pPoint);												// ?º???® ??°¡
	void AddPolygon(MPMPolygon* pPolygon);											// ??¸®°? ??°¡
	void AddStartPoint(MPMPoint* pPoint);											// ½????¡ ??°¡
	void AddEndPoint(MPMPoint* pPoint);												// ³¡ ?¡ ??°¡
	void AddPortal(MPMPortal* pPortal);												// ?º?» ??°¡

	void Clear(void);																	// ??°¡µ? Point, Polygon ??±??­

	MPMRESULT PreparePathFinding(float* pRecX, float* pRecY);							// Path Finding Prepare ( End Point¸¦ ?????º ?§?¡½??²´?.)

	void SetPolygonPathList(int* pPolygonList, int nCount);										// »??§·¹?§?? ??¸®°?´??§?? ??½? ¸®½??®
	bool IsPolygonPathInclude(int nRoomIndexTest, int nRoomIndexBegin, int nRoomIndexEnd);	// Begin~End »???¿¡ ?º??µ?¾? ??´?°¡?
	bool IsPolygonPathInclude(list<int>* pRoomIndexTest, list<int>* pRoomIndexBegin, list<int>* pRoomIndexEnd);

	void SetPolygonSharedMap(MPOLYGONIDPAIRMAP* pSharedMap);

	bool IsEndPointInPolygon(void);														// ³¡ ?¡?? ??¸®°?¿¡ °¤?? ??´?°¡?
	void MergeBreakPolygon(void);														// ¼­·? ´?¸¥ ¿­·???´?(MPMPAT_END) ??¸®°?µ??? ½????¡??³? ³¡?¡?? °???¼­ ¿¬°??? °¡´??? ??¸®°? ???¡±?

	MPMPolygon* IsInnerClosedPolygon(float x, float y, bool bTestClockwise=true);		// ?¡?? ??¸®°? ¾?¿¡ ¼??? ??´?°¡?
	void IsInnerClosedPolygon(CMPtrList<MPMPolygon>* pPolygons, float x, float y, bool bTestClockwise=true);	// ?¡?? ??¸®°? ¾?¿¡ ¼??? ??´?°¡?

	bool IsIntersect(MPMPoint** p1, MPMPoint** p2, float* t, float x1, float y1, float x2, float y2);			// p1->p2?

	MPMPolygon* GetNearestInnerPolygon(CMLinkedList<MPMPolygon>* pPolygons, float x, float y);	// Inner Polygon?? °¡?? °¡±???¿¡ ??´? ??¸®°?( ¹???¿¡ »?°? ¾??½ ) (1, 0, 0) ¹?????´?.

	bool IsEnable(CMLinkedList<MPMPolygon>* pPolygons, float x, float y);	// Enable Position??°¡?
	bool IsEnable(float x, float y);									// Enable Position??°¡?

	void Add(MPolygonMapModel* pNGMM);

	void Enlarge(float fMargin);

	int GetChangeCount(void);
};

#endif

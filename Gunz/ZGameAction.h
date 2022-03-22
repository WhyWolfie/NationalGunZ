#ifndef _ZGAMEACTION_H
#define _ZGAMEACTION_H

#include "MCommand.h"

// 이 enum 은 게임 프로토콜과 관련이 있으므로 변경은 불가. 추가만 하자
enum ZREACTIONID{	
	ZR_CHARGING = 0,	
	ZR_CHARGED,
	ZR_BE_UPPERCUT,			
	ZR_BE_DASH_UPPERCUT,	
	ZR_DISCHARGED,		

	ZR_END
};

class ZCharacter;
class ZMyCharacter;

/// ZGame에서 캐릭터들간의 액션에 관한 것들(ZGame의 OnPeer씨리즈들)은 이곳으로 빼냅시다.
class ZGameAction
{
	bool OnReaction(MCommand* pCommand);
	bool OnPeerSkill(MCommand* pCommand);
	bool OnEnchantDamage(MCommand* pCommand);

public:
	bool ApplyFireEnchantDamage(ZObject* pTarget, ZObject* pOwner, int nDamage, int nDuration);
	bool ApplyColdEnchantDamage(ZObject* pTarget, int nLimitSpeed, int nDuration);
	bool ApplyPoisonEnchantDamage(ZObject* pTarget, ZObject* pOwner, int nDamage, int nDuration);
	bool ApplyLightningEnchantDamage(ZObject* pTarget, ZObject* pOwner, int nDamage, int nDuration);

private:
	void OnPeerSkill_Uppercut(ZCharacter *pOwnerCharacter);
	void OnPeerSkill_LastShot(float fShotTime,ZCharacter *pOwnerCharacter);	// 강베기 스플래시
	void OnPeerSkill_Dash(ZCharacter *pOwnerCharacter);

public:
	bool OnCommand(MCommand* pCommand);
};


#define CHARGED_TIME			15.f		// 강베기 지속시간 (단위:초)
#define COUNTER_CHARGED_TIME	1.f			// 반격기 강베기 지속시간


#endif
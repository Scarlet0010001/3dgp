#include "boss.h"

/*--------------------èÛë‘ëJà⁄------------------------*/
void Boss::TransitionIdleState()
{
	act_update = &Boss::UpdateIdleState;
	state = State::IDLE;
	bossAnimation = BossAnimation::BOSS_IDLE;
}

void Boss::TransitionWalkState()
{
	act_update = &Boss::UpdateWalkState;
	state = State::IDLE;
	//bossAnimation = BossAnimation::BOSS_IDLE;

}

void Boss::TransitionRunState()
{
}

void Boss::TransitionAttack_Melee_State()
{
}

void Boss::TransitionAttack_ShotStraight_State()
{
}

void Boss::TransitionAttack_ShotHoming_State()
{
}

void Boss::TransitionDamageState()
{
}

void Boss::TransitionDeadState()
{
}

void Boss::TransitionDownState()
{
}

void Boss::UpdateIdleState(float elapsedTime)
{
}

void Boss::UpdateWalkState(float elapsedTime)
{
}

void Boss::UpdateRunState(float elapsedTime)
{
}

void Boss::UpdateAttack_Melee_State(float elapsedTime)
{
}

void Boss::UpdateAttack_ShotStraight_State(float elapsedTime)
{
}

void Boss::UpdateAttack_ShotHoming_State(float elapsedTime)
{
}

void Boss::UpdateDamageState(float elapsedTime)
{
}

void Boss::UpdateDeadState(float elapsedTime)
{
}

void Boss::UpdateDownState(float elapsedTime)
{
}

void Boss::AttackRoutine(float elapsedTime)
{
}

void Boss::SelectAttackTypeShort()
{
}

void Boss::SelectAttackTypeLong()
{
}

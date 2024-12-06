#include "boss.h"
#include "noise.h"

/*--------------------状態遷移------------------------*/
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
	stateTimer += elapsedTime;
	if (stateTimer > state_duration)
	{
		if (health < charaParam.maxHealth / 2)
		{
			//HP半分以下なら走る
			TransitionRunState();
		}
		else
		{
			TransitionWalkState();
		}

		stateTimer = 0;
	}
	//速度更新
	UpdateVelocity(elapsedTime, position);
}

void Boss::UpdateWalkState(float elapsedTime)
{
	//プレイヤー方向に歩く
	DirectX::XMFLOAT3 dir_target_vec = Math::calc_vector_AtoB_normalize(position, target_pos);
	Move(dir_target_vec.x, dir_target_vec.z, WALK_SPEED);
	Turn(elapsedTime, dir_target_vec, charaParam.turnSpeed, orientation);

	//攻撃のルーチン
	AttackRoutine(elapsedTime);

	//速度更新
	UpdateVelocity(elapsedTime, position);

}

void Boss::UpdateRunState(float elapsedTime)
{
	DirectX::XMFLOAT3 dir_target_vec = Math::calc_vector_AtoB_normalize(position, target_pos);
	Move(dir_target_vec.x, dir_target_vec.z, RUN_SPEED);
	Turn(elapsedTime, dir_target_vec, charaParam.turnSpeed, orientation);

	//攻撃のルーチン
	AttackRoutine(elapsedTime);

	//速度更新
	UpdateVelocity(elapsedTime, position);
}

void Boss::UpdateAttack_Melee_State(float elapsedTime)
{
	AttackParam.isAttack = true;
	if (model->GetIsEndAnimation())
	{
		TransitionIdleState();
		state_duration = NORMAL_ATTACK_COOLTIME;
		AttackParam.isAttack = false;
	}
	//速度更新
	UpdateVelocity(elapsedTime, position);

}

void Boss::UpdateAttack_ShotStraight_State(float elapsedTime)
{
	

	//速度更新
	UpdateVelocity(elapsedTime, position);
}

void Boss::UpdateAttack_ShotHoming_State(float elapsedTime)
{

	//速度更新
	UpdateVelocity(elapsedTime, position);
}

void Boss::UpdateDamageState(float elapsedTime)
{
	if (model->GetIsEndAnimation())
	{
		state_duration = DAMAGE_STUN_DURATION;;
		TransitionIdleState();
	}
}

void Boss::UpdateDeadState(float elapsedTime)
{
}

void Boss::UpdateDownState(float elapsedTime)
{
}

void Boss::AttackRoutine(float elapsedTime)
{
	float length_to_target = Math::calc_vector_AtoB_length(position, target_pos);
	if (length_to_target < ATTACK_ACTION_LENGTH)
	{
		//攻撃タイプ選択
		SelectAttackTypeShort();
		return;
	}

	//プレイヤーが一定時間自分に近づかなかったら遠距離攻撃
	attackResponderTimer += elapsedTime;
	if (attackResponderTimer > ATTACK_RESPONDER_TIME)
	{
		SelectAttackTypeLong();
		attackResponderTimer = 0;
	}
}

void Boss::SelectAttackTypeShort()
{
	int random = std::abs(static_cast<int>(Noise::instance().get_rnd())) % static_cast<int>(ATTACK_TYPE::MAX_COUNT);
	//ランダムで攻撃方法を選択
	ATTACK_TYPE attack_type = static_cast<ATTACK_TYPE>(random);
	switch (attack_type)
	{
	case ATTACK_TYPE::NORMAL:
		TransitionAttack_Melee_State();
		break;

	case ATTACK_TYPE::SHOT_S:
		TransitionAttack_ShotStraight_State();
		break;

	case ATTACK_TYPE::SHOT_H:
		TransitionAttack_ShotHoming_State();
		break;
	}

}

void Boss::SelectAttackTypeLong()
{
	int random = std::abs(static_cast<int>(Noise::instance().get_rnd())) % static_cast<int>(ATTACK_TYPE::MAX_COUNT);
	//ランダムで攻撃方法を選択
	ATTACK_TYPE attack_type = static_cast<ATTACK_TYPE>(random);
	//通常攻撃は除外し、遠距離攻撃のみ選択
	switch (attack_type)
	{
	case ATTACK_TYPE::SHOT_S:
		TransitionAttack_ShotStraight_State();
		break;

	case ATTACK_TYPE::SHOT_H:
		TransitionAttack_ShotHoming_State();
		break;
	}
}

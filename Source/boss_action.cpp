#include "boss.h"
#include "noise.h"

/*--------------------状態遷移------------------------*/
void Boss::TransitionIdleState()
{
	act_update = &Boss::UpdateIdleState;
	state = STATE::IDLE;
	bossAnimation = BossAnimation::BOSS_IDLE;
	stateTimer = 0;
	stateDuration = 2.0f;

}

void Boss::TransitionWalkState()
{
	act_update = &Boss::UpdateWalkState;
	state = STATE::WALK;
	charaParam.moveSpeed = WALK_SPEED;
	bossAnimation = BossAnimation::BOSS_WALK;
	stateTimer = 0;
	stateDuration = 3.0f;
}

void Boss::TransitionAttack_Tackle_State()
{
	act_update = &Boss::UpdateAttack_Tackle_State;
	state = STATE::TACKLE;
	tackleCameraShake.onDistanceShake = true;
	attackParam = param.tackleParam;
	bossAnimation = BossAnimation::BOSS_RUN;
	stateTimer = 0;
	stateDuration = 1.0f;
}

void Boss::TransitionAttack_Jump_State()
{
	act_update = &Boss::UpdateAttack_Jump_State;
	state = STATE::JUMP;
	charaParam.acceleration = ACCELERATION_JUMP_SPEED;
	attackParam = param.stompParam;
	bossAnimation = BossAnimation::BOSS_CHARGE;
	stateTimer = 0;
	stateDuration = 2.5f;
	chargeEffect->Play(position,3.0f);
}

void Boss::TransitionAttack_ShotStraight_State()
{
	act_update = &Boss::UpdateAttack_ShotStraight_State;
	charaParam.acceleration = ACCELERATION_JUMP_SPEED;
	state = STATE::SHOT_S;
	rapidCount = 0;
	bossAnimation = BossAnimation::BOSS_JUMP;
	stateTimer = 0;
	stateDuration = RAPIDFIRE_TIME;
}

void Boss::TransitionAttack_ShotHoming_State()
{
	act_update = &Boss::UpdateAttack_ShotHoming_State;
	state = STATE::SHOT_H;
	bossAnimation = BossAnimation::BOSS_MISSILE;
	stateTimer = 0;
	stateDuration = 3.0f;
}

void Boss::TransitionDamageState()
{
	act_update = &Boss::UpdateDamageState;
	state = STATE::DAMAGE;
	bossAnimation = BossAnimation::BOSS_HIT;
	stateTimer = 0;
	stateDuration = DAMAGE_STUN_DURATION;
}

void Boss::TransitionDeadState()
{
	act_update = &Boss::UpdateDeadState;
	state = STATE::DEAD;
	bossAnimation = BossAnimation::BOSS_DEAD;
	stateTimer = 0;

}

void Boss::TransitionDownState()
{
}

void Boss::UpdateIdleState(float elapsedTime)
{
	if (stateTimer > stateDuration)
	{
		TransitionWalkState();
	}
	//速度更新
	UpdateVelocity(elapsedTime, position);
}

void Boss::UpdateWalkState(float elapsedTime)
{
	//プレイヤー方向に歩く
	DirectX::XMFLOAT3 dir_target_vec = Math::calc_vector_AtoB_normalize(position, target_pos);
	Move(dir_target_vec.x, dir_target_vec.z, charaParam.moveSpeed);
	Turn(elapsedTime, dir_target_vec, charaParam.turnSpeed, orientation);

	if (stateTimer > stateDuration)
	{
		//攻撃のルーチン
		AttackRoutine(elapsedTime);
	}

	//速度更新
	UpdateVelocity(elapsedTime, position);

}

void Boss::UpdateAttack_Tackle_State(float elapsedTime)
{
	if (stateTimer < stateDuration)
	{
		targetPoint_pos.x = target_pos.x;
		targetPoint_pos.z = target_pos.z;
		return;
	}

	//目標地点までXZ平面での距離判定
	float vx = targetPoint_pos.x - position.x;
	float vz = targetPoint_pos.z - position.z;
	float distSq = vx * vx + vz * vz;

	attackParam.isAttack = true;
	DirectX::XMFLOAT3 pos = { position.x ,0.0f,position.z };
	DirectX::XMFLOAT3 pointPos = { targetPoint_pos.x ,0.0f,targetPoint_pos.z };
	DirectX::XMFLOAT3 dir_target_vec = Math::calc_vector_AtoB_normalize(pos, pointPos);
	Move(dir_target_vec.x, dir_target_vec.z, param.runSpeed);
	Turn(elapsedTime, dir_target_vec, charaParam.turnSpeed, orientation);	

	const float radius = 2.0f;
	if (distSq < radius * radius)
	{
		TransitionIdleState();
		stateDuration = NORMAL_ATTACK_COOLTIME;
		attackParam.isAttack = false;
	}
	//速度更新
	UpdateVelocity(elapsedTime, position);
}

void Boss::UpdateAttack_Jump_State(float elapsedTime)
{
	if (bossAnimation == BossAnimation::BOSS_JUMP
		&& time < 0.1f)
		return;
	if (stateTimer < stateDuration)
	{

		charaParam.moveSpeed = CalcMoveSpeed(target_pos, 0.5f);
		targetPoint_pos.x = target_pos.x;
		targetPoint_pos.y = target_pos.y;
		targetPoint_pos.z = target_pos.z;
		return;
	}
	else bossAnimation = BossAnimation::BOSS_JUMP;

	float length = targetPoint_pos.y - position.y;
	if (!isJump && length > 5.0f && time > 0.1f)
	{
		isJump = true;
		velocity.y = 30.0f;
	}

	//目標地点までXZ平面での距離判定
	float vx = targetPoint_pos.x - position.x;
	float vz = targetPoint_pos.z - position.z;
	float distSq = vx * vx + vz * vz;

	DirectX::XMFLOAT3 dir_target_vec{};
	attackParam.isAttack = true;

	const float radius = 3.0f;
	if (distSq < radius * radius)
	{
		if (model->GetIsEndAnimation())
		{
			TransitionIdleState();
			attackParam.isAttack = false;
			charaParam.moveSpeed = WALK_SPEED;
			//charaParam.maxMoveSpeed = WALK_SPEED;
			charaParam.acceleration = ACCELERATION_NORMAL_SPEED;
			isJump = false;
		}
		else
		{
			stateDuration = NORMAL_ATTACK_COOLTIME;
			charaParam.moveSpeed = 0;
			velocity.x = 0.0f;
			velocity.z = 0.0f;
			charaParam.acceleration = 0.0f;
			//position = targetPoint_pos;
		}
		
		//Camera::Instance().SetOnDistanceShake(false);
	}
	else
		dir_target_vec = Math::calc_vector_AtoB_normalize(position, targetPoint_pos);

	Move(dir_target_vec.x, dir_target_vec.z, charaParam.moveSpeed);
	Turn(elapsedTime, dir_target_vec, charaParam.turnSpeed, orientation);
	
	//速度更新
	UpdateVelocity(elapsedTime, position);
}

void Boss::UpdateAttack_ShotStraight_State(float elapsedTime)
{
	//飛びのいた後連射設定する
	if (model->GetIsEndAnimation() && !isBackJump)
	{
		stateDuration = RAPIDFIRE_TIME;
		stateTimer = 0.0f;
		isBackJump = true;

		charaParam.moveSpeed = 0;
		velocity.x = 0.0f;
		velocity.z = 0.0f;
		charaParam.acceleration = 0.0f;
	}
	//後方に飛びのく
	if (!isBackJump)
	{
		const float backJumpSpeed = 20.0f;
		DirectX::XMFLOAT3 dir_target_vec = Math::calc_vector_AtoB_normalize(target_pos, position);
		Move(dir_target_vec.x, dir_target_vec.z, backJumpSpeed);
		Turn(elapsedTime, dir_target_vec, charaParam.turnSpeed, orientation);
	}
	
	if (stateTimer > stateDuration && isBackJump)
	{
		ShotBullet(ATTACK_TYPE::SHOT_S);
		stateTimer = 0;
		rapidCount++;
	}



	if (rapidCount > RAPID_MAX && isBackJump)
	{
		TransitionIdleState();
		isBackJump = false;
		stateDuration = NORMAL_ATTACK_COOLTIME;
		charaParam.moveSpeed = WALK_SPEED;
		charaParam.maxMoveSpeed = WALK_SPEED;
		charaParam.acceleration = ACCELERATION_NORMAL_SPEED;
	}

	//速度更新
	UpdateVelocity(elapsedTime, position);
}

void Boss::UpdateAttack_ShotHoming_State(float elapsedTime)
{
	ShotBullet(ATTACK_TYPE::SHOT_H);
	TransitionIdleState();
	if (model->GetIsEndAnimation())
	{
		TransitionIdleState();
		stateDuration = NORMAL_ATTACK_COOLTIME;

	}

	//速度更新
	UpdateVelocity(elapsedTime, position);
}

void Boss::UpdateDamageState(float elapsedTime)
{
	if (stateTimer > stateDuration)
	{
		TransitionIdleState();
		stateDuration = DAMAGE_STUN_DURATION;
	}
}

void Boss::UpdateDeadState(float elapsedTime)
{
	if (model->GetIsEndAnimation())
	{
		isDead = true;
	}
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
	int random = std::abs(static_cast<int>(Noise::Instance().get_rnd())) % static_cast<int>(ATTACK_TYPE::MAX_COUNT);
	//ランダムで攻撃方法を選択
	ATTACK_TYPE attack_type = static_cast<ATTACK_TYPE>(random);
	switch (attack_type)
	{
	case ATTACK_TYPE::TACKLE:
		TransitionAttack_Tackle_State();
		break;
	case ATTACK_TYPE::JUMP:
		TransitionAttack_Jump_State();
		break;
	case ATTACK_TYPE::SHOT_S:
		TransitionAttack_ShotStraight_State();

	}

}

void Boss::SelectAttackTypeLong()
{
	int random = std::abs(static_cast<int>(Noise::Instance().get_rnd())) % static_cast<int>(ATTACK_TYPE::MAX_COUNT);
	//ランダムで攻撃方法を選択
	ATTACK_TYPE attack_type = static_cast<ATTACK_TYPE>(random);
	//通常攻撃は除外し、遠距離攻撃のみ選択
	switch (attack_type)
	{
	case ATTACK_TYPE::TACKLE:
		TransitionAttack_Tackle_State();
		break;
	case ATTACK_TYPE::JUMP:
		TransitionAttack_Jump_State();
		break;
	case ATTACK_TYPE::SHOT_S:
		TransitionAttack_ShotStraight_State();
		break;
	}
}


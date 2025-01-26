#include "player.h"
#include "operators.h"
#include "bullet_straight.h"

void Player::TransitionIdleState()
{
	p_update = &Player::UpdateIdleState;
	state = STATE::IDLE;
	playerAnimation = PlayerAnimation::PLAYER_IDLE;
}

void Player::TransitionMoveState()
{
	p_update = &Player::UpdateMoveState;
	state = STATE::MOVE;
	charaParam.acceleration = accelerationState[ACCELERATION_STATE::MOVE];

}

void Player::TransitionWingState()
{
	p_update = &Player::UpdateWingState;
	playerAnimation = PlayerAnimation::PLAYER_WING_START;
	state = STATE::WING;
	charaParam.acceleration = accelerationState[ACCELERATION_STATE::WING];
	isHover = false;
}

void Player::TransitionAvoidanceState()
{
	p_update = &Player::UpdateAvoidanceState;
	playerAnimation = PlayerAnimation::PLAYER_MOVE_FORWARD;
	state = STATE::ROLL;
	charaParam.maxMoveSpeed = param.avoidanceSpeed;
	charaParam.acceleration = accelerationState[ACCELERATION_STATE::AVOIDANCE];

	param.avoidanceTimer = 0;
}

void Player::TransitionJumpState()
{
	p_update = &Player::UpdateJumpState;
	if(isGround)
		playerAnimation = PlayerAnimation::PLAYER_JUMP_START;
	else
		playerAnimation = PlayerAnimation::PLAYER_JUMP;
	charaParam.acceleration = accelerationState[ACCELERATION_STATE::MOVE];
	state = STATE::JUMP;

}

void Player::TransitionLandingState()
{
	p_update = &Player::UpdateLandingState;
	playerAnimation = PlayerAnimation::PLAYER_JUMP_END;
	state = STATE::JUMP;

}

void Player::TransitionShotState()
{
	p_update = &Player::UpdateShotState;
	playerAnimation = PlayerAnimation::PLAYER_SHOT_IDLE;
	state = STATE::SHOT;

}

void Player::TransitionCombo_01_01_State()
{
	p_update = &Player::UpdateCombo_01_01_State;
	playerAnimation = PlayerAnimation::PLAYER_ATTACK_01;
	state = STATE::RIGHT_ATTACK;
	attackParam = param.combo_1;
	nextCombo = false;
}

void Player::TransitionCombo_01_02_State()
{
	p_update = &Player::UpdateCombo_01_02_State;
	playerAnimation = PlayerAnimation::PLAYER_ATTACK_02;
	state = STATE::LEFT_ATTACK;
	attackParam = param.combo_2;
	nextCombo = false;

}

void Player::TransitionCombo_01_03_State()
{
	p_update = &Player::UpdateCombo_01_03_State;
	playerAnimation = PlayerAnimation::PLAYER_ATTACK_03;
	state = STATE::RIGHT_ATTACK;
	attackParam = param.combo_3;
	nextCombo = false;

}

void Player::TransitionCombo_PowerL_State()
{
	p_update = &Player::UpdateCombo_PowerL_State;
	playerAnimation = PlayerAnimation::PLAYER_POWER_L;
	state = STATE::LEFT_ATTACK;
	nextCombo = false;

}

void Player::TransitionCombo_PowerR_State()
{
	p_update = &Player::UpdateCombo_PowerR_State;
	playerAnimation = PlayerAnimation::PLAYER_POWER_R;
	state = STATE::RIGHT_ATTACK;
	nextCombo = false;

}

void Player::TransitionDamage_State()
{
	p_update = &Player::UpdateDamage_State;
	playerAnimation = PlayerAnimation::PLAYER_DAMAGE;
	state = STATE::DAMAGE;

}

void Player::TransitionDead_State()
{
	p_update = &Player::UpdateDead_State;
	playerAnimation = PlayerAnimation::PLAYER_DEAD;
	state = STATE::DAMAGE;

}

void Player::UpdateIdleState(float elapsedTime)
{
	if (InputMove(elapsedTime))
	{
		TransitionMoveState();
	}
	//ジャンプ入力
	InputJump();
	//回避入力
	InputAvoidance();
	//飛行入力
	InputWing();
	//攻撃入力
	if (gamePad->GetButtonDown() & gamePad->BTN_X
		//|| mouse->GetButton() & mouse->BTN_LEFT_CLICK
		)
	{
		TransitionCombo_01_01_State();
	}
	//射撃入力
	if (mouse->GetButtonDown() & mouse->BTN_RIGHT_CLICK)
	{
		TransitionShotState();
	}

	//速力処理更新
	UpdateVelocity(elapsedTime, position);

}

void Player::UpdateMoveState(float elapsedTime)
{
	float ax = gamePad->GetAxis_LX();
	float ay = gamePad->GetAxis_LY();

	if (ax > 0) playerAnimation = PlayerAnimation::PLAYER_MOVE_RIGHT;
	else if (ax < 0)
		playerAnimation = PlayerAnimation::PLAYER_MOVE_LEFT;
	if (ay > 0) playerAnimation = PlayerAnimation::PLAYER_MOVE_FORWARD;
	else if (ay < 0) playerAnimation = PlayerAnimation::PLAYER_MOVE_BACK;
	
	if (!InputMove(elapsedTime) /*&& isGround*/)
	{
		TransitionIdleState();
	}

	//ジャンプ入力
	InputJump();
	//回避入力
	InputAvoidance();
	//飛行入力
	InputWing();
	//攻撃入力
	if (gamePad->GetButtonDown() & gamePad->BTN_X
		//|| mouse->GetButton() & mouse->BTN_LEFT_CLICK
		)
	{
		TransitionCombo_01_01_State();
	}
	//射撃入力
	if (mouse->GetButtonDown() & mouse->BTN_RIGHT_CLICK)
	{
		TransitionShotState();
	}

	//速力処理更新
	UpdateVelocity(elapsedTime, position);
}

void Player::UpdateWingState(float elapsedTime)
{
	if (playerAnimation == PlayerAnimation::PLAYER_WING_START)
	{
		if (model->GetIsEndAnimation())
		{
			playerAnimation = PlayerAnimation::PLAYER_WING;
		}
	}
	else
	{
		//向いている方向に速度を足す
		velocity.x = (forward * (param.wingSpeed)).x;
		velocity.y = (forward * (param.wingSpeed)).y;
		velocity.z = (forward * (param.wingSpeed)).z;

		InputMoveWing(elapsedTime);
	}
	if (gamePad->GetButtonDown() & GamePad::BTN_LEFT_TRIGGER)//Q
	{
		playerAnimation = PlayerAnimation::PLAYER_WING_END;
	}
	if (playerAnimation == PlayerAnimation::PLAYER_WING_END)
	{
		if (model->GetIsEndAnimation())
		{
			TransitionIdleState();
		}
	}
	//回避入力
	//InputAvoidance();

	//攻撃入力
	if (mouse->GetButton() & mouse->BTN_RIGHT_CLICK)
	{

	}

	//速力処理更新
	UpdateVelocity(elapsedTime, position);

}

void Player::UpdateAvoidanceState(float elapsedTime)
{	
	//進行ベクトル取得
	const DirectX::XMFLOAT3 moveVec = GetMoveVec(camera);
	//if(moveVec.x ==0.0f && moveVec.z == 0.0f)

	//徐々に速度を落としていく
	velocity.x /= 2.0f;
	velocity.z /= 2.0f;
	//速力処理更新
	UpdateVelocity(elapsedTime, position);

	//if (param.avoidanceTimer < 0.1f)
	//{
	//	//向いている方向に速度を足す
	//	//velocity.x += moveVec.x * param.avoidanceSpeed;
	//	//velocity.z += moveVec.z * param.avoidanceSpeed;
	//	charaParam.acceleration = accelerationState[ACCELERATION_STATE::AVOIDANCE];
	//}
	if (param.avoidanceTimer > 0.1f)
	{
		charaParam.maxMoveSpeed = charaParam.moveSpeed;

		// MOVEステートへ移行
		if (InputMove(elapsedTime))
		{
			TransitionMoveState();
			return;
		}
		else
		{
			TransitionIdleState();
			return;
		}
	}

	param.avoidanceTimer += elapsedTime;
}

void Player::UpdateJumpState(float elapsedTime)
{
	if (playerAnimation == PlayerAnimation::PLAYER_JUMP_START)
	{
		if (model->GetIsEndAnimation())
		{
			playerAnimation =  PlayerAnimation::PLAYER_JUMP;
		}
	}

	InputMove(elapsedTime);

	if (isHover)
		TransitionIdleState();

	//飛行入力
	InputWing();
	//攻撃入力
	if (gamePad->GetButtonDown() & gamePad->BTN_X
		//|| mouse->GetButton() & mouse->BTN_LEFT_CLICK
		)
	{
		TransitionCombo_01_01_State();
	}

	//速力処理更新
	UpdateVelocity(elapsedTime, position);
}

void Player::UpdateLandingState(float elapsedTime)
{
	if (model->GetIsEndAnimation())
	{
		TransitionIdleState();
	}
}

void Player::UpdateShotState(float elapsedTime)
{
	if (mouse->GetButtonUp() & mouse->BTN_RIGHT_CLICK)
	{
		TransitionIdleState();
	}

	float ax = gamePad->GetAxis_LX();
	float ay = gamePad->GetAxis_LY();

	playerAnimation = PlayerAnimation::PLAYER_SHOT_IDLE;
	if (ax > 0) playerAnimation = PlayerAnimation::PLAYER_SHOT_RIGHT;
	else if (ax < 0) playerAnimation = PlayerAnimation::PLAYER_SHOT_LEFT;
	if (ay > 0) playerAnimation = PlayerAnimation::PLAYER_SHOT_FORWARD;
	else if (ay < 0) playerAnimation = PlayerAnimation::PLAYER_SHOT_BACK;

	if(time <= 0) InputShot();

	InputMove(elapsedTime, param.floatingValue, 1);

	//ジャンプ入力
	//InputJump();
	//回避入力
	//InputAvoidance();
	//飛行入力
	InputWing();
	//攻撃入力
	if (gamePad->GetButtonDown() & gamePad->BTN_X
		//|| mouse->GetButton() & mouse->BTN_LEFT_CLICK
		)
	{
		TransitionCombo_01_01_State();
	}

	//速力処理更新
	UpdateVelocity(elapsedTime, position);
}


void Player::UpdateCombo_01_01_State(float elapsedTime)
{
	if (gamePad->GetButtonDown() & gamePad->BTN_X
		//|| mouse->GetButton() & mouse->BTN_LEFT_CLICK
		)
	{
		nextCombo = true;
	}

	if (0.023f < time && !attackParam.isAttack)
	{
		//camera->SetCameraShake(attackParam.cameraShake);
		//camera->SetHitStop(attackParam.hitStop);
		attackParam.isAttack = true;
	}
	if (0.023f < time && nextCombo)
	{
		TransitionCombo_01_02_State();
		attackParam.isAttack = false;
	}
	if (0.15f < time)
		attackParam.isAttack = false;

	if (model->GetIsEndAnimation())
	{
		TransitionIdleState();
		attackParam.isAttack = false;

	}
}

void Player::UpdateCombo_01_02_State(float elapsedTime)
{
	if (gamePad->GetButtonDown() & gamePad->BTN_X
		//|| mouse->GetButton() & mouse->BTN_LEFT_CLICK
		)
	{
		nextCombo = true;

	}

	if (0.03f < time && !attackParam.isAttack)
	{
		//camera->SetCameraShake(attackParam.cameraShake);
		//camera->SetHitStop(attackParam.hitStop);
		attackParam.isAttack = true;
	}
	if (0.03f < time && nextCombo)
	{
		TransitionCombo_01_03_State();
		attackParam.isAttack = false;

	}
	if (0.175f < time)
		attackParam.isAttack = false;

	if (model->GetIsEndAnimation())
	{
		attackParam.isAttack = false;

		TransitionIdleState();
	}
}

void Player::UpdateCombo_01_03_State(float elapsedTime)
{
	if (gamePad->GetButtonDown() & gamePad->BTN_X
		//|| mouse->GetButton() & mouse->BTN_LEFT_CLICK
		)
	{
		nextCombo = true;
	}
	if (0.03f < time && !attackParam.isAttack)
	{
		//camera->SetCameraShake(attackParam.cameraShake);
		//camera->SetHitStop(attackParam.hitStop);
		attackParam.isAttack = true;
	}
	if (0.65f < time)
		attackParam.isAttack = false;

	if (0.48f < time && nextCombo)
	{
		TransitionCombo_01_03_State();
		attackParam.isAttack = false;

	}
	if (model->GetIsEndAnimation())
	{
		TransitionIdleState();
		attackParam.isAttack = false;

	}
}

void Player::UpdateCombo_PowerL_State(float elapsedTime)
{
	if (0.6f < time)
	{
		if (gamePad->GetButtonDown() & gamePad->BTN_X
			//|| mouse->GetButton() & mouse->BTN_LEFT_CLICK
			)
		{
			TransitionCombo_01_03_State();
		}
	}
	if (model->GetIsEndAnimation())
	{
		TransitionIdleState();
	}

}

void Player::UpdateCombo_PowerR_State(float elapsedTime)
{
}

void Player::UpdateDamage_State(float elapsedTime)
{
	if (model->GetIsEndAnimation())
	{
		TransitionIdleState();
	}
	//速力処理更新
	UpdateVelocity(elapsedTime, position);

}

void Player::UpdateDead_State(float elapsedTime)
{
	if (model->GetIsEndAnimation())
	{
		isDead = true;
	}
}

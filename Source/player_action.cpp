#include "player.h"
#include "operators.h"


void Player::TransitionIdleState()
{
	p_update = &Player::UpdateIdleState;
	state = State::IDLE;
	playerAnimation = PlayerAnimation::PLAYER_IDLE;
}

void Player::TransitionMoveState()
{
	p_update = &Player::UpdateMoveState;
	state = State::MOVE;

}

void Player::TransitionWingState()
{
	p_update = &Player::UpdateWingState;
	playerAnimation = PlayerAnimation::PLAYER_WING_START;
	state = State::WING;

}

void Player::TransitionAvoidanceState()
{
	p_update = &Player::UpdateAvoidanceState;

	state = State::ROLL;
	param.avoidanceTimer = 0;
}

void Player::TransitionJumpState()
{
	p_update = &Player::UpdateJumpState;
	playerAnimation = PlayerAnimation::PLAYER_JUMP_START;
	state = State::JUMP;

}

void Player::TransitionLandingState()
{
	p_update = &Player::UpdateLandingState;
	playerAnimation = PlayerAnimation::PLAYER_JUMP_END;
	state = State::JUMP;

}

void Player::TransitionShotState()
{
	p_update = &Player::UpdateShotState;
	playerAnimation = PlayerAnimation::PLAYER_SHOT_IDLE;
	state = State::SHOT;

}

void Player::TransitionCombo_01_01_State()
{
	p_update = &Player::UpdateCombo_01_01_State;
	playerAnimation = PlayerAnimation::PLAYER_ATTACK_01;
	state = State::NORMAL_ATTACK01;
	nextCombo = false;
}

void Player::TransitionCombo_01_02_State()
{
	p_update = &Player::UpdateCombo_01_02_State;
	playerAnimation = PlayerAnimation::PLAYER_ATTACK_02;
	state = State::NORMAL_ATTACK01;
	nextCombo = false;

}

void Player::TransitionCombo_01_03_State()
{
	p_update = &Player::UpdateCombo_01_03_State;
	playerAnimation = PlayerAnimation::PLAYER_ATTACK_03;
	state = State::NORMAL_ATTACK01;
	nextCombo = false;

}

void Player::TransitionCombo_PowerL_State()
{
	p_update = &Player::UpdateCombo_PowerL_State;
	playerAnimation = PlayerAnimation::PLAYER_POWER_L;
	state = State::NORMAL_ATTACK01;
	nextCombo = false;

}

void Player::TransitionCombo_PowerR_State()
{
	p_update = &Player::UpdateCombo_PowerR_State;
	playerAnimation = PlayerAnimation::PLAYER_POWER_R;
	state = State::NORMAL_ATTACK01;
	nextCombo = false;

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
	if (gamePad->GetButtonDown() & gamePad->BTN_X)
	{
		TransitionCombo_01_01_State();
	}
	//射撃入力
	if (gamePad->GetButtonDown() & gamePad->BTN_RIGHT_TRIGGER
		|| mouse->GetButtonDown() & mouse->BTN_RIGHT_CLICK)
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
	else if (ax < 0) playerAnimation = PlayerAnimation::PLAYER_MOVE_LEFT;
	if (ay > 0) playerAnimation = PlayerAnimation::PLAYER_MOVE_FORWARD;
	else if (ay < 0) playerAnimation = PlayerAnimation::PLAYER_MOVE_BACK;

	if (!InputMove(elapsedTime) && isGround)
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
	if (gamePad->GetButtonDown() & gamePad->BTN_X)
	{
	}
	//射撃入力
	if (gamePad->GetButtonDown() & gamePad->BTN_RIGHT_TRIGGER
		|| mouse->GetButtonDown() & mouse->BTN_RIGHT_CLICK)
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
	if (gamePad->GetButtonDown() & GamePad::BTN_LEFT_TRIGGER)
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
	InputAvoidance();

	//攻撃入力
	if (gamePad->GetButtonDown() & gamePad->BTN_X)
	{
	}

	//速力処理更新
	UpdateVelocity(elapsedTime, position);

}

void Player::UpdateAvoidanceState(float elapsedTime)
{
	//徐々に速度を落としていく
	velocity.x /= 2.0f;
	velocity.z /= 2.0f;

	//速力処理更新
	UpdateVelocity(elapsedTime, position);
	//if (model->animations.anime_param.frame_index > 33 / 2)
	if (param.avoidanceTimer > 20)
	{
		//地面に足がついたフレームからはさらに速度落とす
		velocity.x /= 2.0f;
		velocity.z /= 2.0f;
	}
	else
	{
		//向いている方向に速度を足す
		velocity.x = (Math::get_posture_forward(orientation) * (param.avoidanceSpeed)).x;
		velocity.z = (Math::get_posture_forward(orientation) * (param.avoidanceSpeed)).z;

	}

	//遷移処理
	//if (model->anime_param.frame_index > 35 / 2)
	if(param.avoidanceTimer > 30)
	{
		//ジャンプステートへ移行
		TransitionJumpState();
		// MOVEステートへ移行
		if (InputMove(elapsedTime))
		{
			TransitionMoveState();
			return;
		}
	}

	//if (model->is_end_animation())
	if (param.avoidanceTimer > 60)
	{
		TransitionIdleState();
	}
	param.avoidanceTimer++;
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

	//飛行入力
	InputWing();
	//攻撃入力
	if (gamePad->GetButtonDown() & gamePad->BTN_X)
	{
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
	if (gamePad->GetButtonUp() & gamePad->BTN_RIGHT_TRIGGER
		|| mouse->GetButtonUp() & mouse->BTN_RIGHT_CLICK)
	{
		TransitionIdleState();
	}

	float ax = gamePad->GetAxis_LX();
	float ay = gamePad->GetAxis_LY();

	if (ax > 0) playerAnimation = PlayerAnimation::PLAYER_SHOT_RIGHT;
	else if (ax < 0) playerAnimation = PlayerAnimation::PLAYER_SHOT_LEFT;
	if (ay > 0) playerAnimation = PlayerAnimation::PLAYER_SHOT_FORWARD;
	else if (ay < 0) playerAnimation = PlayerAnimation::PLAYER_SHOT_BACK;

	InputMove(elapsedTime, param.floatingValue, 1);

	//ジャンプ入力
	//InputJump();
	//回避入力
	//InputAvoidance();
	//飛行入力
	InputWing();
	//攻撃入力
	if (gamePad->GetButtonDown() & gamePad->BTN_X)
	{
	}

	//速力処理更新
	UpdateVelocity(elapsedTime, position);
}


void Player::UpdateCombo_01_01_State(float elapsedTime)
{
	if (gamePad->GetButtonDown() & gamePad->BTN_X)
	{
		nextCombo = true;
	}

	if (0.23f < time && nextCombo)
	{
		TransitionCombo_01_02_State();
	}

	if (model->GetIsEndAnimation())
	{
		TransitionIdleState();
	}
}

void Player::UpdateCombo_01_02_State(float elapsedTime)
{
	if (gamePad->GetButtonDown() & gamePad->BTN_X)
	{
		nextCombo = true;
	}

	if (0.3f < time && nextCombo)
	{
		TransitionCombo_01_03_State();
	}
	if (model->GetIsEndAnimation())
	{
		TransitionIdleState();
	}
}

void Player::UpdateCombo_01_03_State(float elapsedTime)
{
	if (gamePad->GetButtonDown() & gamePad->BTN_X)
	{
		nextCombo = true;
	}

	if (0.48f < time && nextCombo)
	{
		TransitionCombo_01_03_State();
	}
	if (model->GetIsEndAnimation())
	{
		TransitionIdleState();
	}
}

void Player::UpdateCombo_PowerL_State(float elapsedTime)
{
	if (0.6f < time)
	{
		if (gamePad->GetButtonDown() & gamePad->BTN_X)
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

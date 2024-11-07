#include "player.h"
#include "operators.h"


void Player::TransitionIdleState()
{
	p_update = &Player::UpdateIdleState;
	state = State::IDLE;
	playerAnimation = PlayerAnimation::PLAYER_IDLE;
	model->SetIsLoop(true);
}

void Player::TransitionMoveState()
{
	p_update = &Player::UpdateMoveState;
	state = State::MOVE;
	model->SetIsLoop(true);

}

void Player::TransitionWingState()
{
	p_update = &Player::UpdateWingState;
	playerAnimation = PlayerAnimation::PLAYER_WING_START;
	state = State::WING;
	model->SetIsLoop(false);

}

void Player::TransitionWing_to_IdleState()
{

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
	model->SetIsLoop(false);
	state = State::JUMP;

}

void Player::TransitionShotState()
{
	p_update = &Player::UpdateShotState;
	//model->play_animation(PlayerAnimation::PLAYER_JUMP, false, 0.1f);
	state = State::SHOT;

}

void Player::TransitionAttack01State()
{
	p_update = &Player::UpdateAttack01State;
	model->SetIsLoop(false);
	playerAnimation = PlayerAnimation::PLAYER_KILL_ATTACK_R01;
	state = State::NORMAL_ATTACK01;

}

void Player::UpdateIdleState(float elapsedTime)
{
	//model->animate(PlayerAnimation::PLAYER_IDLE, anime_time, model->nodes, true);
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
		TransitionAttack01State();
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

	//速力処理更新
	UpdateVelocity(elapsedTime, position);
}

void Player::UpdateWingState(float elapsedTime)
{
	if (playerAnimation == PlayerAnimation::PLAYER_WING_START)
	{
		if (model->GetIsEndAnimation())
		{
			playerAnimation = playerAnimation_transition = PlayerAnimation::PLAYER_WING;
			model->SetIsLoop(true);
		}
	}
	else
	{
		//向いている方向に速度を足す
		velocity.x = (forward * (param.wingSpeed)).x;
		velocity.y = (forward * (param.wingSpeed)).y;
		velocity.z = (forward * (param.wingSpeed)).z;

		if (!InputMoveWing(elapsedTime) && isGround)
		{
			//TransitionWing_to_IdleState();
		}
	}
	if (gamePad->GetButtonDown() & GamePad::BTN_LEFT_TRIGGER)
	{
		playerAnimation = PlayerAnimation::PLAYER_WING_END;
		model->SetIsLoop(false);
	}
	if (playerAnimation == PlayerAnimation::PLAYER_WING_END)
	{
		if (model->GetIsEndAnimation())
		{
			playerAnimation_transition = PlayerAnimation::PLAYER_IDLE;
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
			model->SetIsLoop(true);
		}
	}

	if (isGround)
	{
		//あとで一定の速度で地面に当たると着地アニメーションを再生するようにする
		if (velocity.y > 10.0f)
		{
			TransitionIdleState();
		}
		else
		{
			//is_end_animation = true;
			playerAnimation = PlayerAnimation::PLAYER_JUMP_END;
			model->SetIsLoop(false);

			if (model->GetIsEndAnimation() && playerAnimation == PlayerAnimation::PLAYER_JUMP_END)
			{
				playerAnimation_transition = PlayerAnimation::PLAYER_IDLE;
				TransitionIdleState();
			}
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

void Player::UpdateShotState(float elapsedTime)
{
}

void Player::UpdateAttack01State(float elapsedTime)
{
	if (gamePad->GetButtonDown() & gamePad->BTN_X)
	{
	}
	if (model->GetIsEndAnimation())
	{
		TransitionIdleState();
	}
}

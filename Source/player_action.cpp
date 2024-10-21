#include "player.h"
#include "operators.h"


void Player::TransitionIdleState()
{
	p_update = &Player::UpdateIdleState;
	//model->animate(PlayerAnimation::PLAYER_IDLE, anime_time,model->nodes, true);
	state = State::IDLE;
	playerAnimation = PlayerAnimation::PLAYER_IDLE;
	anime_time = 0.0f;
}

void Player::TransitionMoveState()
{
	p_update = &Player::UpdateMoveState;
	//model->play_animation(PlayerAnimation::PLAYER_RUN, true);
	state = State::MOVE;

}

void Player::TransitionAvoidanceState()
{
	p_update = &Player::UpdateAvoidanceState;
	//model->play_animation(PlayerAnimation::PLAYER_ROLL, false, 0.1f);
	state = State::ROLL;
	param.avoidanceTimer = 0;
}

void Player::TransitionJumpState()
{
	p_update = &Player::UpdateJumpState;
	//model->play_animation(PlayerAnimation::PLAYER_JUMP, false, 0.1f);
	state = State::JUMP;

}

void Player::TransitionShotState()
{
	p_update = &Player::UpdateShotState;
	//model->play_animation(PlayerAnimation::PLAYER_JUMP, false, 0.1f);
	state = State::SHOT;

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

	//攻撃入力
	if (gamePad->GetButtonDown() & gamePad->BTN_X)
	{
	}

	//速力処理更新
	UpdateVelocity(elapsedTime, position);

}

void Player::UpdateMoveState(float elapsedTime)
{
	if (!InputMove(elapsedTime) && isGround)
	{
		TransitionIdleState();
	}

	//ジャンプ入力
	InputJump();
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
		velocity.x += (Math::get_posture_forward(orientation) * (param.avoidanceSpeed)).x;
		velocity.z += (Math::get_posture_forward(orientation) * (param.avoidanceSpeed)).z;

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
	InputMove(elapsedTime);

	if (isGround)
	{
		TransitionIdleState();
	}

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

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
	//�W�����v����
	InputJump();
	//������
	InputAvoidance();

	//�U������
	if (gamePad->GetButtonDown() & gamePad->BTN_X)
	{
	}

	//���͏����X�V
	UpdateVelocity(elapsedTime, position);

}

void Player::UpdateMoveState(float elapsedTime)
{
	if (!InputMove(elapsedTime) && isGround)
	{
		TransitionIdleState();
	}

	//�W�����v����
	InputJump();
	//������
	InputAvoidance();

	//�U������
	if (gamePad->GetButtonDown() & gamePad->BTN_X)
	{
	}

	//���͏����X�V
	UpdateVelocity(elapsedTime, position);
}

void Player::UpdateAvoidanceState(float elapsedTime)
{
	//���X�ɑ��x�𗎂Ƃ��Ă���
	velocity.x /= 2.0f;
	velocity.z /= 2.0f;

	//���͏����X�V
	UpdateVelocity(elapsedTime, position);
	//if (model->animations.anime_param.frame_index > 33 / 2)
	if (param.avoidanceTimer > 20)
	{
		//�n�ʂɑ��������t���[������͂���ɑ��x���Ƃ�
		velocity.x /= 2.0f;
		velocity.z /= 2.0f;
	}
	else
	{
		//�����Ă�������ɑ��x�𑫂�
		velocity.x += (Math::get_posture_forward(orientation) * (param.avoidanceSpeed)).x;
		velocity.z += (Math::get_posture_forward(orientation) * (param.avoidanceSpeed)).z;

	}

	//�J�ڏ���
	//if (model->anime_param.frame_index > 35 / 2)
	if(param.avoidanceTimer > 30)
	{
		//�W�����v�X�e�[�g�ֈڍs
		TransitionJumpState();
		// MOVE�X�e�[�g�ֈڍs
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

	//�U������
	if (gamePad->GetButtonDown() & gamePad->BTN_X)
	{
	}


	//���͏����X�V
	UpdateVelocity(elapsedTime, position);

}

void Player::UpdateShotState(float elapsedTime)
{
}

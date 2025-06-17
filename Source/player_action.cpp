#include "player.h"
#include "operators.h"
#include "bullet_straight.h"

void Player::TransitionIdleState()
{
	//待機状態へ偏移
	p_update = &Player::UpdateIdleState;
	state = STATE::IDLE;
	playerAnimation = PlayerAnimation::PLAYER_IDLE;
}


void Player::TransitionMoveState()
{
	//移動状態へ偏移
	p_update = &Player::UpdateMoveState;
	state = STATE::MOVE;
	charaParam.acceleration = accelerationState[ACCELERATION_STATE::MOVE];

}

void Player::TransitionWingState()
{
	//飛行状態へ偏移
	p_update = &Player::UpdateWingState;
	playerAnimation = PlayerAnimation::PLAYER_WING_START;
	state = STATE::WING;

	//加速度を飛行状態に設定
	charaParam.acceleration = accelerationState[ACCELERATION_STATE::WING];
	isBoost = false;
}

void Player::TransitionAvoidanceState()
{
	//ブースト状態へ偏移
	p_update = &Player::UpdateAvoidanceState;
	playerAnimation = PlayerAnimation::PLAYER_MOVE_FORWARD;
	state = STATE::BOOST;

	//加速度と最大速度をブースト状態に設定
	charaParam.maxMoveSpeed = param.avoidanceSpeed;
	charaParam.acceleration = accelerationState[ACCELERATION_STATE::AVOIDANCE];

	//ブースト効果音
	audios[PLAYER_SE::SE_BOOST]->play();
	audios[PLAYER_SE::SE_BOOST]->volume(0.5f);

	//ラジアルブラー設定
	player_radialBlur_constant.blurStrength = 1.0f;
	player_radialBlur_constant.blurRadius = 1.0f;
	radialTimer = player_radialBlur_constant.blurTimer = 0.5f;

	//ブーストタイマーを一定量減らす
	param.boostTimer -= 2.5f;
	param.avoidanceTimer = 0;
}

void Player::TransitionJumpState()
{
	//ジャンプ状態へ偏移
	p_update = &Player::UpdateJumpState;
	if(isGround)
		playerAnimation = PlayerAnimation::PLAYER_JUMP_START;
	else
		playerAnimation = PlayerAnimation::PLAYER_JUMP;

	//加速度をジャンプ状態に設定
	charaParam.acceleration = accelerationState[ACCELERATION_STATE::MOVE];
	state = STATE::JUMP;

}

void Player::TransitionLandingState()
{
	//着地状態へ偏移
	p_update = &Player::UpdateLandingState;
	playerAnimation = PlayerAnimation::PLAYER_JUMP_END;
	state = STATE::JUMP;

}

void Player::TransitionShotState()
{
	//射撃状態へ偏移
	p_update = &Player::UpdateShotState;
	playerAnimation = PlayerAnimation::PLAYER_SHOT_IDLE;
	state = STATE::SHOT;

}

void Player::TransitionCombo_01_01_State()
{
	//コンボ1状態へ偏移
	p_update = &Player::UpdateCombo_01_01_State;
	playerAnimation = PlayerAnimation::PLAYER_ATTACK_01;
	resetTrail = true;
	state = STATE::RIGHT_ATTACK;

	//攻撃パラメータ設定
	attackParam = param.combo_1;
	nextCombo = false;

	//斬撃音再生
	audios[SE_SABER]->play();
	audios[SE_SABER]->volume(1.0f);

}

void Player::TransitionCombo_01_02_State()
{
	//コンボ2状態へ偏移
	p_update = &Player::UpdateCombo_01_02_State;
	playerAnimation = PlayerAnimation::PLAYER_ATTACK_02;
	resetTrail = true;
	state = STATE::LEFT_ATTACK;

	//攻撃パラメータ設定
	attackParam = param.combo_2;
	nextCombo = false;

	//斬撃音再生
	audios[SE_SABER]->play();
	audios[SE_SABER]->volume(1.0f);

}

void Player::TransitionCombo_01_03_State()
{
	//コンボ3状態へ偏移
	p_update = &Player::UpdateCombo_01_03_State;
	playerAnimation = PlayerAnimation::PLAYER_ATTACK_03;
	resetTrail = true;
	state = STATE::RIGHT_ATTACK;

	//攻撃パラメータ設定
	attackParam = param.combo_3;
	nextCombo = false;

	//斬撃音再生
	audios[SE_SABER]->play();
	audios[SE_SABER]->volume(1.0f);

}

void Player::TransitionDamageState()
{
	//ダメージ状態へ偏移
	p_update = &Player::UpdateDamageState;
	playerAnimation = PlayerAnimation::PLAYER_DAMAGE;
	state = STATE::DAMAGE;
	isGlitch_CA = true;
	glitch_CATimer = 0.03f;
}

void Player::TransitionDeadState()
{
	//死亡状態へ偏移
	p_update = &Player::UpdateDeadState;
	playerAnimation = PlayerAnimation::PLAYER_DEAD;
	state = STATE::DAMAGE;

}

void Player::UpdateIdleState(float elapsedTime)
{
	//動いていたら移動状態へ偏移
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

	//コンボ入力
	if (gamePad->GetButtonDown() & gamePad->BTN_X)
	{
		TransitionCombo_01_01_State();
	}
	//射撃入力
	if (gamePad->GetButtonDown() & gamePad->BTN_RIGHT_TRIGGER)
	{
		TransitionShotState();
	}

	//速力処理更新
	UpdateVelocity(elapsedTime, position);

}

void Player::UpdateMoveState(float elapsedTime)
{
	//スティック入力
	float ax = gamePad->GetAxis_LX();
	float ay = gamePad->GetAxis_LY();

	//アニメーション設定
	if (ax > 0) playerAnimation = PlayerAnimation::PLAYER_MOVE_RIGHT;
	else if (ax < 0)
		playerAnimation = PlayerAnimation::PLAYER_MOVE_LEFT;
	if (ay > 0) playerAnimation = PlayerAnimation::PLAYER_MOVE_FORWARD;
	else if (ay < 0) playerAnimation = PlayerAnimation::PLAYER_MOVE_BACK;
	
	//動いていたら移動状態へ偏移
	if (!InputMove(elapsedTime))
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
		TransitionCombo_01_01_State();
	}
	//射撃入力
	if (gamePad->GetButtonDown() & gamePad->BTN_RIGHT_TRIGGER)
	{
		TransitionShotState();
	}

	//速力処理更新
	UpdateVelocity(elapsedTime, position);
}

void Player::UpdateWingState(float elapsedTime)
{
	// 飛行開始アニメーションの終了判定
	if (playerAnimation == PlayerAnimation::PLAYER_WING_START)
	{
		if (model->GetIsEndAnimation())
		{
			playerAnimation = PlayerAnimation::PLAYER_WING;
		}
	}
	else
	{
		// 向いている方向に速度を加算
		velocity.x = (forward * (param.wingSpeed)).x;
		velocity.y = (forward * (param.wingSpeed)).y;
		velocity.z = (forward * (param.wingSpeed)).z;

		// ラジアルブラー設定
		player_radialBlur_constant.blurStrength = 0.2f;
		player_radialBlur_constant.blurRadius = 1.0f;
		radialTimer = player_radialBlur_constant.blurTimer = 0.5f;

		// 飛行時の移動入力処理
		InputMoveWing(elapsedTime);
	}

	// 飛行終了入力処理
	if (gamePad->GetButtonDown() & GamePad::BTN_B)
	{
		playerAnimation = PlayerAnimation::PLAYER_WING_END;
	}

	// 飛行終了アニメーションの終了判定
	if (playerAnimation == PlayerAnimation::PLAYER_WING_END)
	{
		if (model->GetIsEndAnimation())
		{
			TransitionIdleState();
		}
	}

	// 速度処理の更新
	UpdateVelocity(elapsedTime, position);
}

void Player::UpdateAvoidanceState(float elapsedTime)
{	
	// 進行方向のベクトル取得
	float ax = gamePad->GetAxis_LX();
	float ay = gamePad->GetAxis_LY();

	// 回避方向に応じたアニメーションの設定
	if (ax > 0) playerAnimation = PlayerAnimation::PLAYER_MOVE_RIGHT;
	else if (ax < 0) playerAnimation = PlayerAnimation::PLAYER_MOVE_LEFT;
	if (ay > 0) playerAnimation = PlayerAnimation::PLAYER_MOVE_FORWARD;
	else if (ay < 0) playerAnimation = PlayerAnimation::PLAYER_MOVE_BACK;

	// 徐々に速度を落とす
	velocity.x /= 2.0f;
	velocity.z /= 2.0f;

	// 速度処理の更新
	UpdateVelocity(elapsedTime, position);

	// 回避時間経過後の処理
	if (param.avoidanceTimer > 0.1f)
	{
		charaParam.maxMoveSpeed = charaParam.moveSpeed;

		// 移動入力がある場合は移動状態へ遷移、なければ待機状態へ遷移
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

	// 回避時間の更新
	param.avoidanceTimer += elapsedTime;
}
void Player::UpdateJumpState(float elapsedTime)
{
	// ジャンプ開始アニメーションの終了判定
	if (playerAnimation == PlayerAnimation::PLAYER_JUMP_START)
	{
		if (model->GetIsEndAnimation())
		{
			playerAnimation = PlayerAnimation::PLAYER_JUMP;
		}
	}

	// 移動入力処理
	InputMove(elapsedTime);

	// ブースト状態なら待機状態へ遷移
	if (isBoost)
	{
		TransitionIdleState();
	}

	// 飛行入力処理
	InputWing();

	// 回避入力処理
	InputAvoidance();

	// 攻撃入力処理
	if (gamePad->GetButtonDown() & gamePad->BTN_X)
	{
		TransitionCombo_01_01_State();
	}

	// 速度処理の更新
	UpdateVelocity(elapsedTime, position);
}

void Player::UpdateLandingState(float elapsedTime)
{
	// 着地アニメーションが終了したら待機状態へ遷移
	if (model->GetIsEndAnimation())
	{
		TransitionIdleState();
	}
}

void Player::UpdateShotState(float elapsedTime)
{
	// コントローラーのスティック入力を取得
	float ax = gamePad->GetAxis_LX();
	float ay = gamePad->GetAxis_LY();

	// 射撃アニメーションの選択
	playerAnimation = PlayerAnimation::PLAYER_SHOT_IDLE;
	if (ax > 0) playerAnimation = PlayerAnimation::PLAYER_SHOT_RIGHT;
	else if (ax < 0) playerAnimation = PlayerAnimation::PLAYER_SHOT_LEFT;
	if (ay > 0) playerAnimation = PlayerAnimation::PLAYER_SHOT_FORWARD;
	else if (ay < 0) playerAnimation = PlayerAnimation::PLAYER_SHOT_BACK;

	// 射撃を解除した場合、待機状態へ遷移
	if (gamePad->GetButtonUp() & gamePad->BTN_RIGHT_TRIGGER)
	{
		TransitionIdleState();
	}

	// 射撃処理
	if (time <= 0)
	{
		InputShot();
	}

	// 移動処理（攻撃時の値を考慮）
	InputMove(elapsedTime, param.floatingValue, 1);

	// ジャンプ入力
	InputJump();
	// 回避入力
	InputAvoidance();
	// 飛行入力
	InputWing();
	// 攻撃入力（コンボ開始）
	if (gamePad->GetButtonDown() & gamePad->BTN_X
		//|| mouse->GetButtonDown() & mouse->BTN_LEFT_CLICK
		)
	{
		TransitionCombo_01_01_State();
	}

	// 速度処理の更新
	UpdateVelocity(elapsedTime, position);
}


void Player::UpdateCombo_01_01_State(float elapsedTime)
{
	// 先行入力（次のコンボ入力を検知）
	if (gamePad->GetButtonDown() & gamePad->BTN_X)
	{
		nextCombo = true;
	}

	// 一定時間経過後に攻撃判定をオン
	if (0.023f < time && !attackParam.isAttack)
	{
		attackParam.isAttack = true;
	}

	// 先行入力があれば次のコンボへ遷移
	if (0.173f < time && nextCombo)
	{
		TransitionCombo_01_02_State();
		attackParam.isAttack = false;
	}

	// 攻撃判定をオフにするタイミング
	if (0.15f < time)
	{
		attackParam.isAttack = false;
	}

	// アニメーションが終了したら待機状態へ遷移
	if (model->GetIsEndAnimation())
	{
		TransitionIdleState();
		attackParam.isAttack = false;
	}

	// 移動処理（攻撃中の移動速度を考慮）
	InputMove(elapsedTime, param.attackMoveSpeed, 1);

	// 回避入力処理
	InputAvoidance();

	// 速度処理の更新
	UpdateVelocity(elapsedTime, position);
}

void Player::UpdateCombo_01_02_State(float elapsedTime)
{
	// 先行入力のチェック
	if (gamePad->GetButtonDown() & gamePad->BTN_X)
	{
		nextCombo = true;
	}

	// 攻撃判定のオン
	if (0.03f < time && !attackParam.isAttack)
	{
		attackParam.isAttack = true;
	}

	// 先行入力されていたら次のコンボへ遷移
	if (0.2f < time && nextCombo)
	{
		TransitionCombo_01_03_State();
		attackParam.isAttack = false;
	}

	// 攻撃判定のオフ
	if (0.175f < time)
	{
		attackParam.isAttack = false;
	}

	// アニメーション終了時に待機状態へ遷移
	if (model->GetIsEndAnimation())
	{
		attackParam.isAttack = false;
		TransitionIdleState();
	}

	// 移動入力処理
	InputMove(elapsedTime, param.attackMoveSpeed, 1);

	// 回避入力処理
	InputAvoidance();

	// 速力更新
	UpdateVelocity(elapsedTime, position);
}

void Player::UpdateCombo_01_03_State(float elapsedTime)
{
	if (gamePad->GetButtonDown() & gamePad->BTN_X)
	{
		nextCombo = true;
	}


	if (0.325f < time && !attackParam.isAttack)
	{
		attackParam.isAttack = true;
	}

	if (0.65f < time)
	{
		attackParam.isAttack = false;
	}

	if (model->GetIsEndAnimation())
	{
		TransitionIdleState();
		attackParam.isAttack = false;

	}
	InputMove(elapsedTime, param.attackMoveSpeed, 1);

	//回避入力
	InputAvoidance();

	UpdateVelocity(elapsedTime, position);
}

void Player::UpdateDamageState(float elapsedTime)
{
	static float damageTimer = 0.0f;

	// アニメーション終了し、一定時間経過後にIDLE状態へ遷移
	if (model->GetIsEndAnimation() && damageTimer > 0.2f)
	{
		TransitionIdleState();
		isGlitch_CA = false;
		damageTimer = 0.0f;
	}

	// ダメージタイマー更新
	damageTimer += elapsedTime;

	// 速力更新
	UpdateVelocity(elapsedTime, position);
}

void Player::UpdateDeadState(float elapsedTime)
{
	// アニメーション終了時に死亡フラグを設定
	if (model->GetIsEndAnimation())
	{
		isDead = true;
	}
}

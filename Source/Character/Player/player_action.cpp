#include "Character/Player/player.h"
#include "User/operators.h"
#include "Bullet/bullet_straight.h"

void Player::TransitionIdleState()
{
	//待機状態へ偏移
	pUpdate = &Player::UpdateIdleState;

	//状態とアニメーションをIDLEへ偏移
	state = STATE::IDLE;
	playerAnimation = PlayerAnimation::PLAYER_IDLE;
}

void Player::TransitionMoveState()
{
	//移動状態へ偏移
	pUpdate = &Player::UpdateMoveState;

	//状態をMOVEへ偏移
	state = STATE::MOVE;

	//加速度を移動状態に設定
	charaParam.acceleration = accelerationState[ToInt(ACCELERATION_STATE::MOVE)];
}

void Player::TransitionWingState()
{
	//飛行状態へ偏移
	pUpdate = &Player::UpdateWingState;

	//状態とアニメーションをWINGへ偏移
	state = STATE::WING;
	playerAnimation = PlayerAnimation::PLAYER_WING_START;

	//加速度を飛行状態に設定
	charaParam.acceleration = accelerationState[ToInt(ACCELERATION_STATE::WING)];
	
	//ブースト状態を解除
	isBoost = false;
}

void Player::TransitionBoostState()
{
	//ブースト状態へ偏移
	pUpdate = &Player::UpdateBoostState;

	//状態とアニメーションをBOOSTへ偏移
	state = STATE::BOOST;
	playerAnimation = PlayerAnimation::PLAYER_MOVE_FORWARD;

	//加速度と最大速度をブースト状態に設定
	charaParam.maxMoveSpeed = param.boostSpeed;
	charaParam.acceleration = accelerationState[ToInt(ACCELERATION_STATE::BOOST)];

	//ブースト効果音
	audios[ToInt(PLAYER_SE::SE_BOOST)]->play();
	audios[ToInt(PLAYER_SE::SE_BOOST)]->volume(0.5f);

	//ラジアルブラー設定
	player_RadialBlurConstant.blurStrength = 1.0f;
	player_RadialBlurConstant.blurRadius = 1.0f;
	radialTimer = player_RadialBlurConstant.blurTimer = 0.5f;

	//ブーストタイマーを減少、回避タイマーをリセット
	param.boostTimer -= 2.5f;
	param.avoidanceTimer = 0;
}

void Player::TransitionJumpState()
{
	//ジャンプ状態へ偏移
	pUpdate = &Player::UpdateJumpState;

	//地面にいるかどうかでアニメーションを変更
	if(isGround)
		playerAnimation = PlayerAnimation::PLAYER_JUMP_START;
	else
		playerAnimation = PlayerAnimation::PLAYER_JUMP;

	//状態をJUMPへ偏移
	state = STATE::JUMP;

	//加速度をジャンプ状態に設定
	charaParam.acceleration = accelerationState[ToInt(ACCELERATION_STATE::MOVE)];
}

void Player::TransitionLandingState()
{
	//着地状態へ偏移
	pUpdate = &Player::UpdateLandingState;

	//状態とアニメーションをJUMP_ENDへ偏移
	state = STATE::JUMP;
	playerAnimation = PlayerAnimation::PLAYER_JUMP_END;
}

void Player::TransitionShotState()
{
	//射撃状態へ偏移
	pUpdate = &Player::UpdateShotState;

	//状態とアニメーションをSHOTへ偏移
	state = STATE::SHOT;
	playerAnimation = PlayerAnimation::PLAYER_SHOT_IDLE;
}

void Player::TransitionCombo01State()
{
	//コンボ1状態へ偏移
	pUpdate = &Player::UpdateComboState;

	//状態とアニメーションをATTACK_01へ偏移
	state = STATE::RIGHT_ATTACK;
	playerAnimation = PlayerAnimation::PLAYER_ATTACK_01;
	
	// 軌跡をリセット
	resetTrail = true;

	//攻撃パラメータをコンボ1に設定
	attackParam = param.combo_1;
	//次のコンボを無効化
	nextCombo = false;
	//今のコンボ設定
	nowCombo = COMBO::ATTACK01;

	//斬撃音再生
	audios[ToInt(PLAYER_SE::SE_SABER)]->play();
	audios[ToInt(PLAYER_SE::SE_SABER)]->volume(SOUND_VOLUME_SABER);
}

void Player::TransitionCombo02State()
{
	//コンボ2状態へ偏移
	pUpdate = &Player::UpdateComboState;

	//状態とアニメーションをATTACK_02へ偏移
	state = STATE::LEFT_ATTACK;
	playerAnimation = PlayerAnimation::PLAYER_ATTACK_02;

	// 軌跡をリセット
	resetTrail = true;

	//攻撃パラメータをコンボ2に設定
	attackParam = param.combo_2;
	//次のコンボを無効化
	nextCombo = false;
	//今のコンボ設定
	nowCombo = COMBO::ATTACK02;

	//斬撃音再生
	audios[ToInt(PLAYER_SE::SE_SABER)]->play();
	audios[ToInt(PLAYER_SE::SE_SABER)]->volume(SOUND_VOLUME_SABER);
}

void Player::TransitionCombo03State()
{
	//コンボ3状態へ偏移
	pUpdate = &Player::UpdateComboState;

	//状態とアニメーションをATTACK_03へ偏移
	state = STATE::RIGHT_ATTACK;
	playerAnimation = PlayerAnimation::PLAYER_ATTACK_03;
	// 軌跡をリセット
	resetTrail = true;

	//攻撃パラメータをコンボ3に設定
	attackParam = param.combo_3;
	//次のコンボを無効化
	nextCombo = false;
	//今のコンボ設定
	nowCombo = COMBO::ATTACK03;

	//斬撃音再生
	audios[ToInt(PLAYER_SE::SE_SABER)]->play();
	audios[ToInt(PLAYER_SE::SE_SABER)]->volume(SOUND_VOLUME_SABER);
}

void Player::TransitionDamageState()
{
	//ダメージ状態へ偏移
	pUpdate = &Player::UpdateDamageState;

	//状態とアニメーションをDAMAGEへ偏移
	state = STATE::DAMAGE;
	playerAnimation = PlayerAnimation::PLAYER_DAMAGE;

	//グリッチエフェクトの開始
	isGlitch_CA = true;
	glitch_CATimer = 0.03f;
}

void Player::TransitionDeadState()
{
	//死亡状態へ偏移
	pUpdate = &Player::UpdateDeadState;

	//状態とアニメーションをDEADへ偏移
	state = STATE::DEAD;
	playerAnimation = PlayerAnimation::PLAYER_DEAD;

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
	InputBoost();
	//飛行入力
	InputWing();

	//攻撃入力
	if (gamePad->GetButtonDown() & gamePad->BTN_X)
	{
		TransitionCombo01State();
	}
	//射撃入力
	if (gamePad->GetButtonDown() & gamePad->BTN_RIGHT_TRIGGER)
	{
		TransitionShotState();
	}

	//速力更新
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
	InputBoost();
	//飛行入力
	InputWing();
	//攻撃入力
	if (gamePad->GetButtonDown() & gamePad->BTN_X)
	{
		TransitionCombo01State();
	}
	//射撃入力
	if (gamePad->GetButtonDown() & gamePad->BTN_RIGHT_TRIGGER)
	{
		TransitionShotState();
	}

	//速力更新
	UpdateVelocity(elapsedTime, position);
}

void Player::UpdateWingState(float elapsedTime)
{
	//飛行開始アニメーションの終了判定
	if (playerAnimation == PlayerAnimation::PLAYER_WING_START)
	{
		if (model->GetIsEndAnimation())
		{
			playerAnimation = PlayerAnimation::PLAYER_WING;
		}
	}
	else
	{
		//向いている方向に速度を加算
		velocity.x = (forward * (param.wingSpeed)).x;
		velocity.y = (forward * (param.wingSpeed)).y;
		velocity.z = (forward * (param.wingSpeed)).z;

		//ラジアルブラー設定
		player_RadialBlurConstant.blurStrength = 0.2f;
		player_RadialBlurConstant.blurRadius = 1.0f;
		radialTimer = player_RadialBlurConstant.blurTimer = 0.5f;

		//飛行時の移動入力処理
		InputMoveWing(elapsedTime);
	}

	//飛行終了入力処理
	if (gamePad->GetButtonDown() & GamePad::BTN_B)
	{
		playerAnimation = PlayerAnimation::PLAYER_WING_END;
	}

	//飛行終了アニメーションの終了判定
	if (playerAnimation == PlayerAnimation::PLAYER_WING_END)
	{
		if (model->GetIsEndAnimation())
		{
			TransitionIdleState();
		}
	}

	//速力更新
	UpdateVelocity(elapsedTime, position);
}

void Player::UpdateBoostState(float elapsedTime)
{	
	//進行方向のベクトル取得
	float ax = gamePad->GetAxis_LX();
	float ay = gamePad->GetAxis_LY();

	//回避方向に応じたアニメーションの設定
	if (ax > 0) playerAnimation = PlayerAnimation::PLAYER_MOVE_RIGHT;
	else if (ax < 0) playerAnimation = PlayerAnimation::PLAYER_MOVE_LEFT;
	if (ay > 0) playerAnimation = PlayerAnimation::PLAYER_MOVE_FORWARD;
	else if (ay < 0) playerAnimation = PlayerAnimation::PLAYER_MOVE_BACK;

	//徐々に速度を落とす
	velocity.x /= 2.0f;
	velocity.z /= 2.0f;

	//速力更新
	UpdateVelocity(elapsedTime, position);

	//回避時間経過後の処理
	if (param.avoidanceTimer > 0.1f)
	{
		charaParam.maxMoveSpeed = charaParam.moveSpeed;

		//移動入力がある場合は移動状態へ遷移、なければ待機状態へ遷移
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

	//回避時間の更新
	param.avoidanceTimer += elapsedTime;
}
void Player::UpdateJumpState(float elapsedTime)
{
	//ジャンプ開始アニメーションの終了判定
	if (playerAnimation == PlayerAnimation::PLAYER_JUMP_START)
	{
		if (model->GetIsEndAnimation())
		{
			playerAnimation = PlayerAnimation::PLAYER_JUMP;
		}
	}

	//移動入力
	InputMove(elapsedTime);

	//ブースト状態なら待機状態へ遷移
	if (isBoost)
	{
		TransitionIdleState();
	}

	//飛行入力
	InputWing();

	//回避入力
	InputBoost();

	//攻撃入力
	if (gamePad->GetButtonDown() & gamePad->BTN_X)
	{
		TransitionCombo01State();
	}

	//速力更新
	UpdateVelocity(elapsedTime, position);
}

void Player::UpdateLandingState(float elapsedTime)
{
	//着地アニメーションが終了したら待機状態へ遷移
	if (model->GetIsEndAnimation())
	{
		TransitionIdleState();
	}
}

void Player::UpdateShotState(float elapsedTime)
{
	//コントローラーのスティック入力を取得
	float ax = gamePad->GetAxis_LX();
	float ay = gamePad->GetAxis_LY();

	// 射撃アニメーションの選択
	playerAnimation = PlayerAnimation::PLAYER_SHOT_IDLE;
	if (ax > 0) playerAnimation = PlayerAnimation::PLAYER_SHOT_RIGHT;
	else if (ax < 0) playerAnimation = PlayerAnimation::PLAYER_SHOT_LEFT;
	if (ay > 0) playerAnimation = PlayerAnimation::PLAYER_SHOT_FORWARD;
	else if (ay < 0) playerAnimation = PlayerAnimation::PLAYER_SHOT_BACK;

	//射撃を解除した場合、待機状態へ遷移
	if (gamePad->GetButtonUp() & gamePad->BTN_RIGHT_TRIGGER)
	{
		TransitionIdleState();
	}

	//射撃処理
	if (time <= 0)
	{
		InputShot();
	}

	//移動処理（攻撃時の値を考慮）
	InputMove(elapsedTime, param.floatingValue, 1);

	//ジャンプ/回避/飛行入力
	InputJump();
	InputBoost();
	InputWing();

	//攻撃入力
	if (gamePad->GetButtonDown() & gamePad->BTN_X)
	{
		TransitionCombo01State();
	}

	//速力更新
	UpdateVelocity(elapsedTime, position);
}

void Player::UpdateComboState(float elapsedTime)
{
	//先行入力チェックと攻撃当たり判定のオンオフ
	CheckPreInput(nowCombo);

	//アニメーションが終了したら待機状態へ遷移
	if (model->GetIsEndAnimation())
	{
		TransitionIdleState();
		attackParam.isAttack = false;
	}

	//移動/回避処理（攻撃中の移動速度を考慮）
	InputMove(elapsedTime, param.attackMoveSpeed, 1);
	InputBoost();

	//速力更新
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

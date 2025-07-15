#include "boss.h"
#include "User/noise.h"

/*--------------------状態遷移------------------------*/
void Boss::TransitionIdleState()
{
	//待機状態へ遷移
	actUpdate = &Boss::UpdateIdleState;

	//状態とアニメーションをIDLEに設定
	state = STATE::IDLE;
	bossAnimation = BOSS_ANIMATION::BOSS_IDLE;

	//状態タイマーと持続時間を初期化
	stateTimer = 0;
	stateDuration = 2.0f;
}

void Boss::TransitionWalkState()
{
	//歩行状態へ遷移
	actUpdate = &Boss::UpdateWalkState;

	//状態とアニメーションをWALKに設定
	state = STATE::WALK;
	bossAnimation = BOSS_ANIMATION::BOSS_WALK;

	//移動速度を歩行速度に設定
	charaParam.moveSpeed = WALK_SPEED;

	//状態タイマーと持続時間を初期化
	stateTimer = 0;
	stateDuration = 3.0f;
}

void Boss::TransitionAttackTackleState()
{
	//タックル攻撃状態へ遷移
	actUpdate = &Boss::UpdateAttackTackleState;

	//状態とアニメーションをTACKLEに設定
	state = STATE::TACKLE;
	bossAnimation = BOSS_ANIMATION::BOSS_RUN;

	//攻撃パラメータをタックル用に設定
	attackParam = param.tackleParam;

	//状態タイマーと持続時間を初期化
	stateTimer = 0;
	stateDuration = 1.0f;
}

void Boss::TransitionAttackJumpState()
{
	//ジャンプ攻撃状態へ遷移
	actUpdate = &Boss::UpdateAttackJumpState;

	//状態とアニメーションをJUMPに設定
	state = STATE::JUMP;
	bossAnimation = BOSS_ANIMATION::BOSS_CHARGE;

	//加速度をジャンプ用に設定
	charaParam.acceleration = ACCELERATION_JUMP_SPEED;

	//攻撃パラメータをジャンプ攻撃用に設定
	attackParam = param.stompParam;

	//状態タイマーと持続時間を初期化
	stateTimer = 0;
	stateDuration = 2.5f;

	//チャージエフェクトを再生
	chargeEffect->Play(position, 3.0f);
}

void Boss::TransitionAttackShotStraightState()
{
	//直線射撃状態へ遷移
	actUpdate = &Boss::UpdateAttackShotStraightState;

	//状態をSHOT_Sに、アニメーションをJUMPに設定
	state = STATE::SHOT_S;
	bossAnimation = BOSS_ANIMATION::BOSS_JUMP;

	//加速度をジャンプ用に設定
	charaParam.acceleration = ACCELERATION_JUMP_SPEED;

	//連射カウントを初期化
	rapidCount = 0;

	//状態タイマーと持続時間を初期化
	stateTimer = 0;
	stateDuration = RAPIDFIRE_TIME;
}

void Boss::TransitionDamageState()
{
	//ダメージ状態へ遷移
	actUpdate = &Boss::UpdateDamageState;

	//状態とアニメーションをDAMAGEに設定
	state = STATE::DAMAGE;
	bossAnimation = BOSS_ANIMATION::BOSS_HIT;

	//状態タイマーと持続時間を初期化
	stateTimer = 0;
	stateDuration = DAMAGE_STUN_DURATION;
}

void Boss::TransitionDeadState()
{
	//死亡状態へ遷移
	actUpdate = &Boss::UpdateDeadState;

	//状態とアニメーションをDEADに設定
	state = STATE::DEAD;
	bossAnimation = BOSS_ANIMATION::BOSS_DEAD;

	//状態タイマーを初期化
	stateTimer = 0;
}

void Boss::UpdateIdleState(float elapsedTime)
{
	//状態タイマーが持続時間を超えたら、ウォーク状態に遷移
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
	DirectX::XMFLOAT3 dirTargetVec = Math::CalcVectorAtoBNormalize(position, targetPos);  //プレイヤー位置への方向ベクトルを計算
	Move(dirTargetVec.x, dirTargetVec.z, charaParam.moveSpeed);  //プレイヤー方向に移動
	Turn(elapsedTime, dirTargetVec, charaParam.turnSpeed, orientation);  //プレイヤー方向に向けて回転

	//状態タイマーが持続時間を超えたら、攻撃のルーチンへ遷移
	if (stateTimer > stateDuration)
	{
		AttackRoutine(elapsedTime);
	}

	//速度更新
	UpdateVelocity(elapsedTime, position);
}

void Boss::UpdateAttackTackleState(float elapsedTime)
{
	//タックル攻撃の状態が終了するまで目標地点を設定
	if (stateTimer < stateDuration)
	{
		targetPointPos.x = targetPos.x;
		targetPointPos.z = targetPos.z;
		return;
	}

	//目標地点までのXZ平面での距離判定
	float vx = targetPointPos.x - position.x;
	float vz = targetPointPos.z - position.z;
	float distSq = vx * vx + vz * vz;

	//攻撃フラグを設定
	attackParam.isAttack = true;
	DirectX::XMFLOAT3 pos = { position.x ,0.0f, position.z };
	DirectX::XMFLOAT3 pointPos = { targetPointPos.x ,0.0f, targetPointPos.z };
	
	//目標地点への方向ベクトルを計算
	DirectX::XMFLOAT3 dirTargetVec = Math::CalcVectorAtoBNormalize(pos, pointPos);
	
	//目標地点に向かって移動
	Move(dirTargetVec.x, dirTargetVec.z, param.runSpeed);

	//目標地点に向けて回転
	Turn(elapsedTime, dirTargetVec, charaParam.turnSpeed, orientation);

	//半径2.0fの範囲内に目標地点が入ったら、待機状態に遷移
	if (distSq < TACKLE_HIT_RADIUS * TACKLE_HIT_RADIUS)
	{
		TransitionIdleState();
		stateDuration = NORMAL_ATTACK_COOLTIME;  //通常攻撃のクールタイムを設定
		attackParam.isAttack = false;  //攻撃終了
	}

	//速度更新
	UpdateVelocity(elapsedTime, position);
}

void Boss::UpdateAttackJumpState(float elapsedTime)
{
	//ジャンプアニメーションがまだ開始されていない初期フレームは何もしない
	if (bossAnimation == BOSS_ANIMATION::BOSS_JUMP && time < JUMP_ANIM_START_WAIT)
		return;

	//状態タイマーが持続時間を超えたら、ジャンプ攻撃を行う
	if (stateTimer < stateDuration)
	{
		//目標地点までの移動速度を計算
		charaParam.moveSpeed = CalcMoveSpeed(targetPos, JUMP_MOVE_TIME);
		targetPointPos = targetPos;
		return;
	}
	else
	{
		//ジャンプアニメーションに変更
		bossAnimation = BOSS_ANIMATION::BOSS_JUMP; 
	}

	//垂直方向の目標との距離を計算し、条件を満たしたらジャンプ初速を設定
	float length = targetPointPos.y - position.y;
	if (!isJump && length > JUMP_HEIGHT_THRESHOLD && time > JUMP_ANIM_START_WAIT)
	{
		isJump = true;
		velocity.y = JUMP_SPEED;  //垂直速度にジャンプ初速を設定
	}

	//目標地点までのXZ平面での距離判定
	float vx = targetPointPos.x - position.x;
	float vz = targetPointPos.z - position.z;
	float distSq = vx * vx + vz * vz;

	//攻撃状態を有効に
	attackParam.isAttack = true;

	DirectX::XMFLOAT3 dirTargetVec{};

	// 一定距離内に到達したら着地処理へ
	if (distSq < JUMP_RADIUS * JUMP_RADIUS)
	{
		//ジャンプ攻撃アニメーションが終了したら、待機状態に遷移
		if (model->GetIsEndAnimation())
		{
			TransitionIdleState();
			attackParam.isAttack = false;		//攻撃終了
			charaParam.moveSpeed = WALK_SPEED;  //歩行速度に戻す
			
			//加速度を通常の速度に戻す
			charaParam.acceleration = ACCELERATION_NORMAL_SPEED;
			isJump = false;  //ジャンプフラグをリセット
		}
		else
		{
			//アニメーションがまだ続いている場合、動きを止めてその場に留まる
			
			//通常攻撃のクールタイムを設定
			stateDuration = NORMAL_ATTACK_COOLTIME;
			
			//移動速度を0に設定
			charaParam.moveSpeed = 0;

			velocity.x = 0.0f;
			velocity.z = 0.0f;
			charaParam.acceleration = 0.0f;  //加速度を0に設定
		}
	}
	else
	{
		//目標地点への方向ベクトルを計算
		dirTargetVec = Math::CalcVectorAtoBNormalize(position, targetPointPos);  //目標地点への方向ベクトルを計算
	}

	//移動処理
	Move(dirTargetVec.x, dirTargetVec.z, charaParam.moveSpeed); 
	Turn(elapsedTime, dirTargetVec, charaParam.turnSpeed, orientation);  

	//速度更新
	UpdateVelocity(elapsedTime, position);
}

void Boss::UpdateAttackShotStraightState(float elapsedTime)
{
	//飛びのいた後連射設定する
	//アニメーションが終了し、バックジャンプが行われていなければ連射設定を開始
	if (model->GetIsEndAnimation() && !isBackJump)
	{
		stateDuration = RAPIDFIRE_TIME;   //連射の時間を設定
		stateTimer = 0.0f;                //タイマーリセット
		isBackJump = true;                //バックジャンプ開始

		//その場停止
		charaParam.moveSpeed = 0;
		velocity.x = 0.0f;
		velocity.z = 0.0f;
		charaParam.acceleration = 0.0f;
	}

	//まだバックジャンプが完了していない場合は後退移動する
	if (!isBackJump)
	{
		//プレイヤーからの逆方向に移動するための方向ベクトル計算
		DirectX::XMFLOAT3 dir_target_vec = Math::CalcVectorAtoBNormalize(targetPos, position); 
		
		//後退移動
		Move(dir_target_vec.x, dir_target_vec.z, BACK_JUMP_SPEED);
		//向きの調整
		Turn(elapsedTime, dir_target_vec, charaParam.turnSpeed, orientation);  
	}

	//バックジャンプ後、一定時間経過したら弾を発射
	if (stateTimer > stateDuration && isBackJump)
	{
		ShotBullet(ATTACK_TYPE::SHOT_S);	//弾を発射
		stateTimer = 0;						//タイマーリセット
		rapidCount++;						//連射回数カウント
	}

	//連射回数が上限を超えたら、待機状態に遷移
	if (rapidCount > RAPID_MAX && isBackJump)
	{
		TransitionIdleState();
		isBackJump = false;     //バックジャンプフラグOFF
		
		//通常の攻撃クールタイム
		stateDuration = NORMAL_ATTACK_COOLTIME;
		
		//パラメータリセット
		charaParam.moveSpeed = WALK_SPEED;
		charaParam.maxMoveSpeed = WALK_SPEED;
		charaParam.acceleration = ACCELERATION_NORMAL_SPEED;
	}

	//速度更新
	UpdateVelocity(elapsedTime, position);
}

void Boss::UpdateDamageState(float elapsedTime)
{
	//ダメージ状態が終了したら待機状態に戻る
	if (stateTimer > stateDuration)
	{
		TransitionIdleState();  //待機状態に遷移
		stateDuration = DAMAGE_STUN_DURATION;  //ダメージスタン状態の期間を設定
	}
}

void Boss::UpdateDeadState(float elapsedTime)
{
	//死亡アニメーションが終了したら死亡フラグを立てる
	if (model->GetIsEndAnimation())
	{
		isDead = true;  //死亡フラグを立てる
	}
}

void Boss::AttackRoutine(float elapsedTime)
{
	//プレイヤーまでの距離を計算
	float length_to_target = Math::CalcVectorAtoBLength(position, targetPos);

	//プレイヤーが近ければ近距離攻撃を選択
	if (length_to_target < ATTACK_ACTION_LENGTH)
	{
		SelectAttackTypeShort();  //近距離攻撃を選択
		return;
	}

	//プレイヤーが一定時間自分に近づかなかったら遠距離攻撃を選択
	attackResponderTimer += elapsedTime;
	if (attackResponderTimer > ATTACK_RESPONDER_TIME)
	{
		SelectAttackTypeLong();  //遠距離攻撃を選択
		attackResponderTimer = 0;  //タイマーリセット
	}
}

void Boss::SelectAttackTypeShort()
{
	//ランダムな数を生成して、攻撃タイプを決定
	int random = std::abs(static_cast<int>(Noise::Instance().get_rnd())) % static_cast<int>(ATTACK_TYPE::MAX_COUNT) - 1;
	ATTACK_TYPE attack_type = static_cast<ATTACK_TYPE>(random);  //ランダムに攻撃方法を選択

	//選択した攻撃方法に基づき、遷移する状態を決定
	switch (attack_type)
	{
	case ATTACK_TYPE::TACKLE:
		//タックル攻撃へ遷移
		TransitionAttackTackleState();
		break;
	case ATTACK_TYPE::JUMP:
		//ジャンプ攻撃へ遷移
		TransitionAttackJumpState();
		break;
	}
}

void Boss::SelectAttackTypeLong()
{
	//ランダムな数を生成して、攻撃タイプを決定
	int random = std::abs(static_cast<int>(Noise::Instance().get_rnd())) % static_cast<int>(ATTACK_TYPE::MAX_COUNT);
	ATTACK_TYPE attack_type = static_cast<ATTACK_TYPE>(random);  //ランダムに攻撃方法を選択

	//遠距離攻撃のみを選択
	switch (attack_type)
	{
	case ATTACK_TYPE::TACKLE:
		//タックル攻撃へ遷移
		TransitionAttackTackleState();
		break;
	case ATTACK_TYPE::JUMP:
		//ジャンプ攻撃へ遷移
		TransitionAttackJumpState();
		break;
	case ATTACK_TYPE::SHOT_S:
		//直線射撃攻撃へ遷移
		TransitionAttackShotStraightState();
		break;
	}
}
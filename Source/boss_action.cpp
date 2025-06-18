#include "boss.h"
#include "noise.h"

/*--------------------状態遷移------------------------*/
void Boss::TransitionIdleState()
{
	//待機状態へ遷移
	act_update = &Boss::UpdateIdleState;

	//状態とアニメーションをIDLEに設定
	state = STATE::IDLE;
	bossAnimation = BossAnimation::BOSS_IDLE;

	//状態タイマーと持続時間を初期化
	stateTimer = 0;
	stateDuration = 2.0f;
}

void Boss::TransitionWalkState()
{
	//歩行状態へ遷移
	act_update = &Boss::UpdateWalkState;

	//状態とアニメーションをWALKに設定
	state = STATE::WALK;
	bossAnimation = BossAnimation::BOSS_WALK;

	//移動速度を歩行速度に設定
	charaParam.moveSpeed = WALK_SPEED;

	//状態タイマーと持続時間を初期化
	stateTimer = 0;
	stateDuration = 3.0f;
}

void Boss::TransitionAttack_Tackle_State()
{
	//タックル攻撃状態へ遷移
	act_update = &Boss::UpdateAttack_Tackle_State;

	//状態とアニメーションをTACKLEに設定
	state = STATE::TACKLE;
	bossAnimation = BossAnimation::BOSS_RUN;

	//攻撃パラメータをタックル用に設定
	attackParam = param.tackleParam;

	//状態タイマーと持続時間を初期化
	stateTimer = 0;
	stateDuration = 1.0f;
}

void Boss::TransitionAttack_Jump_State()
{
	//ジャンプ攻撃状態へ遷移
	act_update = &Boss::UpdateAttack_Jump_State;

	//状態とアニメーションをJUMPに設定
	state = STATE::JUMP;
	bossAnimation = BossAnimation::BOSS_CHARGE;

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

void Boss::TransitionAttack_ShotStraight_State()
{
	//直線射撃状態へ遷移
	act_update = &Boss::UpdateAttack_ShotStraight_State;

	//状態をSHOT_Sに、アニメーションをJUMPに設定
	state = STATE::SHOT_S;
	bossAnimation = BossAnimation::BOSS_JUMP;

	//加速度をジャンプ用に設定
	charaParam.acceleration = ACCELERATION_JUMP_SPEED;

	//連射カウントを初期化
	rapidCount = 0;

	//状態タイマーと持続時間を初期化
	stateTimer = 0;
	stateDuration = RAPIDFIRE_TIME;
}

void Boss::TransitionAttack_ShotHoming_State()
{
}

void Boss::TransitionDamageState()
{
	//ダメージ状態へ遷移
	act_update = &Boss::UpdateDamageState;

	//状態とアニメーションをDAMAGEに設定
	state = STATE::DAMAGE;
	bossAnimation = BossAnimation::BOSS_HIT;

	//状態タイマーと持続時間を初期化
	stateTimer = 0;
	stateDuration = DAMAGE_STUN_DURATION;
}

void Boss::TransitionDeadState()
{
	//死亡状態へ遷移
	act_update = &Boss::UpdateDeadState;

	//状態とアニメーションをDEADに設定
	state = STATE::DEAD;
	bossAnimation = BossAnimation::BOSS_DEAD;

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
	DirectX::XMFLOAT3 dir_target_vec = Math::calc_vector_AtoB_normalize(position, target_pos);  //プレイヤー位置への方向ベクトルを計算
	Move(dir_target_vec.x, dir_target_vec.z, charaParam.moveSpeed);  //プレイヤー方向に移動
	Turn(elapsedTime, dir_target_vec, charaParam.turnSpeed, orientation);  //プレイヤー方向に向けて回転

	//状態タイマーが持続時間を超えたら、攻撃のルーチンへ遷移
	if (stateTimer > stateDuration)
	{
		AttackRoutine(elapsedTime);
	}

	//速度更新
	UpdateVelocity(elapsedTime, position);
}

void Boss::UpdateAttack_Tackle_State(float elapsedTime)
{
	//タックル攻撃の状態が終了するまで目標地点を設定
	if (stateTimer < stateDuration)
	{
		targetPoint_pos.x = target_pos.x;
		targetPoint_pos.z = target_pos.z;
		return;
	}

	//目標地点までのXZ平面での距離判定
	float vx = targetPoint_pos.x - position.x;
	float vz = targetPoint_pos.z - position.z;
	float distSq = vx * vx + vz * vz;

	attackParam.isAttack = true;  //攻撃中に設定
	DirectX::XMFLOAT3 pos = { position.x ,0.0f, position.z };
	DirectX::XMFLOAT3 pointPos = { targetPoint_pos.x ,0.0f, targetPoint_pos.z };
	DirectX::XMFLOAT3 dir_target_vec = Math::calc_vector_AtoB_normalize(pos, pointPos);  //目標地点への方向ベクトルを計算
	Move(dir_target_vec.x, dir_target_vec.z, param.runSpeed);  //目標地点に向かって移動
	Turn(elapsedTime, dir_target_vec, charaParam.turnSpeed, orientation);  //目標地点に向けて回転

	//半径2.0fの範囲内に目標地点が入ったら、待機状態に遷移
	const float radius = 2.0f;
	if (distSq < radius * radius)
	{
		TransitionIdleState();
		stateDuration = NORMAL_ATTACK_COOLTIME;  //通常攻撃のクールタイムを設定
		attackParam.isAttack = false;  //攻撃終了
	}

	//速度更新
	UpdateVelocity(elapsedTime, position);
}

void Boss::UpdateAttack_Jump_State(float elapsedTime)
{
	//ジャンプアニメーションが開始されるまで、早すぎる場合は何もしない
	if (bossAnimation == BossAnimation::BOSS_JUMP && time < 0.1f)
		return;

	//状態タイマーが持続時間を超えたら、ジャンプ攻撃を行う
	if (stateTimer < stateDuration)
	{
		//目標地点までの移動速度を計算
		charaParam.moveSpeed = CalcMoveSpeed(target_pos, 0.5f);
		targetPoint_pos.x = target_pos.x;
		targetPoint_pos.y = target_pos.y;
		targetPoint_pos.z = target_pos.z;
		return;
	}
	else
	{
		//ジャンプアニメーションに変更
		bossAnimation = BossAnimation::BOSS_JUMP; 
	}

	//目標地点までの高さの差
	float length = targetPoint_pos.y - position.y;
	if (!isJump && length > 5.0f && time > 0.1f)
	{
		isJump = true;
		velocity.y = 30.0f;  //ジャンプの初期速度を設定
	}

	//目標地点までのXZ平面での距離判定
	float vx = targetPoint_pos.x - position.x;
	float vz = targetPoint_pos.z - position.z;
	float distSq = vx * vx + vz * vz;

	DirectX::XMFLOAT3 dir_target_vec{};
	attackParam.isAttack = true;  //攻撃中に設定

	const float radius = 3.0f;
	if (distSq < radius * radius)
	{
		//ジャンプ攻撃アニメーションが終了したら、待機状態に遷移
		if (model->GetIsEndAnimation())
		{
			TransitionIdleState();
			attackParam.isAttack = false;  //攻撃終了
			charaParam.moveSpeed = WALK_SPEED;  //歩行速度に戻す
			charaParam.acceleration = ACCELERATION_NORMAL_SPEED;  //加速度を通常の速度に戻す
			isJump = false;  //ジャンプフラグをリセット
		}
		else
		{
			stateDuration = NORMAL_ATTACK_COOLTIME;  //通常攻撃のクールタイムを設定
			charaParam.moveSpeed = 0;  //移動速度を0に設定
			velocity.x = 0.0f;
			velocity.z = 0.0f;
			charaParam.acceleration = 0.0f;  //加速度を0に設定
		}
	}
	else
	{
		dir_target_vec = Math::calc_vector_AtoB_normalize(position, targetPoint_pos);  //目標地点への方向ベクトルを計算
	}

	//移動処理
	Move(dir_target_vec.x, dir_target_vec.z, charaParam.moveSpeed); 
	Turn(elapsedTime, dir_target_vec, charaParam.turnSpeed, orientation);  

	//速度更新
	UpdateVelocity(elapsedTime, position);
}

void Boss::UpdateAttack_ShotStraight_State(float elapsedTime)
{
	//飛びのいた後連射設定する
	//アニメーションが終了し、バックジャンプが行われていなければ連射設定を開始
	if (model->GetIsEndAnimation() && !isBackJump)
	{
		stateDuration = RAPIDFIRE_TIME;   //連射の時間を設定
		stateTimer = 0.0f;                //タイマーリセット
		isBackJump = true;                //バックジャンプ開始

		//バックジャンプ中は移動を停止し、加速度もゼロに設定
		charaParam.moveSpeed = 0;
		velocity.x = 0.0f;
		velocity.z = 0.0f;
		charaParam.acceleration = 0.0f;
	}

	//バックジャンプが終了していない場合、後退移動を行う
	if (!isBackJump)
	{
		const float backJumpSpeed = 20.0f;  //バックジャンプの速度
		//プレイヤー位置とのベクトル計算
		DirectX::XMFLOAT3 dir_target_vec = Math::calc_vector_AtoB_normalize(target_pos, position); 
		
		//後退移動
		Move(dir_target_vec.x, dir_target_vec.z, backJumpSpeed);  
		//向きの調整
		Turn(elapsedTime, dir_target_vec, charaParam.turnSpeed, orientation);  
	}

	//連射タイミングになったら弾を撃つ
	if (stateTimer > stateDuration && isBackJump)
	{
		ShotBullet(ATTACK_TYPE::SHOT_S);  //弾を発射
		stateTimer = 0;  //タイマーリセット
		rapidCount++;    //連射回数をカウント
	}

	//連射回数が上限を超えたら、待機状態に遷移
	if (rapidCount > RAPID_MAX && isBackJump)
	{
		TransitionIdleState();  //待機状態に遷移
		isBackJump = false;     //バックジャンプフラグをリセット
		stateDuration = NORMAL_ATTACK_COOLTIME;  //通常の攻撃クールタイム
		charaParam.moveSpeed = WALK_SPEED;  //歩行速度に設定
		charaParam.maxMoveSpeed = WALK_SPEED;  //最大速度も歩行速度に設定
		charaParam.acceleration = ACCELERATION_NORMAL_SPEED;  //通常の加速度
	}

	//速度更新
	UpdateVelocity(elapsedTime, position);  //ボスの速度を更新
}

void Boss::UpdateAttack_ShotHoming_State(float elapsedTime)
{
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
	float length_to_target = Math::calc_vector_AtoB_length(position, target_pos);

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
		TransitionAttack_Tackle_State();
		break;
	case ATTACK_TYPE::JUMP:
		//ジャンプ攻撃へ遷移
		TransitionAttack_Jump_State();
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
		TransitionAttack_Tackle_State();
		break;
	case ATTACK_TYPE::JUMP:
		//ジャンプ攻撃へ遷移
		TransitionAttack_Jump_State();
		break;
	case ATTACK_TYPE::SHOT_S:
		//直線射撃攻撃へ遷移
		TransitionAttack_ShotStraight_State();
		break;
	}
}
#include "character.h"
#include "stage_manager.h"
#include "user.h"

void Character::AddImpulse(const DirectX::XMFLOAT3& impulse)
{
	//速力に力を加える
	velocity.x += impulse.x;
	velocity.y += impulse.y;
	velocity.z += impulse.z;
}

bool Character::ApplyDamage(int damage, float invincibleTime, WINCE_TYPE type)
{
	//ダメージが0の場合は処理不要
	if (damage == 0)return false;

	//既に死亡している場合は処理不要
	if (health <= 0)return false;

	//無敵時間中であれば処理を行わない
	if (invincibleTimer > 0.0f)return false;

	//無敵時間設定
	invincibleTimer = invincibleTime;
	//ダメージ処理
	health -= damage;

	//HPが0以下になった場合は死亡処理
	if (health <= 0)
	{
		OnDead();
	}
	else//生存中の場合のダメージ処理
	{
		OnDamaged(type);
	}

	//健康状態が変更した場合はtrueを返す
	return true;
}

void Character::Move(float vx, float vz, float speed)
{
	//移動方向ベクトルを設定
	moveVec_x = vx;
	moveVec_z = vz;

	//最大速度設定
	charaParam.maxMoveSpeed = speed;
}

void Character::Turn(float elapsedTime, float vx, float vz, float speed)
{
	//実際の旋回速度 = 指定された速度 × 経過時間
	speed *= elapsedTime;

	//入力ベクトルの長さを計算
	float length = sqrtf(vx * vx + vz * vz);
	//ほぼ無方向のベクトルなら処理しない
	if (length < 0.001f) return;

	//正規化
	vx /= length;
	vz /= length;

	//現在の前方ベクトル（Y軸回転による前方向）
	const float forwardX = sinf(angle.y);
	const float forwardZ = cosf(angle.y);

	//外積を用いて回転方向を判定
	float cross = forwardX * vz - forwardZ * vx;

	//内積で向きの類似度を取得
	float dot = forwardX * vx + forwardZ * vz;
	//回転量
	float rot = 1 - dot;

	//回転量を最大速度に制限
	if (rot > speed) rot = speed;


	if (cross < 0.0f)//外積が負なら右回り
	{
		angle.y += rot;
	}

	else //正なら左回り
	{
		angle.y -= rot;
	}

	//各軸角度を正規化
	angle.x = fmod(angle.x, DirectX::XMConvertToRadians(360.0f));
	angle.y = fmod(angle.y, DirectX::XMConvertToRadians(360.0f));
	angle.z = fmod(angle.z, DirectX::XMConvertToRadians(360.0f));
}

void Character::Turn(float elapsedTime, DirectX::XMFLOAT3 move_vec, float speed, DirectX::XMFLOAT4& orien)
{
	//現在の向きをXMVECTORクラスへ変換
	DirectX::XMVECTOR orientationVec = DirectX::XMLoadFloat4(&orien);
	
	//XZ平面の移動ベクトルを抽出
	DirectX::XMFLOAT3 moveV = { move_vec.x, 0, move_vec.z };
	DirectX::XMVECTOR MoveVec = DirectX::XMLoadFloat3(&moveV);
	
	//移動ベクトルがゼロなら回転不要
	if (DirectX::XMVector3Equal(MoveVec, DirectX::XMVectorZero())) return; //もしmove_vecがゼロベクトルならリターン
	

	//現在の姿勢から上方向・前方向ベクトルを取得
	DirectX::XMVECTOR up = Math::get_posture_up_vec(orien);
	DirectX::XMVECTOR forward = Math::get_posture_forward_vec(orien);

	//正規化
	up = DirectX::XMVector3Normalize(up);
	forward = DirectX::XMVector3Normalize(forward);
	MoveVec = DirectX::XMVector3Normalize(MoveVec);

	//回転軸を上方向に固定
	DirectX::XMVECTOR axis = up;

	//目標方向と現在の向きの角度を求める
	DirectX::XMVECTOR Ang = DirectX::XMVector3Dot(forward, MoveVec);
	DirectX::XMStoreFloat(&turnAngle, Ang);
	
	//内積から角度へ変換
	turnAngle = acosf(turnAngle);

	//角度と方向ベクトルをfloat3に変換して回転方向を判定
	DirectX::XMFLOAT3 forw{};
	DirectX::XMFLOAT3 m_vec{};
	DirectX::XMStoreFloat3(&forw, forward);
	DirectX::XMStoreFloat3(&m_vec, MoveVec);

	//回転角が微小な場合は、回転を行わない
	if (fabs(turnAngle) > 1e-8f)
	{
		//外積のZ成分を使って左右の回転方向を判定
		float cross{ forw.x * m_vec.z - forw.z * m_vec.x };

		const float rate = speed;
		if (cross < 0.0f)
		{
			//右回転
			DirectX::XMVECTOR q;
			q = DirectX::XMQuaternionRotationAxis(axis, turnAngle);//正の方向に動くクオータニオン
			DirectX::XMVECTOR End = DirectX::XMQuaternionMultiply(orientationVec, q);
			orientationVec = DirectX::XMQuaternionSlerp(orientationVec, End, rate * elapsedTime);
		}
		else
		{
			//左回転
			DirectX::XMVECTOR q;
			q = DirectX::XMQuaternionRotationAxis(axis, -turnAngle);//負の方向に動くクオータニオン
			DirectX::XMVECTOR End = DirectX::XMQuaternionMultiply(orientationVec, q);
			orientationVec = DirectX::XMQuaternionSlerp(orientationVec, End, rate * elapsedTime);
		}
	}

	// orientationVecからorientationを更新
	DirectX::XMStoreFloat4(&orientation, orientationVec);
}

void Character::Jump(float speed)
{
	//上方向の力を設定
	velocity.y = speed;
}

void Character::UpdateVelocity(float elapsedTime, DirectX::XMFLOAT3& position)
{
	//経過フレーム
	float elapsed_frame = 60.0f * elapsedTime;


	//垂直速力更新処理
	UpdateVerticalVelocity(elapsed_frame);

	//水平速力更新処理
	UpdateHorizontalVelocity(elapsed_frame);

	//垂直移動更新処理
	UpdateVerticalMove(elapsedTime, position);

	//水平移動更新処理
	UpdateHorizontalMove(elapsedTime, position);
}

void Character::UpdateInvicibleTimer(float elapsedTime)
{
	// 無敵時間が残っている場合、経過時間を減算
	if (invincibleTimer > 0.0f)
	{
		invincibleTimer -= elapsedTime;
	}
	else
	{
		// タイマーが0を下回らないようにクリップ
		invincibleTimer = 0.0f;
	}
}

void Character::UpdateVerticalVelocity(float elapsed_frame)
{
	// 重力加速度を時間で積分して、垂直方向の速度に加算
	velocity.y += gravity * elapsed_frame;
}

void Character::UpdateVerticalMove(float elapsedTime, DirectX::XMFLOAT3& position)
{
	//キャラクターの下方向の移動量
	float my = velocity.y * elapsedTime;

	//傾斜率の初期化
	slopeRate = 0.0f;

	//キャラクターのY軸方向となる法線ベクトル
	DirectX::XMFLOAT3 normal = { 0,1,0 };

	//地面法線初期化
	slopeNormal = { 0, 1, 0 };
	//落下中
	if (my < 0.0f)
	{
		//レイの開始位置は足元より少し上
		DirectX::XMFLOAT3 start = { position.x, position.y + stepOffset, position.z };
		//レイの終点位置は移動後の位置
		DirectX::XMFLOAT3 end = { position.x, position.y + my  , position.z };

		//レイキャストによる地面判定
		HitResult hit;
		if (StageManager::Instance().RayCast(start, end, hit))
		{
			//地面に設置している
			position = hit.position;
			// 法線ベクトル取得
			normal = hit.normal;

			//XZ平面の長さ
			float normalLengthXZ =
				sqrtf(hit.normal.x * hit.normal.x +
					hit.normal.z * hit.normal.z);
			
			//傾斜率の計算
			slopeRate = 1.0f - (hit.normal.y / (normalLengthXZ + hit.normal.y));

			//着地時の処理
			if (!isGround)
			{
				OnLanding();
			}

			//落下停止
			velocity.y = 0.0f;
			isGround = true;
		}
		else
		{
			//空中落下
			position.y += my;
			isGround = false;
		}
	}
	//上昇中
	else if (my > 0.0f)
	{
		position.y += my;
		isGround = false;
	}

	//地面の向きに沿うように姿勢補正
	{
		DirectX::XMVECTOR OrientationVec = DirectX::XMLoadFloat4(&orientation);

		//上ベクトル
		DirectX::XMVECTOR up = Math::get_posture_up_vec(orientation);
		//法線のベクトル
		DirectX::XMVECTOR Normal = DirectX::XMLoadFloat3(&slopeNormal);
		Normal = DirectX::XMVector3Normalize(Normal);
		
		//軸ベクトル算出
		DirectX::XMVECTOR axis;	//回転軸
		float angle;			//回転角

		axis = DirectX::XMVector3Cross(up, Normal);
		DirectX::XMVECTOR Ang = DirectX::XMVector3Dot(up, Normal);
		DirectX::XMStoreFloat(&angle, Ang);
		
		angle = acosf(angle);
		//軸がゼロベクトルなら処理しない
		if (DirectX::XMVector3Equal(axis, DirectX::XMVectorZero())) return;
		
		//回転角（angle）が微小な場合は、回転を行わない
		if (fabs(angle) > 1e-8f)
		{
			DirectX::XMVECTOR q;
			q = DirectX::XMQuaternionRotationAxis(axis, angle);
			DirectX::XMVECTOR End = DirectX::XMQuaternionMultiply(OrientationVec, q);
			float rate = 10.0f;
			OrientationVec = DirectX::XMQuaternionSlerp(OrientationVec, End, rate * elapsedTime);
		}
		
		// orientationVecからorientationを更新
		DirectX::XMStoreFloat4(&orientation, OrientationVec);
	}
}

void Character::UpdateHorizontalVelocity(float elapsed_frame)
{
	//XZ平面の速度の大きさを計算
	float length = sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);
	
	//速度がある場合は摩擦によって減速させる
	if (length > 0.0f)
	{
		//摩擦力をフレーム時間で調整
		float friction = charaParam.friction * elapsed_frame;

		//摩擦による速度の減少処理
		if (length > friction)
		{
			//速度ベクトルを単位ベクトル化
			float vx = velocity.x / length;
			float vz = velocity.z / length;

			//摩擦力分だけ速度を減らす
			velocity.x -= vx * friction;
			velocity.z -= vz * friction;
		}
		else
		{
			//摩擦が速度以上の場合は速度を0にする
			velocity.x = 0.0f;
			velocity.z = 0.0f;
		}
	}
	//速度が最大以下の場合は加速処理
	if (length <= charaParam.maxMoveSpeed)
	{
		//移動ベクトルがゼロベクトルでなければ加速する
		float moveVecLength = sqrtf(moveVec_x * moveVec_x + moveVec_z * moveVec_z);

		if (moveVecLength > 0.0f)
		{
			//加速度を計算
			float acceleration = charaParam.acceleration * elapsed_frame;
			
			//空中にいるときは加速力を減らす
			if (GetIsGround()) 
				acceleration += charaParam.airControl;

			//移動ベクトルによる加速処理
			velocity.x += moveVec_x * acceleration;
			velocity.z += moveVec_z * acceleration;

			//最大速度制限
			float length = sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);
			if (length > charaParam.maxMoveSpeed)
			{
				float vx = velocity.x / length;
				float vz = velocity.z / length;
				velocity.x = vx * charaParam.maxMoveSpeed;
				velocity.z = vz * charaParam.maxMoveSpeed;
			}

			//下り坂で速度が不安定にならないよう調整
			if (isGround && slopeRate > 0.0f)
			{
				velocity.y -= length * slopeRate * elapsed_frame;
			}
		}
	}
	else
	{
		//速度が最大を超えている場合は移動入力をクリア
		moveVec_x = 0.0f;
		moveVec_z = 0.0f;
	}
}

void Character::UpdateHorizontalMove(float elapsedTime, DirectX::XMFLOAT3& position)
{
	//水平速力計算
	float velocity_length_xz = sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);
	//もし速力があれば
	if (velocity_length_xz > 0.0f)
	{

		//水平移動量を計算
		float mx = velocity.x * elapsedTime;
		float mz = velocity.z * elapsedTime;

		//レイの開始位置と終点位置
		DirectX::XMFLOAT3 start = { position.x - mx / 50.0f, position.y + stepOffset * 2, position.z - mz / 50.0f };
		DirectX::XMFLOAT3 end = { position.x + mx * vsWallRayPower, start.y, position.z + mz * vsWallRayPower };
		HitResult hit;

		//壁との衝突判定
		if (StageManager::Instance().RayCast(start, end, hit))//何か壁があれば
		{
			//レイのベクトルを計算
			DirectX::XMVECTOR Start = DirectX::XMLoadFloat3(&start);
			DirectX::XMVECTOR End = DirectX::XMLoadFloat3(&end);
			DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(End, Start);

			//壁の法線ベクトルを読み込み
			DirectX::XMVECTOR Normal = DirectX::XMLoadFloat3(&hit.normal);

			//入射ベクトルを法線に射影
			DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(DirectX::XMVectorNegate(Vec), Normal); //この時点では正負が逆なことに注意

			//補正位置の計算
			DirectX::XMVECTOR Correct = DirectX::XMVectorMultiplyAdd(Normal, Dot, End);//Endに射影ベクトルを足す

			DirectX::XMFLOAT3 correct;
			DirectX::XMStoreFloat3(&correct, Correct);

			HitResult hit2;
			//補正位置から再度レイキャストして壁がないか確認
			if (!StageManager::Instance().RayCast(start, correct, hit2))//何か壁があれば
			{
				//壁にめり込まない位置に座標をセット
				position.x = correct.x;
				position.z = correct.z;

				//壁に当たったので速度を0にする
				velocity.x = 0;
				velocity.z = 0;
			}
			else
			{
				//補正位置でも壁があればヒットした位置に座標をセット
				position.x = hit2.position.x;
				position.z = hit2.position.z;

				//速度を0にする
				velocity.x = 0;
				velocity.z = 0;
			}
		}
		else
		{
			//壁に当たらなければ通常移動
			position.x += mx;
			position.z += mz;
		}
	}
}

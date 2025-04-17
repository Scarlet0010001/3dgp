#include "player.h"
#include "bullet_straight.h"
#include "bullet_manager.h"
#include "shader.h"
#include"user.h"
#include "texture.h"
#include "operators.h"
#include "collision.h"
#include "Graphics.h"
#include "magic_enum/include/magic_enum.hpp"

#include "effect_manager.h"

#include <filesystem>
#include <fstream>
#include <cereal/archives/json.hpp>

Player::Player()
{
	//インスタンス取得
	Graphics& graphics = Graphics::Instance();
	//キャラクターモデルを読み込む
	model = std::make_unique<gltf_model>(graphics.GetDevice().Get(),
		"Resources/Character/Player/glb/white_crow.glb", true);
	
	//エフェクト作成（斬撃エフェクト）
	slashEffect = std::make_unique<Effect>("Resources/Effect/Slash/slash.efkefc");

	// 効果音の読み込み
	audios[PLAYER_SE::SE_SABER] = audio::_emplace(L"Resources/Sound/SE/saber.wav");
	audios[PLAYER_SE::SE_LASER] = audio::_emplace(L"Resources/Sound/SE/laser.wav");
	audios[PLAYER_SE::SE_BOOST] = audio::_emplace(L"Resources/Sound/SE/boost.wav");
	audios[PLAYER_SE::SE_DAMAGE] = audio::_emplace(L"Resources/Sound/SE/Damage.wav");

	// モデルのトランスフォームデータを累積
	model->cumulate_transforms(model->nodes, transform);

	// アニメーションノードを初期化
	for (auto& node : animated_nodes)
	{
		node = model->nodes;
	}
	blended_animated_nodes = model->nodes;

	// UIを初期化
	ui = std::make_unique<PlayerUI>();

	// 武器ノードの取得
	beamSaber[LR::LEFT] = model->find_nodes("Left_wep1");
	beamSaber[LR::RIGHT] = model->find_nodes("Right_wep1");
	lowerArm[LR::LEFT] = model->find_nodes("lowerarm_l");
	lowerArm[LR::RIGHT] = model->find_nodes("lowerarm_r");

	// 入力デバイスの取得
	mouse = &Device::Instance().GetMouse();
	gamePad = &Device::Instance().GetGamePad();
	camera = &Camera::Instance();

	// 初期化処理を実行
	Initialize();

}

void Player::Initialize()
{
	//パラメーターロード
	LoadDataFile();

	//パラメーター初期化
	position = { 0.0f, 37.0f, 0.0f };
	velocity = { 0.0f, 0.0f, 0.0f };
	scale.x = scale.y = scale.z = 2.0f;
	//Charactorクラスのパラメーター初期化
	charaParam = param.charaInitParam;

	//当たり判定半径設定
	collider.radius = 1.0f;

	//体力を設定
	charaParam.maxHealth = 150.0f;
	health = charaParam.maxHealth;

	// 移動速度とジャンプ回数を設定
	stepOffset = 2.0f;
	jumpCount = jumpLimit;
	charaParam.moveSpeed = 15.0f;

	// 移動速度とジャンプ回数を設定
	TransitionIdleState();

	// 被ダメージ時の処理を設定
	damagedFunction = [=](int damage, float invincible, WINCE_TYPE type)->bool {return ApplyDamage(damage, invincible, type); };

	// 攻撃時のカメラ揺れパラメータ設定
	attackParam.cameraShake.max_X_shake = 3.0f;
	attackParam.cameraShake.max_Y_shake = 7.0f;
	attackParam.cameraShake.time = 0.5f;

	// ヒットストップの設定
	attackParam.hitStop.time = 0.1f;
}

Player::~Player()
{
}

void Player::Update(float elapsedTime)
{
	//インスタンス取得
	Graphics& graphics = Graphics::Instance();

	//-----------------ブースト更新-----------------//
	BoostUpdate(elapsedTime);

	//-----------------ステート更新処理-----------------//
	(this->*p_update)(elapsedTime);

	//Yボタンを押すとロックオンする
	if (gamePad->GetButtonDown() & GamePad::BTN_Y)//V
	{
		camera->SetLockOn();
	}
	//飛行と射撃状態の時はカメラの姿勢を使う
	if (state == STATE::WING || state == STATE::SHOT)
	{
		orientation = camera->GetOrientation();
	}

	//プレイヤーの正面情報を更新
	forward = Math::get_posture_forward(orientation);
	
	//-----------------無敵時間の更新-----------------//
	UpdateInvicibleTimer(elapsedTime);

	//-----------------デバッグプリミティブ更新-----------------//
	DebugPrimitiveUpdate();

	//-----------------当たり判定カプセル更新-----------------//
	collider.start = position;
	collider.end = { position.x,position.y + charaParam.height, position.z };

	//攻撃モーションじゃない場合攻撃当たり判定オフ
	if (state != STATE::LEFT_ATTACK && state != STATE::RIGHT_ATTACK)
	{
		attackParam.isAttack = false;
	}

	//画面外に行った場合初期位置に戻す
	if (position.y < -10.0f)
	{
		position = { 0.0f,50.0f,0.0f };
	}

	//-----------------シェーダー更新-----------------//
	ShaderUpdate(elapsedTime);

	//-----------------UI更新-----------------//
	ui->SetHPPercent(GetHpPercent());
	ui->SetBoostPercent(GetBoostPercent());
	if (camera->GetLockOn())
	{
		ui->SetLockonPosition(bossPosition);
		ui->SetLockonDistance(Math::calc_vector_AtoB_length(position, bossPosition));
	}
	ui->Update(elapsedTime);
}

void Player::Render_d(float elapsedTime)
{
}

void Player::Render_f(float elapsedTime)
{
	// グラフィックスインスタンスを取得
	Graphics& graphics = Graphics::Instance();

	// 自機モデルのトランスフォームを更新（ワールド行列を計算）
	transform = Math::calc_world_matrix(scale, orientation, position, Math::COORDINATE_SYSTEM::RHS_YUP);

	// アニメーションの遷移チェック
	if (playerAnimation_transition != playerAnimation)
	{
		if (transition_state != TRANSITION_STATE::NONE)
		{
			// 現在の遷移アニメーションを保存
			playerAnimation_old = playerAnimation_transition;
			animated_nodes[ANIME_NODE::OLD_ANIMATION] = blended_animated_nodes;
			transitionToTransition = true;
		}
		// 新しいアニメーションへの遷移開始
		playerAnimation_transition = playerAnimation;
		transition_state = TRANSITION_STATE::START;
	}

	// 現在のアニメーションがループするか判定
	bool isLoop = FindLoopAnimation(playerAnimation);

	// アニメーションブレンド処理
	if (transition_state > 0 && transition_time > 0.0f)
	{
		switch (transition_state)
		{
		case TRANSITION_STATE::NONE:
			break;

		case TRANSITION_STATE::START:
			if (!transitionToTransition)
			{
				// 直前のアニメーションを設定
				model->animate(playerAnimation_old, time, animated_nodes[ANIME_NODE::OLD_ANIMATION],
					FindLoopAnimation(playerAnimation_old));
			}
			// 新しいアニメーションを0秒の状態から設定
			model->animate(playerAnimation, 0.0f, animated_nodes[ANIME_NODE::NOW_ANIMATION], isLoop);

			// 遷移ステートを「移行中」に設定
			transition_state = TRANSITION_STATE::TRANSITION;
			time = 0.0f;
			factor = 0.0f;

		case TRANSITION_STATE::TRANSITION:
			// アニメーション遷移のブレンド率を計算
			factor = time / transition_time;

			// 旧アニメーションと新アニメーションをブレンド
			model->blend_animations(animated_nodes[ANIME_NODE::OLD_ANIMATION],
				animated_nodes[ANIME_NODE::NOW_ANIMATION],
				factor, blended_animated_nodes);

			// 経過時間を加算
			time += elapsedTime;

			// 遷移完了判定
			if (factor > 1.0f)
			{
				// 遷移終了処理
				transitionToTransition = false;
				transition_state = TRANSITION_STATE::NONE;
				time = 0;
			}
			break;
		}
		// ブレンド後のアニメーションを描画
		model->render(graphics.Get_DC().Get(), transform, blended_animated_nodes);
	}
	else
	{
		// 通常アニメーションの更新
		time += elapsedTime;

		// アニメーションが終了した場合の処理
		if (model->animations.at(playerAnimation).duration < time)
		{
			if (isLoop)
				time = 0;  // ループする場合は最初に戻す
			else
				time = model->animations.at(playerAnimation).duration; // ループしない場合は最後のフレームで停止
		}

		// アニメーションを適用
		model->animate(playerAnimation, time, animated_nodes[ANIME_NODE::NOW_ANIMATION], isLoop);

		// モデルを描画
		model->render(graphics.Get_DC().Get(), transform, animated_nodes[ANIME_NODE::NOW_ANIMATION]);

		// 前回のアニメーションを更新
		playerAnimation_old = playerAnimation;
	}
}

void Player::Render_s(float elapsedTime)
{
}

void Player::RenderUI(float elapsed_time)
{
	//プレイヤーのUI
	ui->Render();
	
}

void Player::CalcCollision_vs_Enemy(Capsule capsule_collider, float collider_height)
{
	//身体の押し出し判定
	Collision::CylinderVsCylinder(
		capsule_collider.start, capsule_collider.radius, collider_height,
		position, charaParam.radius, charaParam.height, &position);

}

void Player::CalcAttack_vs_Enemy(Capsule capsule_collider, float collider_height, AddDamageFunc damaged_func)
{
	//攻撃フラグがオフなら終わる
	if (!attackParam.isAttack) return;

	//どっちの腕で攻撃するか
	int side = 0;
	if (state == STATE::LEFT_ATTACK) side = LR::LEFT;
	else if (state == STATE::RIGHT_ATTACK) side = LR::RIGHT;

	//攻撃時の当たり判定
	if (Collision::SphereVsCylinder(attackCollision_position[side], 1.5f,
		capsule_collider.start, capsule_collider.radius,collider_height))
	{
		//攻撃対象に与えるダメージ量と無敵時間
		if (damaged_func(attackParam.power, attackParam.invinsibleTime, WINCE_TYPE::SMALL))
		{
			//カメラシェイク
			camera->SetCameraShake(attackParam.cameraShake);

			//ヒットストップ
			camera->SetHitStop(attackParam.hitStop);

			//ヒットエフェクト再生
			slashEffect->Play(attackCollision_position[side], 1.5f);
		}
	}
}

bool Player::FindLoopAnimation(PlayerAnimation PA)
{
	//ループさせたいアニメーションじゃなかったらfalse
	if (PA == PlayerAnimation::PLAYER_ATTACK_01
		|| PA == PlayerAnimation::PLAYER_ATTACK_02
		|| PA == PlayerAnimation::PLAYER_ATTACK_03
		|| PA == PlayerAnimation::PLAYER_DAMAGE
		|| PA == PlayerAnimation::PLAYER_DEAD
		|| PA == PlayerAnimation::PLAYER_JUMP_START
		|| PA == PlayerAnimation::PLAYER_JUMP_END
		|| PA == PlayerAnimation::PLAYER_POWER_L
		|| PA == PlayerAnimation::PLAYER_POWER_R
		|| PA == PlayerAnimation::PLAYER_WING_START
		|| PA == PlayerAnimation::PLAYER_WING_END
		) return false;
	return true;
}

void Player::Move(float vx, float vz, float speed)
{
	//移動方向ベクトルを設定
	moveVec_x = vx;
	moveVec_z = vz;

	//飛行ステートだったら最大速度を飛行用に設定
	if (state == STATE::WING)
	{
		charaParam.maxMoveSpeed = param.wingSpeed;
	}
	else
	{
		//最大速度設定
		charaParam.maxMoveSpeed = speed;
	}
}

void Player::Move(float vx, float vy, float vz, float speed)
{
	//移動方向ベクトルを設定
	moveVec_x = vx;
	moveVec_y = vy;
	moveVec_z = vz;
	
	//飛行ステートだったら最大速度を飛行用に設定
	if (state == STATE::WING)
	{
		charaParam.maxMoveSpeed = param.wingSpeed;
	}
	else
	{
		//最大速度設定
		charaParam.maxMoveSpeed = speed;
	}
}

void Player::BoostUpdate(float elapsedTime)
{
	//ブーストステートじゃないかつ地面に接していたらブーストゲージを回復する
	if (isGround && STATE::BOOST != state)
	{
		param.boostTimer += elapsedTime * 3.0f;
	}

	//ブースト最大設定
	if (param.boostTimer >= BOOST_MAX)
	{
		param.boostTimer = BOOST_MAX;
	}
	
	//ブーストゲージがなくなったら強制解除
	if (param.boostTimer < 0 && isBoost)
	{
		isBoost = false;
		TransitionJumpState();
	}
}

bool Player::InputMove(float elapsedTime)
{
	//進行ベクトル取得
	const DirectX::XMFLOAT3 moveVec = GetMoveVec(camera);

	//移動処理
	Move(moveVec.x, moveVec.z, charaParam.moveSpeed);
	Turn(elapsedTime, camera->GetForward(), charaParam.turnSpeed, orientation);

	return moveVec.x != 0.0f || moveVec.y != 0.0f || moveVec.z != 0.0f;
}

bool Player::InputMove(float elapsedTime, float restrictionMove, float restrictionTurn)
{
	//進行ベクトル取得
	const DirectX::XMFLOAT3 move_vec = GetMoveVec(camera);

	//移動処理
	Move(move_vec.x, move_vec.z, charaParam.moveSpeed / restrictionMove);
	Turn(elapsedTime, camera->GetForward(), charaParam.turnSpeed / restrictionTurn, orientation);

	return move_vec.x != 0.0f || move_vec.y != 0.0f || move_vec.z != 0.0f;
}

bool Player::InputMove(float elapsedTime, float move_speed)
{
	//進行ベクトル取得
	const DirectX::XMFLOAT3 move_vec = GetMoveVec(camera);

	//移動処理
	Move(move_vec.x, move_vec.z, move_speed);
	Turn(elapsedTime, camera->GetForward(), charaParam.turnSpeed, orientation);

	return move_vec.x != 0.0f || move_vec.y != 0.0f || move_vec.z != 0.0f;
}

bool Player::InputMoveWing(float elapsedTime)
{
	//進行ベクトル取得
	const DirectX::XMFLOAT3 move_vec = GetMoveVec(camera);

	//移動処理
	Move(move_vec.x,move_vec.y, move_vec.z, charaParam.moveSpeed);
	Turn(elapsedTime, move_vec, charaParam.turnSpeed, orientation);

	return move_vec.x != 0.0f || move_vec.y != 0.0f || move_vec.z != 0.0f;
}

const DirectX::XMFLOAT3 Player::GetMoveVec(Camera* camera, bool wing) const
{
	//入力情報を取得
	float ax = gamePad->GetAxis_LX();
	float ay = gamePad->GetAxis_LY();

	// コントローラーのスティック入力値が一定以下なら入力を無効化
	if (fabs(ax) < 0.3f)  ax = 0.0f;
	if (fabs(ay) < 0.3f)  ay = 0.0f;

	// カメラの前方向ベクトルを取得（XZ平面に変換）
	float camera_forward_x = camera->GetForward().x;
	float camera_forward_y = wing ? camera->GetForward().y : 0.0f; // "wing" が有効ならY方向も考慮
	float camera_forward_z = camera->GetForward().z;

	// カメラの前方向ベクトルを正規化
	float camera_forward_length = sqrtf(camera_forward_x * camera_forward_x +
		camera_forward_y * camera_forward_y +
		camera_forward_z * camera_forward_z);
	if (camera_forward_length > 0.0f)
	{
		camera_forward_x /= camera_forward_length;
		camera_forward_y /= camera_forward_length;
		camera_forward_z /= camera_forward_length;
	}

	// カメラの右方向ベクトルを取得（XZ平面に変換）
	float camera_right_x = camera->GetRight().x;
	float camera_right_y = wing ? camera->GetRight().y : 0.0f; // "wing" が有効ならY方向も考慮
	float camera_right_z = camera->GetRight().z;

	// カメラの右方向ベクトルを正規化
	float camera_right_length = sqrtf(camera_right_x * camera_right_x +
		camera_right_y * camera_right_y +
		camera_right_z * camera_right_z);
	if (camera_right_length > 0.0f)
	{
		camera_right_x /= camera_right_length;
		camera_right_y /= camera_right_length;
		camera_right_z /= camera_right_length;
	}

	// 移動方向の計算（カメラの向きに基づいて移動ベクトルを作成）
	DirectX::XMFLOAT3 vec{};
	vec.x = (camera_forward_x * ay) + (camera_right_x * ax);
	vec.y = (camera_forward_y * ay) + (camera_right_y * ax);
	vec.z = (camera_forward_z * ay) + (camera_right_z * ax);

	// 移動ベクトルを正規化
	DirectX::XMVECTOR v = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&vec));
	DirectX::XMStoreFloat3(&vec, v);

	return vec;
}

void Player::ShaderUpdate(float elapsedTime)
{
	//ラジアルブラータイマーがオフの場合
	if (player_radialBlur_constant.blurStrength <= 0 
		|| radialTimer <= 0) {
		player_radialBlur_constant.blurStrength = 0.0f;
		player_radialBlur_constant.blurRadius = 0.0f;
	}
	else
	{
		//飛行モード時
		if (state == STATE::WING)
		{
			player_radialBlur_constant.blurStrength -= elapsedTime;
			player_radialBlur_constant.blurRadius -= elapsedTime;
		}
		//ブースト時
		else
		{
			float factor = radialTimer / player_radialBlur_constant.blurTimer;
			radialTimer -= elapsedTime;

			player_radialBlur_constant.blurStrength = factor;
		}
	}
	
	//色収差
	//ダメージを食らったとき
	if (isGlitch_CA && glitch_CATimer > 0)
	{
		float factor = glitch_CATimer / 0.03f;
		glitch_CATimer -= 0.03f * elapsedTime;

		player_glitch_CA_constant.density = factor;
		player_glitch_CA_constant.shift = 0.015f;
		player_glitch_CA_constant.x_shifting = 0.015f;
		player_glitch_CA_constant.y_shifting = 0.015f;
	}
	//平常時
	else
	{
		player_glitch_CA_constant.center = { 0.5f,0.5f };
		player_glitch_CA_constant.brightness = 0.0f;
		player_glitch_CA_constant.density = 0.0f;
		player_glitch_CA_constant.extension = 0.0f;
		player_glitch_CA_constant.glitch_mask_radius = 0.0f;
		player_glitch_CA_constant.glitch_sampling_count = 0.0f;
		player_glitch_CA_constant.rand_float = 0.0f;
		player_glitch_CA_constant.shift = 0.0f;
		player_glitch_CA_constant.uv_slider = 0.0f;
		player_glitch_CA_constant.x_shift = { 0,0 };
		player_glitch_CA_constant.x_shifting = 0.0f;
		player_glitch_CA_constant.y_shift = { 0,0 };
		player_glitch_CA_constant.y_shifting = 0.0f;

	}
}

void Player::InputJump()
{
	//スペースを押したらジャンプ
	if (gamePad->GetButtonDown() & GamePad::BTN_A)
	{
		//ジャンプ回数が最大になったらジャンプしない
		if (jumpCount < jumpLimit)
		{
			{
				TransitionJumpState();
				Jump(param.jumpSpeed);
			}

			//ジャンプしても地面についているというありえない状況を回避するため
			isGround = false;

			++jumpCount;
		}
	}
}

void Player::InputAvoidance()
{
	//ブースト量が25%以下だと出来ない
	if (param.boostTimer < 2.5f)return;

	//右トリガーを押したら回避
	if (gamePad->GetButtonDown() & GamePad::BTN_LEFT_SHOULDER)
	{
		TransitionAvoidanceState();
	}

}

void Player::InputWing()
{
	//Xボタン押すと飛行モードになる
	if (gamePad->GetButtonDown() & GamePad::BTN_B)
	{
		TransitionWingState();
	}
}

void Player::InputShot()
{
	// 弾管理クラスのインスタンス取得
	BulletManager& bulletManager = BulletManager::Instance();

	// 剣（ビームサーベル）の位置をアニメーションのボーン情報から取得
	model->fech_by_bone(playerAnimation, time, transform, beamSaber[LR::RIGHT], beamSaber_position[LR::RIGHT]);
	model->fech_by_bone(playerAnimation, time, transform, beamSaber[LR::LEFT], beamSaber_position[LR::LEFT]);

	DirectX::XMFLOAT3 dir{}; // 発射方向
	DirectX::XMFLOAT3 pos{}; // 発射位置

	// 発射位置の決定（プレイヤーの腰あたりのビームサーベル位置）
	if (playerAnimation == PlayerAnimation::PLAYER_SHOT_RIGHT
		|| playerAnimation == PlayerAnimation::PLAYER_SHOT_BACK)
	{
		// 右手側のビームサーベル位置
		pos = beamSaber_position[LR::RIGHT];
	}
	else
	{
		// 左手側のビームサーベル位置
		pos = beamSaber_position[LR::LEFT];
	}

	// ロックオンしていなかったらカメラの前方方向に発射
	if (!camera->GetLockOn())
	{
		// プレイヤーの向いている方向を取得
		dir = Math::get_posture_forward(transform);
	}
	else
	{
		// ロックオン対象（ボス）に向かう方向を計算
		dir = Math::calc_vector_AtoB_normalize(pos, bossPosition);
	}

	// 直線弾（BulletStraight）の生成
	BulletStraight* bullet =
		new BulletStraight(&BulletManager::Instance(), Bullet::BULLET_MASTER::Player);

	// 弾を発射
	bullet->Launch(dir, pos);

	//射撃音
	audios[PLAYER_SE::SE_LASER]->play();
	audios[PLAYER_SE::SE_LASER]->volume(0.3f);
}

void Player::OnLanding()
{
	// ジャンプ回数をリセット（地面に着地したと判断）
	jumpCount = 0;

	// 落下速度が一定以上なら着地ステートへ遷移
	// 坂道を歩いているときに不要な遷移が起こらないように調整
	if (velocity.y < gravity * LANDING_SPEED)
	{
		// 着地ステートへ遷移
		TransitionLandingState();

		// 速度をゼロにリセット（着地したため）
		velocity = { 0,0,0 };
	}
	else
	{
		// 攻撃やダメージ状態でなければ、待機ステート（Idle）へ遷移
		if (state != STATE::SHOT
			&& state != STATE::LEFT_ATTACK
			&& state != STATE::RIGHT_ATTACK
			&& state != STATE::DAMAGE)
		{
			TransitionIdleState();
		}
	}
}

void Player::OnDead()
{
	TransitionDeadState();
}

void Player::OnDamaged(WINCE_TYPE type)
{
	//typeによって偏移する状態を変える
	switch (type)
	{
	case WINCE_TYPE::NONE:
		break;
	case WINCE_TYPE::SMALL:
		TransitionDamageState();
		break;
	case WINCE_TYPE::BIG:
		break;
	default:
		break;
	}

}

bool Player::ApplyDamage(int damage, float invincible_time, WINCE_TYPE type)
{
	//ダメージが0の場合は健康状態を変更する必要がない
	if (damage == 0)return false;

	//死亡している場合は健康状態を変更しない
	if (health <= 0)return false;

	//無敵時間がある場合は変更しない
	if (invincibleTimer > 0.0f)return false;

	//無敵時間設定
	invincibleTimer = invincible_time;
	//ダメージ処理
	health -= damage;

	//死亡通知
	if (health <= 0)
	{
		OnDead();
	}
	else//ダメージ通知
	{
		OnDamaged(type);
	}

	//ダメージ効果音
	audios[PLAYER_SE::SE_DAMAGE]->play();
	audios[PLAYER_SE::SE_DAMAGE]->volume(0.5f);

	//健康状態が変更した場合はtrueを返す
	return true;

}

bool Player::Floating()
{
	//落下中なら
	if (velocity.y < 0)
	{
		//落下速度を弱める
		velocity.y /= param.floatingValue;
		return true;
	}
	//浮遊中でない
	return false;
}

bool Player::Flying()
{
    return false;
}

void Player::UpdateVerticalVelocity(float elapsed_frame)
{
	// プレイヤーのアニメーションがPLAYER_WING_STARTでない場合、重力を適用
	if (playerAnimation != PlayerAnimation::PLAYER_WING_START)
		velocity.y += gravity * elapsed_frame;
	// プレイヤーのアニメーションがPLAYER_WING_STARTの場合、重力の影響を20%に減少
	else if (playerAnimation == PlayerAnimation::PLAYER_WING_START)
		velocity.y += (gravity * 0.2f) * elapsed_frame;
}

void Player::LoadDataFile()
{
	// Jsonファイルから値を取得
	std::filesystem::path path = filePath;
	path.replace_extension(".json");  // 拡張子を.jsonに変更
	if (std::filesystem::exists(path.c_str()))  // ファイルが存在する場合
	{
		std::ifstream ifs;
		ifs.open(path);  // ファイルを開く
		if (ifs)
		{
			cereal::JSONInputArchive o_archive(ifs);  // JSON形式でファイルを読み込み
			o_archive(param);  // データをparamにデシリアライズ
		}
	}
}

void Player::SaveDataFile()
{
	// ベースクラスの初期化パラメーター情報を更新
	param.charaInitParam = charaParam;
	// Jsonファイルに値を保存
	std::filesystem::path path = filePath;
	path.replace_extension(".json");  // 拡張子を.jsonに変更
	std::ofstream ifs;
	ifs.open(path);  // ファイルを開く
	if (ifs)
	{
		cereal::JSONOutputArchive o_archive(ifs);  // JSON形式でファイルに書き込み
		o_archive(param);  // paramのデータをシリアライズして保存
	}
}
void Player::DebugPrimitiveUpdate()
{
	// デバッグレンダラーのインスタンスを取得
	DebugRenderer* debugRender = Graphics::Instance().GetDebugRenderer();

	// サーベルの当たり判定処理
	{
		// ボーン位置を取得して、サーベルや下腕の位置を更新
		model->fech_by_bone(playerAnimation, time, transform, beamSaber[LR::LEFT], beamSaber_position[LR::LEFT]);
		model->fech_by_bone(playerAnimation, time, transform, lowerArm[LR::LEFT], lowerArm_position[LR::LEFT]);
		model->fech_by_bone(playerAnimation, time, transform, beamSaber[LR::RIGHT], beamSaber_position[LR::RIGHT]);
		model->fech_by_bone(playerAnimation, time, transform, lowerArm[LR::RIGHT], lowerArm_position[LR::RIGHT]);

		// サーベルの位置を求める関数（腕とサーベルの位置から中間点を計算）
		std::function<DirectX::XMFLOAT3(DirectX::XMFLOAT3&, DirectX::XMFLOAT3&)> saber_position{
			[](DirectX::XMFLOAT3& arm, DirectX::XMFLOAT3& saber)->DirectX::XMFLOAT3 {

				DirectX::XMFLOAT3 Arm{ arm };
				DirectX::XMFLOAT3 Saber{ saber };

				// 腕とサーベルの方向ベクトルと距離を計算
				DirectX::XMFLOAT3 direction = Math::calc_vector_AtoB_normalize(Arm, Saber);
				float length = Math::calc_vector_AtoB_length(Arm, Saber);

				// 腕の位置から方向ベクトルを使って、長さの半分の位置を計算
				return Math::calc_designated_point(Arm, direction, length * 0.5f);
		} };

		// サーベルの衝突判定位置を計算
		attackCollision_position[LR::LEFT] = saber_position(lowerArm_position[LR::LEFT], beamSaber_position[LR::LEFT]);
		attackCollision_position[LR::RIGHT] = saber_position(lowerArm_position[LR::RIGHT], beamSaber_position[LR::RIGHT]);

		// 攻撃が有効な場合、デバッグ用にサーベルの位置に球体を描画
		if (attackParam.isAttack)
		{
			debugRender->CreateSphere(
				attackCollision_position[LR::LEFT],
				1.0f, { 1.0f,0.0f,0.0f,1.0f });  // 左側のサーベルの位置
			debugRender->CreateSphere(
				attackCollision_position[LR::RIGHT],
				1.0f, { 1.0f,0.0f,0.0f,1.0f });  // 右側のサーベルの位置
		}
	}

	// 自分の当たり判定を描画（円柱形状で表示）
	debugRender->CreateCylinder(collider.start,
		collider.radius,
		charaParam.height,
		{ 0.0f,1.0f,0.0f,1.0f });  // 自キャラの当たり判定を緑色で表示
}

void Player::DebugGUI()
{
#ifdef USE_IMGUI
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
	imguiMenuBar("Character", "player", displayPlayerImgui);

	if (displayPlayerImgui)
	{

		if (ImGui::Begin("Player", nullptr, ImGuiWindowFlags_None))
		{
			//カメラ
			//トランスフォーム
			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
			{

				//位置
				ImGui::DragFloat3("Position", &position.x);
				ImGui::DragFloat3("Scale", &scale.x);
				//回転
				DirectX::XMFLOAT3 forward;
				DirectX::XMStoreFloat3(&forward, Math::get_posture_forward_vec(orientation));
				ImGui::DragFloat3("forward", &forward.x);
				ImGui::DragFloat4("ori", &orientation.x);
				std::string state_name;
				state_name = magic_enum::enum_name<STATE>(state);
				ImGui::Text(state_name.c_str());
				ImGui::DragFloat3("velocity:", &velocity.x);
				ImGui::DragFloat("moveVec_x:", &moveVec_x);
				ImGui::DragFloat("moveVec_y:", &moveVec_y);
				ImGui::DragFloat("moveVec_z:", &moveVec_z);
			}
			if (ImGui::CollapsingHeader("Param", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::DragFloat("height", &charaParam.height);
				ImGui::DragInt("max_health", &charaParam.maxHealth);
				ImGui::DragInt("health", &health);
				ImGui::DragFloat("radius", &charaParam.radius);
				ImGui::DragFloat("gravity", &gravity);
				ImGui::DragFloat("floating_value", &param.floatingValue);
				ImGui::DragFloat("attack_MoveSpeed", &param.attackMoveSpeed);
				ImGui::DragFloat("invinsible_timer", &invincibleTimer);
				ImGui::DragFloat("boostTimer", &param.boostTimer);
				ImGui::DragFloat("TurnSpeed", &charaParam.turnSpeed, 0.1f);
				ImGui::DragFloat("MoveSpeed", &charaParam.moveSpeed, 0.1f);
				ImGui::DragFloat("wingSpeed", &param.wingSpeed, 0.1f);
				ImGui::DragFloat("avoidance_speed", &param.avoidanceSpeed);
				ImGui::DragFloat("friction", &charaParam.friction);
				ImGui::DragFloat("acceleration", &charaParam.acceleration);
				ImGui::DragFloat("jump_speed", &param.jumpSpeed);
				ImGui::DragFloat("air_control", &charaParam.airControl);
				ImGui::Checkbox("is_ground", &isGround);
				float control_x = gamePad->GetAxis_LX();
				float control_y = gamePad->GetAxis_LY();
				ImGui::DragFloat("control_x", &control_x);
				ImGui::DragFloat("control_y", &control_y);

			}
			if (ImGui::Button("load"))
			{
				LoadDataFile();
			}
			ImGui::Separator();
			if (ImGui::Button("save"))
			{
				SaveDataFile();
			}
			ImGui::Text("attack_param");
			if (ImGui::CollapsingHeader("combo1"))
			{
				ImGui::DragInt("combo1_power", &param.combo_1.power, 0.1f);
				ImGui::DragFloat("combo1_invinsible_time", &param.combo_1.invinsibleTime, 0.1f);

				ImGui::Text("combo1_camera_shake");
				ImGui::DragFloat("combo1_shake_x", &param.combo_1.cameraShake.max_X_shake, 0.1f);
				ImGui::DragFloat("combo1_shake_y", &param.combo_1.cameraShake.max_Y_shake, 0.1f);
				ImGui::DragFloat("combo1_time", &param.combo_1.cameraShake.time, 0.1f);
				ImGui::DragFloat("combo1_smmoth", &param.combo_1.cameraShake.shakeSmoothness, 0.1f, 0.1f, 1.0f);

				ImGui::Text("hit_stop");
				ImGui::DragFloat("combo1_stop_time", &param.combo_1.hitStop.time, 0.1f);
				ImGui::DragFloat("combo1_hit_viberation.l_moter", &param.combo_1.hitViberation.L_moter, 0.1f);
				ImGui::DragFloat("combo1_hit_viberation.r_moter", &param.combo_1.hitViberation.R_moter, 0.1f);
				ImGui::DragFloat("combo1_vibe_time", &param.combo_1.hitViberation.VibeTime, 0.1f);
			}
			if (ImGui::CollapsingHeader("combo2"))
			{
				ImGui::DragInt("combo2_power", &param.combo_2.power, 0.1f);
				ImGui::DragFloat("combo2_invinsible_time", &param.combo_2.invinsibleTime, 0.1f);

				ImGui::Text("combo2_camera_shake");
				ImGui::DragFloat("combo2_shake_x", &param.combo_2.cameraShake.max_X_shake, 0.1f);
				ImGui::DragFloat("combo2_shake_y", &param.combo_2.cameraShake.max_Y_shake, 0.1f);
				ImGui::DragFloat("combo2_time", &param.combo_2.cameraShake.time, 0.1f);
				ImGui::DragFloat("combo2_smmoth", &param.combo_2.cameraShake.shakeSmoothness, 0.1f, 0.1f, 1.0f);
				ImGui::Text("hit_stop");
				ImGui::DragFloat("combo2_stop_time", &param.combo_2.hitStop.time, 0.1f);
				ImGui::DragFloat("combo2_hit_viberation.l_moter", &param.combo_2.hitViberation.L_moter, 0.1f);
				ImGui::DragFloat("combo2_hit_viberation.r_moter", &param.combo_2.hitViberation.R_moter, 0.1f);
				ImGui::DragFloat("combo2_vibe_time", &param.combo_2.hitViberation.VibeTime, 0.1f);
			}

			if (ImGui::CollapsingHeader("combo3"))
			{
				ImGui::DragInt("combo3_power", &param.combo_3.power, 0.1f);
				ImGui::DragFloat("combo3_invinsible_time", &param.combo_3.invinsibleTime, 0.1f);

				ImGui::Text("combo3_camera_shake");
				ImGui::DragFloat("combo3_shake_x", &param.combo_3.cameraShake.max_X_shake, 0.1f);
				ImGui::DragFloat("combo3_shake_y", &param.combo_3.cameraShake.max_Y_shake, 0.1f);
				ImGui::DragFloat("combo3_time", &param.combo_3.cameraShake.time, 0.1f);
				ImGui::DragFloat("combo3_smmoth", &param.combo_3.cameraShake.shakeSmoothness, 0.1f, 0.1f, 1.0f);
				ImGui::Text("hit_stop");
				ImGui::DragFloat("combo3_stop_time", &param.combo_3.hitStop.time, 0.1f);
				ImGui::DragFloat("combo3_hit_viberation.l_moter", &param.combo_3.hitViberation.L_moter, 0.1f);
				ImGui::DragFloat("combo3_hit_viberation.r_moter", &param.combo_3.hitViberation.R_moter, 0.1f);
				ImGui::DragFloat("combo3_vibe_time", &param.combo_3.hitViberation.VibeTime, 0.1f);
			}

			if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen))
			{
				const char* anime_item[] = {
					"PLAYER_IDLE",
					"PLAYER_MOVE_FORWARD",
					"PLAYER_MOVE_LEFT",
					"PLAYER_MOVE_RIGHT",
					"PLAYER_MOVE_BACK",
					"PLAYER_JUMP_INIT",
					"PLAYER_JUMP_FALL",
					"PLAYER_JUMP_END",
					"PLAYER_TRANSITION_WING",
					"PLAYER_WING",
					"PLAYER_TRANSITION_IDLE",
					"PLAYER_TRANSITION_KILL",
					"PLAYER_KILL_POWERL",
					"PLAYER_KILL_ATTACK_R01",
				};
				static int item_current = 0;
				static bool loop = false;
				ImGui::Combo("anime", &item_current, anime_item, IM_ARRAYSIZE(anime_item)); ImGui::Checkbox("is_loop", &loop);
				if (ImGui::Button("play", { 80,20 }))
				{
					playerAnimation = static_cast<PlayerAnimation>(item_current);
				}
				ImGui::SliderFloat("transition_time", &transition_time, 0.0f, 5.0f);

			}

		}
		ImGui::End();

	}

	//UIの描画
	ui->DebugGUI();
#endif // USE_IMGUI

}

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

PLAYER::PLAYER()
{
	//インスタンス取得
	Graphics& graphics = Graphics::Instance();
	//キャラクターモデルを読み込む
	model = std::make_unique<gltf_model>(graphics.GetDevice().Get(),
		"Resources/Character/PLAYER/glb/white_crow.glb", true);
	
	//エフェクト作成（斬撃エフェクト）
	slashEffect = std::make_unique<Effect>("Resources/Effect/Slash/slash.efkefc");

	// 効果音の読み込み
	audios[ToInt(PLAYER_SE::SE_SABER)] = audio::_emplace(L"Resources/Sound/SE/saber.wav");
	audios[ToInt(PLAYER_SE::SE_LASER)] = audio::_emplace(L"Resources/Sound/SE/laser.wav");
	audios[ToInt(PLAYER_SE::SE_BOOST)] = audio::_emplace(L"Resources/Sound/SE/boost.wav");
	audios[ToInt(PLAYER_SE::SE_DAMAGE)] = audio::_emplace(L"Resources/Sound/SE/Damage.wav");

	// モデルのトランスフォームデータを累積
	model->cumulate_transforms(model->nodes, transform);

	// アニメーションノードを初期化
	for (auto& node : animatedNodes)
	{
		node = model->nodes;
	}
	blendedAnimatedNodes = model->nodes;

	// UIを初期化
	ui = std::make_unique<PlayerUI>();

	// 武器ノードの取得
	beamSaber[ToInt(LR::LEFT)] = model->find_nodes("Left_wep1");
	beamSaber[ToInt(LR::RIGHT)] = model->find_nodes("Right_wep1");
	lowerArm[ToInt(LR::LEFT)] = model->find_nodes("lowerarm_l");
	lowerArm[ToInt(LR::RIGHT)] = model->find_nodes("lowerarm_r");

	// 入力デバイスの取得
	mouse = &Device::Instance().GetMouse();
	gamePad = &Device::Instance().GetGamePad();
	camera = &Camera::Instance();

	// 初期化処理を実行
	Initialize();

}

void PLAYER::Initialize()
{
	//パラメーターロード
	LoadDataFile();

	//各変数初期化
	//初期位置設定
	position = INIT_POSITION;

	//スケールを2倍に設定
	scale.x = scale.y = scale.z = PLAYER_SCALE;

	//Characterクラスのパラメーター初期化
	charaParam = param.charaInitParam;

	//当たり判定半径設定
	collider.radius = COLLIDER_RADIUS;

	//体力を設定
	health = charaParam.maxHealth;
	//移動速度とジャンプ回数を設定
	jumpCount = jumpLimit;

	//ステートマシンの初期化
	TransitionIdleState();

	//コンボ01のフレーム初期化
	attackFlameParam[ToInt(COMBO::ATTACK01)].startFlame		= ATTACK01_START;
	attackFlameParam[ToInt(COMBO::ATTACK01)].endFlame		= ATTACK01_END;
	attackFlameParam[ToInt(COMBO::ATTACK01)].preInputFlame	= ATTACK01_PREINPUT;
	//コンボ02のフレーム初期化
	attackFlameParam[ToInt(COMBO::ATTACK02)].startFlame		= ATTACK02_START;
	attackFlameParam[ToInt(COMBO::ATTACK02)].endFlame		= ATTACK02_END;
	attackFlameParam[ToInt(COMBO::ATTACK02)].preInputFlame	= ATTACK02_PREINPUT;
	
	//コンボ03のフレーム初期化
	attackFlameParam[ToInt(COMBO::ATTACK03)].startFlame		= ATTACK03_START;
	attackFlameParam[ToInt(COMBO::ATTACK03)].endFlame		= ATTACK03_END;
	attackFlameParam[ToInt(COMBO::ATTACK03)].preInputFlame	= ATTACK03_PREINPUT;

	// 被ダメージ時の処理を設定
	damagedFunction = [=](int damage, float invincible, WINCE_TYPE type)->bool {return ApplyDamage(damage, invincible, type); };
}

PLAYER::~PLAYER()
{
}

void PLAYER::Update(float elapsedTime)
{
	//インスタンス取得
	Graphics& graphics = Graphics::Instance();

	//-----------------ブースト更新-----------------//
	BoostUpdate(elapsedTime);

	//軌跡の初期化
	resetTrail = false;

	//-----------------ステート更新処理-----------------//
	(this->*pUpdate)(elapsedTime);

	//YボタンかVキーを押すとロックオンする
	if (gamePad->GetButtonDown() & GamePad::BTN_Y)
	{
		camera->SetLockOn();
	}
	//飛行と射撃状態の時はカメラの姿勢を使う
	if (state == STATE::WING || state == STATE::SHOT)
	{
		orientation = camera->GetOrientation();
	}

	//プレイヤーの正面情報を更新
	forward = Math::GetPostureForward(orientation);
	
	//-----------------無敵時間の更新-----------------//
	UpdateInvicibleTimer(elapsedTime);

	//-----------------デバッグプリミティブ更新-----------------//
	DebugPrimitiveUpdate();

	//-----------------軌跡更新-----------------//
	TrailUpdate();

	//-----------------当たり判定カプセル更新-----------------//
	collider.start = position;
	collider.end = { position.x,position.y + charaParam.height, position.z };

	//攻撃モーションじゃない場合攻撃当たり判定オフ
	if (state != STATE::LEFT_ATTACK && state != STATE::RIGHT_ATTACK)
	{
		attackParam.isAttack = false;
	}

	//画面外に行った場合初期位置に戻す
	if (position.y < LIMIT_Y)
	{
		position.y = RESPAWN_Y;
	}

	//-----------------シェーダー更新-----------------//
	ShaderUpdate(elapsedTime);

	//-----------------UI更新-----------------//
	ui->SetHPPercent(GetHpPercent());
	ui->SetBoostPercent(GetBoostPercent());
	if (camera->GetLockOn())
	{
		ui->SetLockonPosition(bossPosition);
		ui->SetLockonDistance(Math::CalcVectorAtoBLength(position, bossPosition));
	}
	ui->Update(elapsedTime);
}

void PLAYER::Render_f(float elapsedTime)
{
	//グラフィックスインスタンスを取得
	Graphics& graphics = Graphics::Instance();

	//自機モデルのトランスフォームを更新（ワールド行列を計算）
	transform = Math::CalcWorldMatrix(scale, orientation, position, Math::COORDINATE_SYSTEM::RHS_YUP);

	//アニメーションの遷移チェック
	if (playerAnimation_transition != playerAnimation)
	{
		if (transitionState != TRANSITION_STATE::NONE)
		{
			//現在の遷移アニメーションを保存
			playerAnimation_old = playerAnimation_transition;
			animatedNodes[ToInt(ANIME_NODE::OLD_ANIMATION)] = blendedAnimatedNodes;
			transitionToTransition = true;
		}
		//新しいアニメーションへの遷移開始
		playerAnimation_transition = playerAnimation;
		transitionState = TRANSITION_STATE::START;
	}

	//現在のアニメーションがループするか判定
	bool isLoop = FindLoopAnimation(playerAnimation);

	//ブレンドアニメーション処理
	if (transitionState > TRANSITION_STATE::NONE && transitionTime > 0.0f)
	{
		switch (transitionState)
		{
		case TRANSITION_STATE::NONE:
			break;
		case TRANSITION_STATE::START:
			if (!transitionToTransition)
			{
				//直前のアニメーションを設定
				model->animate(ToInt(playerAnimation_old), time, animatedNodes[ToInt(ANIME_NODE::OLD_ANIMATION)],
					FindLoopAnimation(playerAnimation_old));
			}
			//新しいアニメーションを0秒の状態から設定
			model->animate(ToInt(playerAnimation), 0.0f, animatedNodes[ToInt(ANIME_NODE::NOW_ANIMATION)], isLoop);

			//遷移ステートを「移行中」に設定
			transitionState = TRANSITION_STATE::TRANSITION;
			time = 0.0f;	//時間のリセット
			factor = 0.0f;	//遷移係数の初期化
			break;

		case TRANSITION_STATE::TRANSITION:
			//アニメーション遷移のブレンド率を計算
			factor = time / transitionTime;

			//旧アニメーションと新アニメーションをブレンド
			model->blend_animations(animatedNodes[ToInt(ANIME_NODE::OLD_ANIMATION)],
				animatedNodes[ToInt(ANIME_NODE::NOW_ANIMATION)],
				factor, blendedAnimatedNodes);

			//経過時間を加算
			time += elapsedTime;

			//遷移完了判定
			if (factor > FACTOR_MAX)
			{
				//遷移終了処理
				transitionToTransition = false;
				transitionState = TRANSITION_STATE::NONE;
				time = 0;
			}
			break;
		}
		//ブレンド後のアニメーションを描画
		model->render(graphics.Get_DC().Get(), transform, blendedAnimatedNodes);
	}
	else
	{
		//通常アニメーションの更新
		time += elapsedTime;

		//アニメーションが終了した場合の処理
		if (model->animations.at(ToInt(playerAnimation)).duration < time)
		{
			if (isLoop)
			{
				time = 0;  //ループする場合は最初に戻す
			}
			else
			{
				// ループしない場合は最後のフレームで停止
				time = model->animations.at(ToInt(playerAnimation)).duration;
			}
		}

		//アニメーションを適用
		model->animate(ToInt(playerAnimation), time, animatedNodes[ToInt(ANIME_NODE::NOW_ANIMATION)], isLoop);

		//モデルを描画
		model->render(graphics.Get_DC().Get(), transform, animatedNodes[ToInt(ANIME_NODE::NOW_ANIMATION)]);

		//前回のアニメーションを更新
		playerAnimation_old = playerAnimation;
	}
}

void PLAYER::RenderUI(float elapsed_time)
{
	//プレイヤーのUI
	ui->Render();
	
}

void PLAYER::CalcCollision_vs_Enemy(Capsule capsule_collider, float collider_height)
{
	//身体の押し出し判定
	Collision::CylinderVsCylinder(
		capsule_collider.start, capsule_collider.radius, collider_height,
		position, charaParam.radius, charaParam.height, &position);

}

void PLAYER::CalcAttack_vs_Enemy(Capsule capsule_collider, float collider_height, AddDamageFunc damaged_func)
{
	//攻撃フラグがオフなら終わる
	if (!attackParam.isAttack) return;

	//どっちの腕で攻撃するか
	int side = 0;
	if (state == STATE::LEFT_ATTACK) side = ToInt(LR::LEFT);
	else if (state == STATE::RIGHT_ATTACK) side = ToInt(LR::RIGHT);

	//攻撃時の当たり判定
	if (Collision::SphereVsCylinder(attackCollisionPosition[side], ATTACK_RADIUS,
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
			slashEffect->Play(attackCollisionPosition[side], ATTACK_RADIUS);
		}
	}
}

bool PLAYER::FindLoopAnimation(PlayerAnimation PA)
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

void PLAYER::Move(float vx, float vz, float speed)
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

void PLAYER::Move(float vx, float vy, float vz, float speed)
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

void PLAYER::BoostUpdate(float elapsedTime)
{
	//ブーストステートじゃないかつ地面に接していたらブーストゲージを回復する
	if (isGround && STATE::BOOST != state)
	{
		param.boostTimer += elapsedTime * CHARGE_SPEED;
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

bool PLAYER::InputMove(float elapsedTime)
{
	//進行ベクトル取得
	const DirectX::XMFLOAT3 moveVec = GetMoveVec(camera);

	//移動処理
	Move(moveVec.x, moveVec.z, charaParam.moveSpeed);
	Turn(elapsedTime, camera->GetForward(), charaParam.turnSpeed, orientation);

	return moveVec.x != 0.0f || moveVec.y != 0.0f || moveVec.z != 0.0f;
}

bool PLAYER::InputMove(float elapsedTime, float restrictionMove, float restrictionTurn)
{
	//進行ベクトル取得
	const DirectX::XMFLOAT3 move_vec = GetMoveVec(camera);

	//移動処理
	Move(move_vec.x, move_vec.z, charaParam.moveSpeed / restrictionMove);
	Turn(elapsedTime, camera->GetForward(), charaParam.turnSpeed / restrictionTurn, orientation);

	return move_vec.x != 0.0f || move_vec.y != 0.0f || move_vec.z != 0.0f;
}

bool PLAYER::InputMoveWing(float elapsedTime)
{
	//進行ベクトル取得
	const DirectX::XMFLOAT3 move_vec = GetMoveVec(camera);

	//移動処理
	Move(move_vec.x,move_vec.y, move_vec.z, charaParam.moveSpeed);
	Turn(elapsedTime, move_vec, charaParam.turnSpeed, orientation);

	return move_vec.x != 0.0f || move_vec.y != 0.0f || move_vec.z != 0.0f;
}

const DirectX::XMFLOAT3 PLAYER::GetMoveVec(Camera* camera, bool wing) const
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

void PLAYER::ShaderUpdate(float elapsedTime)
{
	//ラジアルブラータイマーがオフの場合
	if (player_RadialBlurConstant.blurStrength <= 0 
		|| radialTimer <= 0)
	{
		player_RadialBlurConstant.blurStrength = 0.0f;
		player_RadialBlurConstant.blurRadius = 0.0f;
	}
	else
	{
		//飛行モード時
		if (state == STATE::WING)
		{
			//時間経過に応じてラジアルブラーを減衰
			player_RadialBlurConstant.blurStrength -= elapsedTime;
			player_RadialBlurConstant.blurRadius -= elapsedTime;
		}
		//ブースト時
		else
		{
			//タイマーに基づいてブラー強度を補間
			float factor = radialTimer / player_RadialBlurConstant.blurTimer;
			radialTimer -= elapsedTime;

			player_RadialBlurConstant.blurStrength = factor;
		}
	}
	
	//色収差処理
	//ダメージを食らったとき
	if (isGlitch_CA && glitch_CATimer > 0)
	{
		//時間経過に応じてグリッチの強度を減衰
		float factor = glitch_CATimer / GLITCH_MAX_DURATION;
		glitch_CATimer -= GLITCH_MAX_DURATION * elapsedTime;

		//色収差のパラメータ設定
		player_Glitch_CA_Constant.density = factor;
		player_Glitch_CA_Constant.shift = GLITCH_SHIFT_AMOUNT;
		player_Glitch_CA_Constant.XShifting = GLITCH_SHIFT_AMOUNT;
		player_Glitch_CA_Constant.YShifting = GLITCH_SHIFT_AMOUNT;
	}
	//平常時
	else
	{
		//色収差を全てリセット
		player_Glitch_CA_Constant.center = { GLITCH_CENTER_X, GLITCH_CENTER_Y };
		player_Glitch_CA_Constant.brightness = 0.0f;
		player_Glitch_CA_Constant.density = 0.0f;
		player_Glitch_CA_Constant.extension = 0.0f;
		player_Glitch_CA_Constant.glitchMaskRadius = 0.0f;
		player_Glitch_CA_Constant.glitchSamplingCount = 0.0f;
		player_Glitch_CA_Constant.randFloat = 0.0f;
		player_Glitch_CA_Constant.shift = 0.0f;
		player_Glitch_CA_Constant.uvSlider = 0.0f;
		player_Glitch_CA_Constant.XShift = { 0,0 };
		player_Glitch_CA_Constant.XShifting = 0.0f;
		player_Glitch_CA_Constant.YShift = { 0,0 };
		player_Glitch_CA_Constant.YShifting = 0.0f;
	}
}

void PLAYER::InputJump()
{
	//スペースを押したらジャンプ
	if (gamePad->GetButtonDown() & GamePad::BTN_A)
	{
		//ジャンプ回数が上限に達していなければジャンプを実行
		if (jumpCount < jumpLimit)
		{
			//ジャンプ状態へ遷移
			TransitionJumpState();
			Jump(param.jumpSpeed);

			//ジャンプ直後に地面にいると判定されないように強制的にfalseにする
			isGround = false;

			//ジャンプ回数を加算
			++jumpCount;
		}
	}
}

void PLAYER::InputBoost()
{
	//ブースト量が25%以下だと出来ない
	if (param.boostTimer < BOOST_MIN_THRESHOLD)return;

	//右トリガーを押したら回避
	if (gamePad->GetButtonDown() & GamePad::BTN_LEFT_SHOULDER)
	{
		TransitionBoostState();
	}
}

void PLAYER::InputWing()
{
	//Xボタン押すと飛行モードになる
	if (gamePad->GetButtonDown() & GamePad::BTN_B)
	{
		TransitionWingState();
	}
}

void PLAYER::InputShot()
{
	// 弾管理クラスのインスタンス取得
	BulletManager& bulletManager = BulletManager::Instance();

	// 剣（ビームサーベル）の位置をアニメーションのボーン情報から取得
	model->fech_by_bone(ToInt(playerAnimation), time, transform, beamSaber[ToInt(LR::RIGHT)], beamSaberPosition[ToInt(LR::RIGHT)]);
	model->fech_by_bone(ToInt(playerAnimation), time, transform, beamSaber[ToInt(LR::LEFT)], beamSaberPosition[ToInt(LR::LEFT)]);

	DirectX::XMFLOAT3 dir{}; // 発射方向
	DirectX::XMFLOAT3 pos{}; // 発射位置

	// 発射位置の決定（プレイヤーの腰あたりのビームサーベル位置）
	if (playerAnimation == PlayerAnimation::PLAYER_SHOT_RIGHT
		|| playerAnimation == PlayerAnimation::PLAYER_SHOT_BACK)
	{
		// 右手側のビームサーベル位置
		pos = beamSaberPosition[ToInt(LR::RIGHT)];
	}
	else
	{
		// 左手側のビームサーベル位置
		pos = beamSaberPosition[ToInt(LR::LEFT)];
	}

	// ロックオンしていなかったらカメラの前方方向に発射
	if (!camera->GetLockOn())
	{
		// プレイヤーの向いている方向を取得
		dir = Math::GetPostureForward(transform);
	}
	else
	{
		// ロックオン対象（ボス）に向かう方向を計算
		dir = Math::CalcVectorAtoBNormalize(pos, bossPosition);
	}

	// 直線弾（BulletStraight）の生成
	BulletStraight* bullet =
		new BulletStraight(&BulletManager::Instance(), Bullet::BULLET_MASTER::PLAYER);

	// 弾を発射
	bullet->Launch(dir, pos);

	//射撃音
	audios[ToInt(PLAYER_SE::SE_LASER)]->play();
	audios[ToInt(PLAYER_SE::SE_LASER)]->volume(SOUND_VOLUME_LASER);
}

void PLAYER::OnLanding()
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
		velocity = {};
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

void PLAYER::CheckPreInput(COMBO combo)
{
	//先行入力のチェック
	if (gamePad->GetButtonDown() & gamePad->BTN_X)
	{
		nextCombo = true;
	}

	//一定時間経過後に攻撃判定をオン
	if (attackFlameParam[ToInt(combo)].startFlame < time && !attackParam.isAttack)
	{
		attackParam.isAttack = true;
	}

	//先行入力があれば次のコンボへ遷移
	if (attackFlameParam[ToInt(combo)].preInputFlame < time && nextCombo)
	{
		switch (combo)
		{
		case PLAYER::COMBO::ATTACK01:
			TransitionCombo02State();
			break;
		case PLAYER::COMBO::ATTACK02:
			TransitionCombo03State();
			break;
		case PLAYER::COMBO::ATTACK03:
			//最後のコンボだから偏移しない
			break;
		}
		attackParam.isAttack = false;
	}

	//攻撃判定をオフにするタイミング
	if (attackFlameParam[ToInt(combo)].endFlame < time)
	{
		attackParam.isAttack = false;
	}
}

void PLAYER::OnDead()
{
	//死亡状態へ偏移
	TransitionDeadState();
}

void PLAYER::OnDamaged(WINCE_TYPE type)
{
	//typeによって偏移する状態を変える
	switch (type)
	{
	case WINCE_TYPE::NONE:
		break;
	case WINCE_TYPE::SMALL:
		//怯みダメージ
		TransitionDamageState();
		break;
	case WINCE_TYPE::BIG:
		break;
	default:
		break;
	}

}

bool PLAYER::ApplyDamage(int damage, float invincible_time, WINCE_TYPE type)
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
	audios[ToInt(PLAYER_SE::SE_DAMAGE)]->play();
	audios[ToInt(PLAYER_SE::SE_DAMAGE)]->volume(SOUND_VOLUME_DAMAGE);

	//健康状態が変更した場合はtrueを返す
	return true;

}

void PLAYER::TrailUpdate()
{
	//攻撃状態でない、またはトレイルリセットフラグが立っている場合
	if ((state != STATE::LEFT_ATTACK
		&& state != STATE::RIGHT_ATTACK
		&& !attackParam.isAttack)
		|| resetTrail
		)
	{
		//全ての頂点を現在の腕とサーベルの位置に固定し、透明化する
		for (int lr = 0; lr < ToInt(LR::COUNT); lr++)
		{
			for (int i = MAX_POLYGON - 1; i >= 0; --i)
			{
				trailAttack[lr].trailPositions[ToInt(TRAIL::LOWER_ARM)][i] = lowerArmPosition[lr];
				trailAttack[lr].trailPositions[ToInt(TRAIL::BEAM_SABER)][i] = beamSaberPosition[lr];
				trailAttack[lr].color[i] = { 1.0f,0.0f,1.0f,0.0f };
			}
		}

		//resetTrailがtrueであればフラグをリセットして抜けずに処理継続
		if (resetTrail)
		{
			resetTrail = false;
		}
		//それ以外は処理終了
		else
		{ 
			return;
		}
	}

	//頂点バッファを1フレーム分後ろにずらす
	for(int lr = 0;lr<ToInt(LR::COUNT);lr++)
	{
		for (int i = MAX_POLYGON - 1; i > 0; --i)
		{
			trailAttack[lr].trailPositions[ToInt(TRAIL::LOWER_ARM)][i] = trailAttack[lr].trailPositions[ToInt(TRAIL::LOWER_ARM)][i - 1];
			trailAttack[lr].trailPositions[ToInt(TRAIL::BEAM_SABER)][i] = trailAttack[lr].trailPositions[ToInt(TRAIL::BEAM_SABER)][i - 1];
			
			//透明度をフレームごとに下げていく
			trailAttack[lr].color[i] = { 1.0f,0.0f,1.0f,1.0f - (static_cast<float>(i) / (MAX_POLYGON - 1)) };

		}
	}

	//左右どちらの腕で攻撃しているかを判定
	int side = 0;
	if (state == STATE::LEFT_ATTACK)
	{
		side = ToInt(LR::LEFT);
	}
	else if (state == STATE::RIGHT_ATTACK)
	{
		side = ToInt(LR::RIGHT);
	}
	
	//現在の腕・サーベルの位置をtrailの先頭に保存
	trailAttack[side].trailPositions[ToInt(TRAIL::LOWER_ARM)][0] = lowerArmPosition[side];
	trailAttack[side].trailPositions[ToInt(TRAIL::BEAM_SABER)][0] = beamSaberPosition[side];
	
	// ポリゴン作成
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();

	//補間点数（分割数）
	const int SPLINE_DIV = 9;
	const float DIV_STEP = 1.0f / static_cast<float>(SPLINE_DIV); //0.1f 相当

	// 保存していた頂点バッファを用いてスプライン補完処理を行い、滑らかなポリゴンを描画
	{
		for (int i = 0; i < MAX_POLYGON - 3; i++)
		{
			primitiveRenderer->AddVertex(trailAttack[side].trailPositions[ToInt(TRAIL::LOWER_ARM)][i], trailAttack[side].color[i]);
			primitiveRenderer->AddVertex(trailAttack[side].trailPositions[ToInt(TRAIL::BEAM_SABER)][i], trailAttack[side].color[i]);
			for (int j = 1; j < SPLINE_DIV; j++)
			{
				//LOWER_ARM側のスプライン補間
				DirectX::XMVECTOR Spline0 =
					DirectX::XMVectorCatmullRom(
						DirectX::XMLoadFloat3(&trailAttack[side].trailPositions[ToInt(TRAIL::LOWER_ARM)][i - 1]),
						DirectX::XMLoadFloat3(&trailAttack[side].trailPositions[ToInt(TRAIL::LOWER_ARM)][i]),
						DirectX::XMLoadFloat3(&trailAttack[side].trailPositions[ToInt(TRAIL::LOWER_ARM)][i + 1]),
						DirectX::XMLoadFloat3(&trailAttack[side].trailPositions[ToInt(TRAIL::LOWER_ARM)][i + 2]),
						j * DIV_STEP
					);

				//BEAM_SABER側のスプライン補間
				DirectX::XMVECTOR Spline1 =
					DirectX::XMVectorCatmullRom(
						DirectX::XMLoadFloat3(&trailAttack[side].trailPositions[ToInt(TRAIL::BEAM_SABER)][i - 1]),
						DirectX::XMLoadFloat3(&trailAttack[side].trailPositions[ToInt(TRAIL::BEAM_SABER)][i]),
						DirectX::XMLoadFloat3(&trailAttack[side].trailPositions[ToInt(TRAIL::BEAM_SABER)][i + 1]),
						DirectX::XMLoadFloat3(&trailAttack[side].trailPositions[ToInt(TRAIL::BEAM_SABER)][i + 2]),
						j * DIV_STEP
					);

				//補間結果をXMFLOAT3に変換
				DirectX::XMFLOAT3 splineposition0;
				DirectX::XMStoreFloat3(&splineposition0, Spline0);
				DirectX::XMFLOAT3 splineposition1;
				DirectX::XMStoreFloat3(&splineposition1, Spline1);

				//先頭フレームでない場合のみ追加
				if (i > 0)
				{
					primitiveRenderer->AddVertex(splineposition0, trailAttack[side].color[i]);
					primitiveRenderer->AddVertex(splineposition1, trailAttack[side].color[i]);
				}
			}
		}
	}
}

void PLAYER::UpdateVerticalVelocity(float elapsed_frame)
{
	//プレイヤーのアニメーションが飛行開始でない場合は通常の重力を適用
	if (playerAnimation != PlayerAnimation::PLAYER_WING_START)
	{
		velocity.y += gravity * elapsed_frame;
	}

	//飛行開始アニメーション中は、重力を軽減
	else if (playerAnimation == PlayerAnimation::PLAYER_WING_START)
	{
		//重力の影響を20%に抑える
		velocity.y += (gravity * WING_GRAVITY_SCALE) * elapsed_frame;
	}
}

void PLAYER::LoadDataFile()
{
	// Jsonファイルから値を取得
	std::filesystem::path path = filePath;

	path.replace_extension(".json");  //拡張子を.jsonに変更

	//ファイルが存在する場合
	if (std::filesystem::exists(path.c_str()))
	{
		std::ifstream ifs;

		//ファイルを開く
		ifs.open(path);  
		if (ifs)
		{
			cereal::JSONInputArchive o_archive(ifs);  //JSON形式でファイルを読み込み
			o_archive(param);  //データをparamにデシリアライズ
		}
	}
}

void PLAYER::SaveDataFile()
{
	// ベースクラスの初期化パラメーター情報を更新
	param.charaInitParam = charaParam;

	// Jsonファイルに値を保存
	std::filesystem::path path = filePath;

	path.replace_extension(".json");  // 拡張子を.jsonに変更
	std::ofstream ifs;

	// ファイルを開く
	ifs.open(path);  
	if (ifs)
	{
		cereal::JSONOutputArchive o_archive(ifs);  // JSON形式でファイルに書き込み
		o_archive(param);  // paramのデータをシリアライズして保存
	}
}

void PLAYER::DebugPrimitiveUpdate()
{
	//デバッグレンダラーのインスタンスを取得
	DebugRenderer* debugRender = Graphics::Instance().GetDebugRenderer();

	//サーベルの当たり判定処理
	{
		//現在アニメーションを元に、各ボーンのワールド位置を取得
		model->fech_by_bone(ToInt(playerAnimation), time, transform, beamSaber[ToInt(LR::LEFT)], beamSaberPosition[ToInt(LR::LEFT)]);
		model->fech_by_bone(ToInt(playerAnimation), time, transform, lowerArm[ToInt(LR::LEFT)], lowerArmPosition[ToInt(LR::LEFT)]);
		model->fech_by_bone(ToInt(playerAnimation), time, transform, beamSaber[ToInt(LR::RIGHT)], beamSaberPosition[ToInt(LR::RIGHT)]);
		model->fech_by_bone(ToInt(playerAnimation), time, transform, lowerArm[ToInt(LR::RIGHT)], lowerArmPosition[ToInt(LR::RIGHT)]);

		//腕とサーベルの中間点を計算するラムダ関数
		std::function<DirectX::XMFLOAT3(DirectX::XMFLOAT3&, DirectX::XMFLOAT3&)> saber_position{
			[](DirectX::XMFLOAT3& arm, DirectX::XMFLOAT3& saber)->DirectX::XMFLOAT3 {

				DirectX::XMFLOAT3 Arm{ arm };
				DirectX::XMFLOAT3 Saber{ saber };

				//方向ベクトルと距離を計算
				DirectX::XMFLOAT3 direction = Math::CalcVectorAtoBNormalize(Arm, Saber);
				float length = Math::CalcVectorAtoBLength(Arm, Saber);

				//腕から見てサーベルまでの中間点（攻撃位置の中心）を返す
				return Math::CalcDesignatedPoint(Arm, direction, length * 0.5f);
		} };

		//サーベルの衝突判定位置を計算
		attackCollisionPosition[ToInt(LR::LEFT)] = saber_position(lowerArmPosition[ToInt(LR::LEFT)], beamSaberPosition[ToInt(LR::LEFT)]);
		attackCollisionPosition[ToInt(LR::RIGHT)] = saber_position(lowerArmPosition[ToInt(LR::RIGHT)], beamSaberPosition[ToInt(LR::RIGHT)]);

		//攻撃が有効な場合、デバッグ用にサーベルの位置に球体を描画
		if (attackParam.isAttack)
		{
			debugRender->CreateSphere(
				attackCollisionPosition[ToInt(LR::LEFT)],
				ATTACK_RADIUS, DEBUG_ATTACK_COLOR);  //左側のサーベルの位置
			debugRender->CreateSphere(
				attackCollisionPosition[ToInt(LR::RIGHT)],
				ATTACK_RADIUS, DEBUG_ATTACK_COLOR);  //右側のサーベルの位置
		}
	}

	// 自分の当たり判定を描画（円柱形状で表示）
	debugRender->CreateCylinder(collider.start,
		collider.radius,
		charaParam.height,
		DEBUG_COLLIDER_COLOR);  // 自キャラの当たり判定を緑色で表示
}

void PLAYER::DebugGUI()
{
#ifdef USE_IMGUI
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
	ImguiMenuBar("Character", "player", displayPlayerImgui);

	if (displayPlayerImgui)
	{

		if (ImGui::Begin("PLAYER", nullptr, ImGuiWindowFlags_None))
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
				DirectX::XMStoreFloat3(&forward, Math::GetPostureForwardVec(orientation));
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
				ImGui::DragFloat("boost_speed", &param.boostSpeed);
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
				ImGui::SliderFloat("transitionTime", &transitionTime, 0.0f, 5.0f);

			}

		}
		ImGui::End();

	}

	//UIの描画
	ui->DebugGUI();
#endif // USE_IMGUI

}

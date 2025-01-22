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

#include <filesystem>
#include <fstream>
#include <cereal/archives/json.hpp>

Player::Player()
{
	Graphics& graphics = Graphics::Instance();
	//キャラクターモデル
	model = std::make_unique<gltf_model>(graphics.GetDevice().Get(),
		"Resources/Character/Player/glb/white_crow.glb", true);
	
	model->cumulate_transforms(model->nodes, transform);
	for (auto& node : animated_nodes)
	{
		node = model->nodes;
	}
	blended_animated_nodes = model->nodes;

	//skill_manager = std::make_unique<SkillManager>();
	//キャラが持つ剣
	//sword = std::make_unique<Sword>();
	//UI
	//ui = std::make_unique<PlayerUI>();

	beamSaber[LR::LEFT] = model->find_nodes("Left_wep1");
	beamSaber[LR::RIGHT] = model->find_nodes("Right_wep1");
	lowerArm[LR::LEFT] = model->find_nodes("lowerarm_l");
	lowerArm[LR::RIGHT] = model->find_nodes("lowerarm_r");

	mouse = &Device::Instance().GetMouse();
	gamePad = &Device::Instance().GetGamePad();
	camera = &Camera::Instance();

	Initialize();

}

void Player::Initialize()
{
	//パラメーターロード
	LoadDataFile();

	//パラメーター初期化
	position = { 0.0f, 15.0f, 0.0f };
	velocity = { 0.0f, 0.0f, 0.0f };
	//Charactorクラスのパラメーター初期化
	charaParam = param.charaInitParam;

	//体力初期化
	health = charaParam.maxHealth;

	jumpCount = jumpLimit;

	position = { 0.0f,2.0f,0.0f };
	scale.x = scale.y = scale.z = 2.0f;

	charaParam.moveSpeed = 15.0f;

	TransitionIdleState();

	damagedFunction = [=](int damage, float invincible, WINCE_TYPE type)->bool {return ApplyDamage(damage, invincible, type); };

	attackParam.cameraShake.max_X_shake = 3.0f;
	attackParam.cameraShake.max_Y_shake = 7.0f;
	attackParam.cameraShake.time = 0.5f;

	attackParam.hitStop.time = 0.005f;
	attackParam.hitStop.stoppingStrength = 3.0f;
}

Player::~Player()
{
	//delete beamSaber;
	//delete lowerArm;
}

void Player::Update(float elapsedTime)
{
	Graphics& graphics = Graphics::Instance();

	//滞空処理
	BoostUpdate(elapsedTime);
	//更新処理
	(this->*p_update)(elapsedTime);

	if (gamePad->GetButtonDown() & GamePad::BTN_Y)
	{
		camera->SetLockOn();
	}
	if (state == STATE::WING || state == STATE::SHOT) orientation = camera->GetOrientation();

	//プレイヤーの正面情報を更新
	forward = Math::get_posture_forward(orientation);
	
	//無敵時間の更新
	UpdateInvicibleTimer(elapsedTime);

	DebugPrimitiveUpdate();

	collider.start = position;
	collider.end = { position.x,position.y + charaParam.height, position.z };
	collider.radius = 1.0f;
}

void Player::Render_d(float elapsedTime)
{
}

void Player::Render_f(float elapsedTime)
{
	Graphics& graphics = Graphics::Instance();

	//自機モデルのトランスフォーム更新
	transform = Math::calc_world_matrix(scale, orientation, position, Math::COORDINATE_SYSTEM::RHS_YUP);
	if (playerAnimation_transition != playerAnimation)
	{
		if (transition_state != TRANSITION_STATE::NONE)
		{
			playerAnimation_old = playerAnimation_transition;
			animated_nodes[ANIME_NODE::OLD_ANIMATION] = blended_animated_nodes;
			transitionToTransition = true;
		}
		playerAnimation_transition = playerAnimation;
		transition_state = TRANSITION_STATE::START;
	}
	bool isLoop = FindLoopAnimation(playerAnimation);
	//ブレンドアニメーション
	if (transition_state > 0 && transition_time > 0.0f)
	{
		switch (transition_state)
		{
		case TRANSITION_STATE::NONE:
			break;
		case TRANSITION_STATE::START:
			if (!transitionToTransition)
				model->animate(playerAnimation_old, time, animated_nodes[ANIME_NODE::OLD_ANIMATION], FindLoopAnimation(playerAnimation_old));
			model->animate(playerAnimation, 0.0f, animated_nodes[ANIME_NODE::NOW_ANIMATION], isLoop);
			transition_state = TRANSITION_STATE::TRANSITION;
			time = 0.0f;
			factor = 0.0f;

		case TRANSITION_STATE::TRANSITION:
			factor = time / transition_time;
			model->blend_animations(animated_nodes[ANIME_NODE::OLD_ANIMATION], animated_nodes[ANIME_NODE::NOW_ANIMATION], factor, blended_animated_nodes);
			time += elapsedTime;
			if (factor > 1.0f)
			{
				 //End of transition
				transitionToTransition = false;
				transition_state = TRANSITION_STATE::NONE;
				time = 0;
			}
			break;
		}
		model->render(graphics.Get_DC().Get(), transform, blended_animated_nodes);
	}
	else
	{
		time += elapsedTime;
		if (model->animations.at(playerAnimation).duration < time)
		{
			if (isLoop)
				time = 0;
			else time = model->animations.at(playerAnimation).duration;
		}
		model->animate(playerAnimation, time, animated_nodes[ANIME_NODE::NOW_ANIMATION], isLoop);
		model->render(graphics.Get_DC().Get(), transform, animated_nodes[ANIME_NODE::NOW_ANIMATION]);
		playerAnimation_old = playerAnimation;

	}

	//デバッグGUI描画
	DebugGUI();

}

void Player::Render_s(float elapsedTime)
{
}

void Player::RenderUI(float elapsed_time)
{
}

void Player::CalcCollision_vs_Enemy(Capsule capsule_collider, float collider_height)
{
	Collision::CylinderVsCylinder(
		capsule_collider.start, capsule_collider.radius, collider_height,
		position, charaParam.radius, charaParam.height, &position);

}

void Player::CalcAttack_vs_Enemy(Capsule capsule_collider, float collider_height, AddDamageFunc damaged_func)
{
	if (!attackParam.isAttack) return;

	int side = 0;
	if (state == STATE::LEFT_ATTACK) side = LR::LEFT;
	else if (state == STATE::RIGHT_ATTACK) side = LR::RIGHT;

	if (Collision::SphereVsCylinder(attackCollision_position[side], 1.0f,
		capsule_collider.start, capsule_collider.radius,collider_height))
	{
		//攻撃対象に与えるダメージ量と無敵時間
		if (damaged_func(attackParam.power, attackParam.invinsibleTime, WINCE_TYPE::NONE))
		{
			//カメラシェイク
			camera->SetCameraShake(attackParam.cameraShake);

			//ヒットストップ
			camera->SetHitStop(attackParam.hitStop);

			//game_pad->set_vibration(attack_sword_param.hit_viberation.l_moter, attack_sword_param.hit_viberation.r_moter, attack_sword_param.hit_viberation.vibe_time);

			//ヒットエフェクト再生

		}
	}
}

void Player::JudgeSkillCollision(Capsule object_colider, AddDamageFunc damaged_func)
{
}

bool Player::FindLoopAnimation(PlayerAnimation PA)
{
	if (PA == PlayerAnimation::PLAYER_IDLE
		|| PA == PlayerAnimation::PLAYER_MOVE_FORWARD
		|| PA == PlayerAnimation::PLAYER_MOVE_LEFT
		|| PA == PlayerAnimation::PLAYER_MOVE_RIGHT
		|| PA == PlayerAnimation::PLAYER_MOVE_BACK
		|| PA == PlayerAnimation::PLAYER_WING
		|| PA == PlayerAnimation::PLAYER_JUMP
		|| PA == PlayerAnimation::PLAYER_SHOT_IDLE
		|| PA == PlayerAnimation::PLAYER_SHOT_FORWARD
		|| PA == PlayerAnimation::PLAYER_SHOT_LEFT
		|| PA == PlayerAnimation::PLAYER_SHOT_RIGHT
		|| PA == PlayerAnimation::PLAYER_SHOT_BACK
		) return true;
	return false;
}

void Player::Move(float vx, float vz, float speed)
{
	//移動方向ベクトルを設定
	moveVec_x = vx;
	moveVec_z = vz;

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
	if (isGround)
		param.boostTimer += elapsedTime * 3.0f;
	if (isHover)
		param.boostTimer -= elapsedTime;
	if (param.boostTimer >= MAX_BOOST_TIMER)
		param.boostTimer = MAX_BOOST_TIMER;

	if (param.boostTimer < 0 && isHover)
	{
		isHover = false;
		TransitionJumpState();
	}

	if (state != STATE::WING
		&& state != STATE::DAMAGE
		&& state != STATE::DIE
		)
	{
		if (!isGround
			&& gamePad->GetButtonDown() & GamePad::BTN_RIGHT_SHOULDER)
		{
			isHover = !isHover;
		}
	}
}

bool Player::InputMove(float elapsedTime)
{
	//進行ベクトル取得
	const DirectX::XMFLOAT3 move_vec = GetMoveVec(camera);
	//移動処理
	Move(move_vec.x, move_vec.z, charaParam.moveSpeed);
	Turn(elapsedTime, camera->GetForward(), charaParam.turnSpeed, orientation);

	return move_vec.x != 0.0f || move_vec.y != 0.0f || move_vec.z != 0.0f;
}

bool Player::InputMove(float elapsedTime, float restrictionMove, float restrictionTurn)
{
	//進行ベクトル取得
	const DirectX::XMFLOAT3 move_vec = GetMoveVec(camera);

	//移動処理
	Move(move_vec.x, move_vec.z, charaParam.moveSpeed / restrictionMove);
	//Turn(elapsedTime, move_vec, charaParam.turnSpeed / restrictionTurn, orientation);
	Turn(elapsedTime, camera->GetForward(), charaParam.turnSpeed / restrictionTurn, orientation);

	return move_vec.x != 0.0f || move_vec.y != 0.0f || move_vec.z != 0.0f;
}

bool Player::InputMove(float elapsedTime, float move_speed)
{
	//進行ベクトル取得
	const DirectX::XMFLOAT3 move_vec = GetMoveVec(camera);

	//移動処理
	Move(move_vec.x, move_vec.z, move_speed);
	Turn(elapsedTime, move_vec, charaParam.turnSpeed, orientation);

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

	//コントローラーのスティック入力値が一定以下なら入力をはじく
	//if (fabs(ax) > 0.0f && fabs(ax) < 0.5f)  ax += -1.4f * (ax * ax) + 0.5f;
	//if (fabs(ay) > 0.0f && fabs(ay) < 0.5f)  ay += -1.4f * (ay * ay) + 0.5f;
	if (fabs(ax) < 0.3f)  ax = 0.0f;
	if (fabs(ay) < 0.3f)  ay = 0.0f;
	//カメラ右方向ベクトルをXZ単位ベクトルに変換
	float camera_forward_x = camera->GetForward().x;
	float camera_forward_y = wing ? camera->GetForward().y : 0.0f;
	float camera_forward_z = camera->GetForward().z;

	float camera_forward_lengh = sqrtf(camera_forward_x * camera_forward_x + camera_forward_y * camera_forward_y + camera_forward_z * camera_forward_z);
	if (camera_forward_lengh > 0.0f)
	{
		camera_forward_x /= camera_forward_lengh;
		camera_forward_y /= camera_forward_lengh;
		camera_forward_z /= camera_forward_lengh;
	}

	float camera_right_x = camera->GetRight().x;
	float camera_right_y = wing ? camera->GetRight().y : 0.0f;
	float camera_right_z = camera->GetRight().z;
	float camera_right_lengh = sqrtf(camera_right_x * camera_right_x + camera_right_y * camera_right_y + camera_right_z * camera_right_z);

	if (camera_right_lengh > 0.0f)
	{
		camera_right_x /= camera_right_lengh;
		camera_right_y /= camera_right_lengh;
		camera_right_z /= camera_right_lengh;
	}

	DirectX::XMFLOAT3 vec{};
	vec.x = (camera_forward_x * ay) + (camera_right_x * ax);
	vec.y = (camera_forward_y * ay) + (camera_right_y * ax);
	vec.z = (camera_forward_z * ay) + (camera_right_z * ax);
	
	//vec.x = 0.5f;
	//vec.z = 0.5f;
	return vec;
}

void Player::InputJump()
{
	if (gamePad->GetButtonDown() & GamePad::BTN_A 
		|| gamePad->GetButtonDown() & GamePad::BTN_RIGHT_SHOULDER
		) //スペースを押したらジャンプ
	{
		if (jumpCount < jumpLimit)
		{
			{
				TransitionJumpState();
				Jump(param.jumpSpeed);
			}
			isGround = false;//ジャンプしても地面についているというありえない状況を回避するため

			++jumpCount;
		}
	}
}

void Player::InputAvoidance()
{
	if (gamePad->GetButtonDown() & GamePad::BTN_B)
	{
		//TransitionAvoidanceState();
	}

}

void Player::InputWing()
{
	if (gamePad->GetButtonDown() & GamePad::BTN_LEFT_TRIGGER)
	{
		TransitionWingState();
	}
}

void Player::InputShot()
{
	BulletManager& bulletManager = BulletManager::Instance();

	model->fech_by_bone(playerAnimation, time, transform, beamSaber[LR::RIGHT], beamSaber_position[LR::RIGHT]);
	model->fech_by_bone(playerAnimation, time, transform, beamSaber[LR::LEFT], beamSaber_position[LR::LEFT]);

	//前方向
	DirectX::XMFLOAT3 dir = Math::get_posture_forward(transform);
	//発射位置(プレイヤーの腰あたり)
	DirectX::XMFLOAT3 pos;
	if(playerAnimation == PlayerAnimation::PLAYER_SHOT_RIGHT
		|| playerAnimation == PlayerAnimation::PLAYER_SHOT_BACK)
		pos = beamSaber_position[LR::RIGHT];
	else pos = beamSaber_position[LR::LEFT];

	BulletStraight* bullet = 
		new BulletStraight(&BulletManager::Instance(), Bullet::BULLET_MASTER::Player);
	bullet->Launch(dir, pos);

}

void Player::OnLanding()
{
	jumpCount = 0;
	//transition_landing_state();
	if (velocity.y < gravity * 30.0f)// 坂道歩いているときは遷移しない程度に調整
	{
		// 着地ステートへ遷移
		TransitionLandingState();
		velocity = { 0,0,0 };
	}
	else
	{
		TransitionIdleState();
	}

}

void Player::OnDead()
{
	Initialize();

}

void Player::OnDamaged(WINCE_TYPE type)
{
	switch (type)
	{
	case WINCE_TYPE::NONE:
		break;
	case WINCE_TYPE::SMALL:
		
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
	if (isHover)
		velocity.y = 0.0f;
	else if (playerAnimation != PlayerAnimation::PLAYER_WING_START)
		velocity.y += gravity * elapsed_frame;
	else if(playerAnimation == PlayerAnimation::PLAYER_WING_START)
		velocity.y += (gravity * 0.2f) * elapsed_frame;
}

void Player::LoadDataFile()
{
	// Jsonファイルから値を取得
	std::filesystem::path path = filePath;
	path.replace_extension(".json");
	if (std::filesystem::exists(path.c_str()))
	{
		std::ifstream ifs;
		ifs.open(path);
		if (ifs)
		{
			cereal::JSONInputArchive o_archive(ifs);
			o_archive(param);
		}
	}
}

void Player::SaveDataFile()
{
	//ベースクラスの初期化パラメーター情報を更新
	param.charaInitParam = charaParam;
	// Jsonファイルから値を取得
	std::filesystem::path path = filePath;
	path.replace_extension(".json");
	std::ofstream ifs;
	ifs.open(path);
	if (ifs)
	{
		cereal::JSONOutputArchive o_archive(ifs);
		o_archive(param);
	}

}

void Player::DebugPrimitiveUpdate()
{
	DebugRenderer* debugRender = Graphics::Instance().GetDebugRenderer();
	
	//サーベルの当たり判定
	{
		model->fech_by_bone(playerAnimation, time, transform, beamSaber[LR::LEFT], beamSaber_position[LR::LEFT]);
		model->fech_by_bone(playerAnimation, time, transform, lowerArm[LR::LEFT], lowerArm_position[LR::LEFT]);
		model->fech_by_bone(playerAnimation, time, transform, beamSaber[LR::RIGHT], beamSaber_position[LR::RIGHT]);
		model->fech_by_bone(playerAnimation, time, transform, lowerArm[LR::RIGHT], lowerArm_position[LR::RIGHT]);

		std::function<DirectX::XMFLOAT3(DirectX::XMFLOAT3&, DirectX::XMFLOAT3&)> saber_position{
			[](DirectX::XMFLOAT3& arm, DirectX::XMFLOAT3& saber)->DirectX::XMFLOAT3 {

				DirectX::XMFLOAT3 Arm{ arm };
				DirectX::XMFLOAT3 Saber{ saber };

				DirectX::XMFLOAT3 direction = Math::calc_vector_AtoB_normalize(Arm, Saber);
				float length = Math::calc_vector_AtoB_length(Arm, Saber);

				return Math::calc_designated_point(Arm, direction, length * 0.5f);
		} };
		attackCollision_position[LR::LEFT] = saber_position(lowerArm_position[LR::LEFT], beamSaber_position[LR::LEFT]);
		attackCollision_position[LR::RIGHT] = saber_position(lowerArm_position[LR::RIGHT], beamSaber_position[LR::RIGHT]);

		//debugRender->CreateSphere(
		//	attackCollision_position[LR::LEFT],
		//	1.0f, { 1.0f,0.0f,0.0f,1.0f });
		//debugRender->CreateSphere(
		//	attackCollision_position[LR::RIGHT],
		//	1.0f, { 1.0f,0.0f,0.0f,1.0f });
	}

	//自分の当たり判定
	debugRender->CreateCylinder(collider.start,
		collider.radius,
		charaParam.height,
		{ 0.0f,1.0f,0.0f,1.0f });
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
			}
			if (ImGui::CollapsingHeader("Param", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::DragFloat("height", &charaParam.height);
				ImGui::DragInt("max_health", &charaParam.maxHealth);
				ImGui::DragInt("health", &health);
				ImGui::DragFloat("radius", &charaParam.radius);
				ImGui::DragFloat("gravity", &gravity);
				ImGui::DragFloat("floating_value", &param.floatingValue);
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
			//if (ImGui::CollapsingHeader("AttackCameraShake", ImGuiTreeNodeFlags_DefaultOpen))
			//{
			//	if (ImGui::Button("set_vibration"))
			//	{
			//		gamePad->SetVibration(1, 1, 0.5);
			//
			//	}
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
				ImGui::DragFloat("combo1_stopping_strength", &param.combo_1.hitStop.stoppingStrength, 0.1f);
				//ImGui::DragFloat("combo1_hit_viberation.l_moter", &param.combo_1.hitViberation.L_moter, 0.1f);
				//ImGui::DragFloat("combo1_hit_viberation.r_moter", &param.combo_1.hitViberation.R_moter, 0.1f);
				//ImGui::DragFloat("combo1_vibe_time", &param.combo_1.hitViberation.VibeTime, 0.1f);
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
				ImGui::DragFloat("combo2_stopping_strengthy", &param.combo_2.hitStop.stoppingStrength, 0.1f);
				//ImGui::DragFloat("combo2_hit_viberation.l_moter", &param.combo_2.hitViberation.L_moter, 0.1f);
				//ImGui::DragFloat("combo2_hit_viberation.r_moter", &param.combo_2.hitViberation.R_moter, 0.1f);
				//ImGui::DragFloat("combo2_vibe_time", &param.combo_2.hitViberation.VibeTime, 0.1f);
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
				ImGui::DragFloat("combo3_stopping_strength", &param.combo_3.hitStop.stoppingStrength, 0.1f);
				//ImGui::DragFloat("combo3_hit_viberation.l_moter", &param.combo_3.hitViberation.L_moter, 0.1f);
				//ImGui::DragFloat("combo3_hit_viberation.r_moter", &param.combo_3.hitViberation.R_moter, 0.1f);
				//ImGui::DragFloat("combo3_vibe_time", &param.combo_3.hitViberation.VibeTime, 0.1f);
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

#endif // USE_IMGUI

}

#include "player.h"
#include"camera.h"
#include "shader.h"
#include"user.h"
#include "texture.h"
#include "operators.h"
#include "collision.h"
#include "Graphics.h"
#include "magic_enum/include/magic_enum.hpp"

/*
#include <filesystem>
#include <fstream>
*/


Player::Player()
{
	Graphics& graphics = Graphics::Instance();
	//キャラクターモデル
	model = std::make_unique<gltf_model>(graphics.GetDevice().Get(),
		//"Resources/Player/fbxGLTF/white_crow.gltf");
		//"Resources/Player/glb/white_crow.glb");
		"Resources/Player/glb/white_crow_test.glb");
		//"Resources/Player/white_crow.gltf");

	//skill_manager = std::make_unique<SkillManager>();
	//キャラが持つ剣
	//sword = std::make_unique<Sword>();
	//UI
	//ui = std::make_unique<PlayerUI>();

	mouse = &Device::Instance().GetMouse();
	gamePad = &Device::Instance().GetGamePad();
	camera = &Camera::Instance();

	Initialize();

}

void Player::Initialize()
{
	//パラメーター初期化
	position = { 0.0f, 0.0f, 0.0f };
	velocity = { 0.0f, 0.0f, 0.0f };
	//Charactorクラスのパラメーター初期化
	charaParam = param.charaInitParam;

	//体力初期化
	health = charaParam.maxHealth;

	jump_count = jump_limit;

	position = { 0.0f,2.0f,0.0f };
	scale.x = scale.y = scale.z = 1.0f;

	charaParam.moveSpeed = 5.0f;

	playerAnimation = PlayerAnimation::PLAYER_IDLE;
	state = State::IDLE;

	damagedFunction = [=](int damage, float invincible, WINCE_TYPE type)->bool {return ApplyDamage(damage, invincible, type); };


}

Player::~Player()
{
}

void Player::Update(float elapsedTime)
{
	Graphics& graphics = Graphics::Instance();
	//更新処理
	(this->*p_update)(elapsedTime);

	if (gamePad->GetButtonDown() & GamePad::BTN_Y)
	{
		camera->SetLockOn();
	}

	if (isFallDawn)
	{
		velocity.y = 0.0f;
	}

	//プレイヤーの正面情報を更新
	forward = Math::get_posture_forward(orientation);

	//無敵時間の更新
	UpdateInvicibleTimer(elapsedTime);

	anime_time += elapsedTime;
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
	static std::vector<gltf_model::node> animated_nodes{ model->nodes };
	static float time{ 0 };
	//ブレンドアニメーションはゲープロか福井先生のやつ
	model->animate(playerAnimation, time += elapsedTime, animated_nodes, true);
	model->render(graphics.Get_DC().Get(), transform, animated_nodes);

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
	Collision::CylinderVsCylinder(collider.start, collider.radius, collider_height, position, charaParam.radius, charaParam.height, &position);

}

void Player::CalcAttack_vs_Enemy(Capsule collider, AddDamageFunc damaged_func)
{

}

void Player::JudgeSkillCollision(Capsule object_colider, AddDamageFunc damaged_func)
{
}

bool Player::InputMove(float elapsedTime)
{
	//進行ベクトル取得
	const DirectX::XMFLOAT3 move_vec = GetMoveVec(camera);

	//移動処理
	Move(move_vec.x, move_vec.z, charaParam.moveSpeed);
	Turn(elapsedTime, move_vec, charaParam.turnSpeed, orientation);

	return move_vec.x != 0.0f || move_vec.y != 0.0f || move_vec.z != 0.0f;
}

bool Player::InputMove(float elapsedTime, float restrictionMove, float restrictionTurn)
{
	//進行ベクトル取得
	const DirectX::XMFLOAT3 move_vec = GetMoveVec(camera);

	//移動処理
	Move(move_vec.x, move_vec.z, charaParam.moveSpeed / restrictionMove);
	Turn(elapsedTime, move_vec, charaParam.turnSpeed / restrictionTurn, orientation);

	return move_vec.x != 0.0f || move_vec.y != 0.0f || move_vec.z != 0.0f;
}

const DirectX::XMFLOAT3 Player::GetMoveVec(Camera* camera) const
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
	float camera_forward_z = camera->GetForward().z;
	float camera_forward_lengh = sqrtf(camera_forward_x * camera_forward_x + camera_forward_z * camera_forward_z);
	if (camera_forward_lengh > 0.0f)
	{
		camera_forward_x /= camera_forward_lengh;
		camera_forward_z /= camera_forward_lengh;
	}

	float camera_right_x = camera->GetRight().x;
	float camera_right_z = camera->GetRight().z;
	float camera_right_lengh = sqrtf(camera_right_x * camera_right_x + camera_right_z * camera_right_z);

	if (camera_right_lengh > 0.0f)
	{
		camera_right_x /= camera_right_lengh;
		camera_right_z /= camera_right_lengh;
	}

	DirectX::XMFLOAT3 vec{};
	vec.x = (camera_forward_x * ay) + (camera_right_x * ax);
	vec.z = (camera_forward_z * ay) + (camera_right_z * ax);

	return vec;
}

void Player::InputJump()
{
	if (gamePad->GetButtonDown() & GamePad::BTN_A
		) //スペースを押したらジャンプ
	{
		if (jump_count < jump_limit)
		{
			TransitionJumpState();
			Jump(param.jumpSpeed);
			isGround = false;//ジャンプしても地面についているというありえない状況を回避するため

			++jump_count;
		}
	}

}

void Player::InputAvoidance()
{
	if (gamePad->GetButtonDown() & GamePad::BTN_B)
	{
		TransitionAvoidanceState();
	}

}

void Player::OnLanding()
{
	jump_count = 0;
	//transition_landing_state();
	if (velocity.y < gravity * 30.0f)// 坂道歩いているときは遷移しない程度に調整
	{
		// 着地ステートへ遷移
		TransitionIdleState();
		velocity = { 0,0,0 };
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
				state_name = magic_enum::enum_name<State>(state);
				ImGui::Text(state_name.c_str());
				ImGui::DragFloat3("velocity:", &velocity.x);
				ImGui::Checkbox("isFallDawn:", &isFallDawn);
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
				ImGui::DragFloat("TurnSpeed", &charaParam.turnSpeed, 0.1f);
				ImGui::DragFloat("MoveSpeed", &charaParam.moveSpeed, 0.1f);
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
			//	if (ImGui::Button("load"))
			//	{
			//		load_data_file();
			//	}
			//	ImGui::Separator();
			//	if (ImGui::Button("save"))
			//	{
			//		save_data_file();
			//	}
			//	ImGui::Text("attack_pparam");
			//	if (ImGui::CollapsingHeader("combo1"))
			//	{
			//		ImGui::DragInt("combo1_power", &param.combo_1.power, 0.1f);
			//		ImGui::DragFloat("combo1_invinsible_time", &param.combo_1.invinsible_time, 0.1f);
			//
			//		ImGui::Text("combo1_camera_shake");
			//		ImGui::DragFloat("combo1_shake_x", &param.combo_1.camera_shake.max_x_shake, 0.1f);
			//		ImGui::DragFloat("combo1_shake_y", &param.combo_1.camera_shake.max_y_shake, 0.1f);
			//		ImGui::DragFloat("combo1_time", &param.combo_1.camera_shake.time, 0.1f);
			//		ImGui::DragFloat("combo1_smmoth", &param.combo_1.camera_shake.shake_smoothness, 0.1f, 0.1f, 1.0f);
			//
			//		ImGui::Text("hit_stop");
			//		ImGui::DragFloat("combo1_stop_time", &param.combo_1.hit_stop.time, 0.1f);
			//		ImGui::DragFloat("combo1_stopping_strength", &param.combo_1.hit_stop.stopping_strength, 0.1f);
			//		ImGui::DragFloat("combo1_hit_viberation.l_moter", &param.combo_1.hit_viberation.l_moter, 0.1f);
			//		ImGui::DragFloat("combo1_hit_viberation.r_moter", &param.combo_1.hit_viberation.r_moter, 0.1f);
			//		ImGui::DragFloat("combo1_vibe_time", &param.combo_1.hit_viberation.vibe_time, 0.1f);
			//	}
			//	if (ImGui::CollapsingHeader("combo2"))
			//	{
			//		ImGui::DragInt("combo2_power", &param.combo_2.power, 0.1f);
			//		ImGui::DragFloat("combo2_invinsible_time", &param.combo_2.invinsible_time, 0.1f);
			//
			//		ImGui::Text("combo2_camera_shake");
			//		ImGui::DragFloat("combo2_shake_x", &param.combo_2.camera_shake.max_x_shake, 0.1f);
			//		ImGui::DragFloat("combo2_shake_y", &param.combo_2.camera_shake.max_y_shake, 0.1f);
			//		ImGui::DragFloat("combo2_time", &param.combo_2.camera_shake.time, 0.1f);
			//		ImGui::DragFloat("combo2_smmoth", &param.combo_2.camera_shake.shake_smoothness, 0.1f, 0.1f, 1.0f);
			//		ImGui::Text("hit_stop");
			//		ImGui::DragFloat("combo2_stop_time", &param.combo_2.hit_stop.time, 0.1f);
			//		ImGui::DragFloat("combo2_stopping_strengthy", &param.combo_2.hit_stop.stopping_strength, 0.1f);
			//		ImGui::DragFloat("combo2_hit_viberation.l_moter", &param.combo_2.hit_viberation.l_moter, 0.1f);
			//		ImGui::DragFloat("combo2_hit_viberation.r_moter", &param.combo_2.hit_viberation.r_moter, 0.1f);
			//		ImGui::DragFloat("combo2_vibe_time", &param.combo_2.hit_viberation.vibe_time, 0.1f);
			//	}
			//
			//	if (ImGui::CollapsingHeader("combo3"))
			//	{
			//		ImGui::DragInt("combo3_power", &param.combo_3.power, 0.1f);
			//		ImGui::DragFloat("combo3_invinsible_time", &param.combo_3.invinsible_time, 0.1f);
			//
			//		ImGui::Text("combo3_camera_shake");
			//		ImGui::DragFloat("combo3_shake_x", &param.combo_3.camera_shake.max_x_shake, 0.1f);
			//		ImGui::DragFloat("combo3_shake_y", &param.combo_3.camera_shake.max_y_shake, 0.1f);
			//		ImGui::DragFloat("combo3_time", &param.combo_3.camera_shake.time, 0.1f);
			//		ImGui::DragFloat("combo3_smmoth", &param.combo_3.camera_shake.shake_smoothness, 0.1f, 0.1f, 1.0f);
			//		ImGui::Text("hit_stop");
			//		ImGui::DragFloat("combo3_stop_time", &param.combo_3.hit_stop.time, 0.1f);
			//		ImGui::DragFloat("combo3_stopping_strength", &param.combo_3.hit_stop.stopping_strength, 0.1f);
			//		ImGui::DragFloat("combo3_hit_viberation.l_moter", &param.combo_3.hit_viberation.l_moter, 0.1f);
			//		ImGui::DragFloat("combo3_hit_viberation.r_moter", &param.combo_3.hit_viberation.r_moter, 0.1f);
			//		ImGui::DragFloat("combo3_vibe_time", &param.combo_3.hit_viberation.vibe_time, 0.1f);


			//	}
			//}
			if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen))
			{
				const char* anime_item[] = { "IDLE","MOVE","JUMP","FALL","LANDING" };
				static int item_current = 0;
				static bool loop = false;
				ImGui::Combo("anime", &item_current, anime_item, IM_ARRAYSIZE(anime_item)); ImGui::Checkbox("is_loop", &loop);
				if (ImGui::Button("play", { 80,20 }))
				{
					//model->animate(item_current, loop, 0.1f);
				}
				//string s;
				//ImGui::DragInt("index", &model->anime_param.current_index);
				//ImGui::DragInt("frame_index", &model->anime_param.frame_index);
				//ImGui::Checkbox("end_flag", &model->anime_param.end_flag);
				//ImGui::DragFloat("current_time", &model->anime_param.current_time);
				//ImGui::DragFloat("playback_speed", &model->anime_param.playback_speed, 0.1f);
				//ImGui::DragFloat("blend_time", &model->anime_param.blend_time);
				//ImGui::DragFloat("sampling", &model->anime_param.animation.sampling_rate);
			}

		}
		ImGui::End();

	}

#endif // USE_IMGUI

}

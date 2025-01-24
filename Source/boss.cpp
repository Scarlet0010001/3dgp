#include "boss.h"
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

#include <stack>

Boss::Boss()
{
	Graphics& graphics = Graphics::Instance();
	//キャラクターモデル
	model = std::make_unique<gltf_model>(graphics.GetDevice().Get(),
		"Resources/Character/Boss/RobotDog_main.glb", false);
	
	model->cumulate_transforms(model->nodes, transform);
	for (auto& node : animated_nodes)
	{
		node = model->nodes;
	}
	blended_animated_nodes = model->nodes;
	//lookAt_nodes = model->nodes;

	//turretHeadNode = model->find_nodes("Bone_Turret_Head_Main");
	//turretNode = model->find_nodes("Bone_MGun_Main");

	// 初期姿勢時の頭ノードのローカル空間前方向を求める
	{
		DirectX::XMFLOAT4X4 worldTransform = model->nodes.at(42).world_transform;
		DirectX::XMMATRIX WorldTransform =
			DirectX::XMLoadFloat4x4(&worldTransform);
		DirectX::XMMATRIX InverseWorldTransform =
			DirectX::XMMatrixInverse(nullptr, WorldTransform);
		DirectX::XMVECTOR HeadWorldForward = DirectX::XMLoadFloat3(&Math::get_posture_forward(worldTransform));
		HeadWorldForward = DirectX::XMVector3Normalize(HeadWorldForward);
		//DirectX::XMVECTOR HeadLocalForward =
		//	DirectX::XMVector3TransformNormal(HeadWorldForward, InverseWorldTransform);
		//HeadLocalForward = DirectX::XMVector3Normalize(HeadLocalForward);

		//DirectX::XMStoreFloat3(&turretLocalForward, HeadLocalForward);
		DirectX::XMStoreFloat3(&turretWorldForward, HeadWorldForward);
	}

	//skill_manager = std::make_unique<SkillManager>();
	//UI 
	//ui = std::make_unique<PlayerUI>();

	//arm = model->find_nodes("lowerarm_l");

	Initialize();

}

void Boss::Initialize()
{
	//パラメーター初期化
	position = { 0.0f, 15.0f, 10.0f };
	velocity = { 0.0f, 0.0f, 0.0f };
	scale.x = scale.y = scale.z = 10.0f;
	//Charactorクラスのパラメーター初期化
	charaParam = param.chara_init_param;
	charaParam.maxHealth = 2000.0f;
	TransitionIdleState();

	//体力初期化
	health = charaParam.maxHealth;

	stepOffset = 2.0f;

	tackleCameraShake.max_X_shake = 8.0f;
	tackleCameraShake.max_Y_shake = 12.0f;

	charaParam.moveSpeed = WALK_SPEED;
	state_duration = 2.0f;
	param.run_speed = RUN_SPEED;
	bossBodyCollision.capsule.start = position;
	bossBodyCollision.capsule.radius = 5;
	bossBodyCollision.height = 10;
	bossBodyCollision.capsule.end = bossBodyCollision.capsule.start;
	bossBodyCollision.capsule.end.y = bossBodyCollision.capsule.start.y + bossBodyCollision.height;
	bossBodyCollision.attackRadius = 5.2f;
	bossBodyCollision.attackHeight = 10;
	
	damagedFunction = [=](int damage, float invincible, WINCE_TYPE type)->bool {return ApplyDamage(damage, invincible, type); };
}

void Boss::Update(float elapsedTime)
{
#if _DEBUG
	if (!isUpdate) return;
#endif

	(this->*act_update)(elapsedTime);

	//{//テスト
	//	DirectX::XMFLOAT4X4 worldTransform = animated_nodes[ANIME_NODE::NOW_ANIMATION].at(42).world_transform;
	//	DirectX::XMMATRIX WorldTransform =
	//		DirectX::XMLoadFloat4x4(&worldTransform);
	//	DirectX::XMMATRIX InverseWorldTransform =
	//		DirectX::XMMatrixInverse(nullptr, WorldTransform);
	//	DirectX::XMVECTOR HeadWorldForward = DirectX::XMLoadFloat3(&Math::get_posture_forward(worldTransform));
	//	HeadWorldForward = DirectX::XMVector3Normalize(HeadWorldForward);
	//	//DirectX::XMVECTOR HeadLocalForward =
	//	//	DirectX::XMVector3TransformNormal(HeadWorldForward, InverseWorldTransform);
	//	//HeadLocalForward = DirectX::XMVector3Normalize(HeadLocalForward);
	//	
	//	//DirectX::XMStoreFloat3(&turretLocalForward, HeadLocalForward);
	//	DirectX::XMStoreFloat3(&turretWorldForward, HeadWorldForward);
	//}


	UpdateInvicibleTimer(elapsedTime);

	bossBodyCollision.capsule.start = position;
	bossBodyCollision.capsule.end = bossBodyCollision.capsule.start;
	bossBodyCollision.capsule.end.y = bossBodyCollision.capsule.start.y + bossBodyCollision.height;

	stateTimer += elapsedTime;

	DebugPrimitiveUpdate();
}

void Boss::Render_f(float elapsedTime)
{
	Graphics& graphics = Graphics::Instance();

	//ボスモデルのトランスフォーム更新
	transform = Math::calc_world_matrix(scale, orientation, position, Math::COORDINATE_SYSTEM::RHS_YUP);
	if (bossAnimation_transition != bossAnimation)
	{
		if (transition_state != TRANSITION_STATE::NONE)
		{
			bossAnimation_old = bossAnimation_transition;
			animated_nodes[ANIME_NODE::OLD_ANIMATION] = blended_animated_nodes;
			transitionToTransition = true;
		}
		bossAnimation_transition = bossAnimation;
		transition_state = TRANSITION_STATE::START;
	}

	bool nowLoop = FindLoopAnimation(bossAnimation);
	//ブレンドアニメーション
	if (transition_state > 0 && transition_time > 0.0f)
	{
		switch (transition_state)
		{
		case TRANSITION_STATE::NONE:
			break;
		case TRANSITION_STATE::START:
			model->animate(bossAnimation_old, time, animated_nodes[ANIME_NODE::OLD_ANIMATION], FindLoopAnimation(bossAnimation_old));
			model->animate(bossAnimation, 0.0f, animated_nodes[ANIME_NODE::NOW_ANIMATION], nowLoop);
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
				transition_state = TRANSITION_STATE::NONE;
				time = 0;
			}
			break;
		}
		//lookAt_nodes = &LookAt_turret(blended_animated_nodes);
		model->render(graphics.Get_DC().Get(), transform, blended_animated_nodes);
		//model->render(graphics.Get_DC().Get(), transform, lookAt_nodes);
	}
	else
	{
		time += elapsedTime;
		if (model->animations.at(bossAnimation).duration < time)
		{
			if (nowLoop)
				time = 0;
			else time = model->animations.at(bossAnimation).duration;
		}
		model->animate(bossAnimation, time, animated_nodes[ANIME_NODE::NOW_ANIMATION], nowLoop);
		lookAt_nodes = animated_nodes[ANIME_NODE::NOW_ANIMATION];
		//LookAt_turret(lookAt_nodes);
		model->render(graphics.Get_DC().Get(), transform, animated_nodes[ANIME_NODE::NOW_ANIMATION]);
		//model->render(graphics.Get_DC().Get(), transform, lookAt_nodes);
		bossAnimation_old = bossAnimation;
	}

}

void Boss::Render_ui(float elapsedTime)
{
}

void Boss::ShotBullet(ATTACK_TYPE type)
{
	BulletManager& bulletManager = BulletManager::Instance();

	//model->fech_by_bone(bossAnimation, time, transform, beamSaber[LR::RIGHT], beamSaber_position[LR::RIGHT]);
	//model->fech_by_bone(bossAnimation, time, transform, beamSaber[LR::LEFT], beamSaber_position[LR::LEFT]);

	//前方向
	DirectX::XMFLOAT3 dir = Math::get_posture_forward(transform);
	//発射位置(プレイヤーの腰あたり)
	DirectX::XMFLOAT3 pos = { position.x,position.y + charaParam.height,position.z };
	//if (bossAnimation == BossAnimation::PLAYER_SHOT_RIGHT
	//	|| bossAnimation == BossAnimation::PLAYER_SHOT_BACK)
	//	pos = beamSaber_position[LR::RIGHT];
	//else pos = beamSaber_position[LR::LEFT];

	BulletStraight* bullet =
		type == ATTACK_TYPE::SHOT_S ? new BulletStraight(&BulletManager::Instance(), Bullet::BULLET_MASTER::Enemy)
		: new BulletStraight(&BulletManager::Instance(), Bullet::BULLET_MASTER::Enemy);
	bullet->Launch(dir, pos);

}
void Boss::LookAt_turret(std::vector<gltf_model::node>& nodes)
{
	//gltf_model::node& turretBaseNode = model->find_nodes("Bone_Turret_Head_Main");
	//gltf_model::node& turretNode = model->find_nodes("Bone_MGun_Main");
	gltf_model::node& turretBaseNode = nodes.at(42);
	gltf_model::node& turretNode = nodes.at(43);

	// ワールド行列更新処理関数
	std::stack<DirectX::XMFLOAT4X4> parent_global_transforms;
	std::function<void(gltf_model::node&)> updateWorldTransforms = [&](gltf_model::node& node)
	{	
		//node& node{ nodes.at(node_index) };
		DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(node.scale.x,node.scale.y,node.scale.z) };
		DirectX::XMMATRIX R{ DirectX::XMMatrixRotationQuaternion(
			DirectX::XMVectorSet(node.rotation.x,node.rotation.y,node.rotation.z,node.rotation.w)) };
		DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(node.translation.x,node.translation.y,node.translation.z) };
		DirectX::XMStoreFloat4x4(&node.global_transform, S * R * T * DirectX::XMLoadFloat4x4(&parent_global_transforms.top()));

		for (auto child : node.children)
		{
			parent_global_transforms.push(node.global_transform);
			updateWorldTransforms(nodes.at(child));
			parent_global_transforms.pop();

		}
	};
	
	//横回転から計算
	{
		DirectX::XMMATRIX HeadWorldTransform =
			DirectX::XMLoadFloat4x4(&turretBaseNode.world_transform);
		DirectX::XMMATRIX InverseWorldTransform =
			DirectX::XMMatrixInverse(nullptr, HeadWorldTransform);

		DirectX::XMVECTOR TargetVec = DirectX::XMLoadFloat3(&target_pos);//ワールドで計算
		//DirectX::XMVECTOR a = DirectX::XMVector3TransformCoord(TargetVec, InverseWorldTransform);
		//TargetVec = DirectX::XMVectorSubtract(a, DirectX::XMLoadFloat3(&turretBaseNode.translation));
		DirectX::XMVECTOR WorldTranslation = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&turretBaseNode.translation), HeadWorldTransform);
		//ワールドのTargetVec
		TargetVec = DirectX::XMVectorSubtract(TargetVec, WorldTranslation);

		DirectX::XMVECTOR TargetVecNormal{};
		TargetVecNormal = DirectX::XMVector3Normalize(TargetVec);

		DirectX::XMFLOAT3 targetVec{};
		DirectX::XMStoreFloat3(&targetVec, TargetVecNormal);

		//ワールドに変換と更新
		DirectX::XMVECTOR HeadLocalForward = DirectX::XMLoadFloat3(&turretWorldForward);

		//DirectX::XMVECTOR Axis = DirectX::XMVector3Cross(HeadLocalForward, TargetVecNormal);
		DirectX::XMVECTOR Axis = DirectX::XMVectorSet(0.0f,1.0f,0.0f,1.0f);

		float dotProduct = DirectX::XMVectorGetX(
			DirectX::XMVector3Dot(HeadLocalForward, TargetVecNormal));

		float angle = 0.5f;// DirectX::XMConvertToRadians(acosf(dotProduct));

		DirectX::XMVECTOR RotationMatrix = DirectX::XMQuaternionRotationAxis(Axis, angle);

		RotationMatrix = DirectX::XMVector3TransformCoord(RotationMatrix, InverseWorldTransform);

		DirectX::XMVECTOR HR =
			DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&turretBaseNode.rotation), RotationMatrix);
		DirectX::XMStoreFloat4(&turretBaseNode.rotation, HR);
		//turretBaseNode.rotation.y = DirectX::XMConvertToRadians(170.0f);
		//turretBaseNode.rotation.x = DirectX::XMConvertToRadians(170.0f);

		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(turretBaseNode.scale.x, turretBaseNode.scale.y, turretBaseNode.scale.z);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMQuaternionNormalize(DirectX::XMLoadFloat4(&turretBaseNode.rotation)));
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(turretBaseNode.translation.x, turretBaseNode.translation.y, turretBaseNode.translation.z);

		DirectX::XMMATRIX LocalTransform = S * R * T;

		//gltf_model::node& parent_turretBaseNode = model->find_nodes("Bone_Turret_Base_Main");
		gltf_model::node& parent_turretBaseNode = nodes.at(41);
		
		DirectX::XMMATRIX ParentGlobalTransform =
			DirectX::XMLoadFloat4x4(&parent_turretBaseNode.global_transform);

		DirectX::XMMATRIX GlobalTransform = LocalTransform * ParentGlobalTransform;
		DirectX::XMMATRIX WorldTransform = GlobalTransform * DirectX::XMLoadFloat4x4(&transform);
		DirectX::XMStoreFloat4x4(&turretBaseNode.local_transform, LocalTransform);
		DirectX::XMStoreFloat4x4(&turretBaseNode.global_transform, GlobalTransform);
		DirectX::XMStoreFloat4x4(&turretBaseNode.world_transform, WorldTransform);
		//for (auto child : turretBaseNode.children)
		//{
		//	parent_global_transforms.push(parent_turretBaseNode.global_transform);
		//	updateWorldTransforms(nodes.at(child));
		//	parent_global_transforms.pop();
		//}
		//model->nodes.at(42);
		model->cumulate_transforms(nodes, transform);
	}
}

void Boss::CalcAttack_vs_Player(Capsule capsule_collider, float collider_height, AddDamageFunc damaged_func)
{
	if (!attackParam.isAttack)
		return;

	Camera &camera = Camera::Instance();

	if (Collision::CylinderVsCylinder(
		bossBodyCollision.capsule.start, bossBodyCollision.attackRadius, bossBodyCollision.attackHeight,
		capsule_collider.start, capsule_collider.radius, collider_height))
	{
		//攻撃対象に与えるダメージ量と無敵時間
		if (damaged_func(attackParam.power, attackParam.invinsibleTime, WINCE_TYPE::NONE))
		{
			//カメラシェイク
			camera.SetCameraShake(attackParam.cameraShake);

			//ヒットストップ
			camera.SetHitStop(attackParam.hitStop);
		}
	}

}

float Boss::CalcMoveSpeed(DirectX::XMFLOAT3 target, float time)
{
	float direction = Math::calc_vector_AtoB_length(position, target);
	return direction / time;
}

void Boss::OnDead()
{
}

void Boss::OnDamaged(WINCE_TYPE type)
{
}

void Boss::LoadDataFile()
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

void Boss::SaveDataFile()
{
	//ベースクラスの初期化パラメーター情報を更新
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

bool Boss::FindLoopAnimation(BossAnimation BA)
{
	if (BA == BossAnimation::BOSS_IDLE
		|| BA == BossAnimation::BOSS_WALK
		|| BA == BossAnimation::BOSS_RUN
		) return true;
	return false;
}

void Boss::DebugDUI()
{
#if USE_IMGUI
	imguiMenuBar("Character", "boss", displayImgui);
	if (displayImgui)
	{
		if (ImGui::Begin("Boss", nullptr, ImGuiWindowFlags_None))
		{
#if _DEBUG
			ImGui::Checkbox("is_update", &isUpdate);
			ImGui::Separator();
			ImGui::Checkbox("is_render", &isRender);
#endif
			//if (ImGui::CollapsingHeader("Skill", ImGuiTreeNodeFlags_DefaultOpen))
			//{
			//	if (ImGui::Button("skill_1"))
			//	{
			//		transition_skill_1_state();
			//	}
			//	if (ImGui::Button("skill_2"))
			//	{
			//		transition_skill_2_start_state();
			//	}
			//	if (ImGui::Button("skill_3"))
			//	{
			//		transition_skill_3_state();
			//	}
			//}
			//if (ImGui::Button("load"))
			//{
			//	LoadDataFile();
			//}
			//ImGui::Separator();
			//if (ImGui::Button("save"))
			//{
			//	SaveDataFile();
			//}
			//トランスフォーム
			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
			{
				//位置
				ImGui::DragFloat3("Position", &position.x);
				//回転				
				ImGui::DragFloat3("scale:", &scale.x);
				//速度
				ImGui::DragFloat3("velocity:", &velocity.x);
			}
			//トランスフォーム
			//if (ImGui::CollapsingHeader("Turret_Head_Transform", ImGuiTreeNodeFlags_DefaultOpen))
			//{
			//	//回転
			//	ImGui::DragFloat4("Rotation", &lookAt_nodes.at(42).rotation.x);
			//	//回転				
			//	ImGui::DragFloat4("global_transform0:", &lookAt_nodes.at(42).global_transform._11);
			//	ImGui::DragFloat4("global_transform1:", &lookAt_nodes.at(42).global_transform._21);
			//	ImGui::DragFloat4("global_transform2:", &lookAt_nodes.at(42).global_transform._31);
			//	ImGui::DragFloat4("global_transform3:", &lookAt_nodes.at(42).global_transform._41);
			//	//回転
			//	ImGui::DragFloat4("Rotation", &model->nodes.at(42).rotation.x);
			//	//回転				
			//	ImGui::DragFloat4("global_transform0:", &model->nodes.at(42).global_transform._11);
			//	ImGui::DragFloat4("global_transform1:", &model->nodes.at(42).global_transform._21);
			//	ImGui::DragFloat4("global_transform2:", &model->nodes.at(42).global_transform._31);
			//	ImGui::DragFloat4("global_transform3:", &model->nodes.at(42).global_transform._41);
			//}
			if (ImGui::CollapsingHeader("Param", ImGuiTreeNodeFlags_DefaultOpen))
			{
				std::string state_name;
				state_name = magic_enum::enum_name<STATE>(state);
				ImGui::Text(state_name.c_str());

				ImGui::DragInt("max_health", &charaParam.maxHealth);
				ImGui::DragInt("hp", &health);
				ImGui::DragFloat("height", &charaParam.height);
				ImGui::DragFloat("radius", &charaParam.radius);
				ImGui::DragFloat("moveSpeed", &charaParam.moveSpeed, 0.1f);
				ImGui::DragFloat("turnspeed", &charaParam.turnSpeed, 0.1f);
				ImGui::DragFloat("invinsible_timer", &invincibleTimer);
				ImGui::DragFloat("friction", &charaParam.friction);
				ImGui::DragFloat("acceleration", &charaParam.acceleration);
				ImGui::Checkbox("is_ground", &isGround);

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
				if (ImGui::CollapsingHeader("tackle"))
				{
					ImGui::DragInt("tackle_power", &param.tackleParam.power, 0.1f);
					ImGui::DragFloat("tackle_invinsible_time", &param.tackleParam.invinsibleTime, 0.1f);

					ImGui::Text("tackle_camera_shake");
					ImGui::DragFloat("tackle_shake_x", &param.tackleParam.cameraShake.max_X_shake, 0.1f);
					ImGui::DragFloat("tackle_shake_y", &param.tackleParam.cameraShake.max_Y_shake, 0.1f);
					ImGui::DragFloat("tackle_time", &param.tackleParam.cameraShake.time, 0.1f);
					ImGui::DragFloat("tackle_smmoth", &param.tackleParam.cameraShake.shakeSmoothness, 0.1f, 0.1f, 1.0f);

					ImGui::Text("hit_stop");
					ImGui::DragFloat("tackle_stop_time", &param.tackleParam.hitStop.time, 0.1f);
					ImGui::DragFloat("tackle_stopping_strength", &param.tackleParam.hitStop.stoppingStrength, 0.1f);
					//ImGui::DragFloat("combo1_hit_viberation.l_moter", &param.combo_1.hitViberation.L_moter, 0.1f);
					//ImGui::DragFloat("combo1_hit_viberation.r_moter", &param.combo_1.hitViberation.R_moter, 0.1f);
					//ImGui::DragFloat("combo1_vibe_time", &param.combo_1.hitViberation.VibeTime, 0.1f);
				}
				if (ImGui::CollapsingHeader("stomp"))
				{
					ImGui::DragInt("stomp_power", &param.stompParam.power, 0.1f);
					ImGui::DragFloat("stomp_invinsible_time", &param.stompParam.invinsibleTime, 0.1f);

					ImGui::Text("stomp_camera_shake");
					ImGui::DragFloat("stomp_shake_x", &param.stompParam.cameraShake.max_X_shake, 0.1f);
					ImGui::DragFloat("stomp_shake_y", &param.stompParam.cameraShake.max_Y_shake, 0.1f);
					ImGui::DragFloat("stomp_time", &param.stompParam.cameraShake.time, 0.1f);
					ImGui::DragFloat("stomp_smmoth", &param.stompParam.cameraShake.shakeSmoothness, 0.1f, 0.1f, 1.0f);
					ImGui::Text("hit_stop");
					ImGui::DragFloat("stomp_stop_time", &param.stompParam.hitStop.time, 0.1f);
					ImGui::DragFloat("stomp_stopping_strengthy", &param.stompParam.hitStop.stoppingStrength, 0.1f);
					//ImGui::DragFloat("combo2_hit_viberation.l_moter", &param.combo_2.hitViberation.L_moter, 0.1f);
					//ImGui::DragFloat("combo2_hit_viberation.r_moter", &param.combo_2.hitViberation.R_moter, 0.1f);
					//ImGui::DragFloat("combo2_vibe_time", &param.combo_2.hitViberation.VibeTime, 0.1f);
				}
			}
			if (ImGui::CollapsingHeader("bossBodyCollision", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::DragFloat3("start", &bossBodyCollision.capsule.start.x);
				ImGui::DragFloat3("end", &bossBodyCollision.capsule.end.x);
				ImGui::DragFloat("radius", &bossBodyCollision.capsule.radius);
				ImGui::DragFloat("height", &bossBodyCollision.height);
				ImGui::DragFloat("attackRadius", &bossBodyCollision.attackRadius);
				ImGui::DragFloat("attackHeight", &bossBodyCollision.attackHeight);
			}
			if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen))
			{
				const char* anime_item[] = {
					"BOSS_IDLE",
					"BOSS_WALK",
					"BOSS_RUN",
					"BOSS_JUMP",
					"BOSS_MISSILE",
					"BOSS_HIT",
					"BOSS_DEAD",
				};
				static int item_current = 0;
				static bool loop = false;
				ImGui::Combo("anime", &item_current, anime_item, IM_ARRAYSIZE(anime_item)); ImGui::Checkbox("is_loop", &loop);
				if (ImGui::Button("play", { 80,20 }))
				{
					bossAnimation = static_cast<BossAnimation>(item_current);
				}
				ImGui::SliderFloat("transition_time", &transition_time, 0.0f, 5.0f);

			}		
		}
		ImGui::End();
	}
	//attack_skill_1->debug_gui("");
	//attack_skill_2->debug_gui("");
#endif
}

void Boss::DebugPrimitiveUpdate()
{
	DebugRenderer* debugRender = Graphics::Instance().GetDebugRenderer();
	
	//debugRender->CreateSphere(
	//	position,
	//	1.0f, { 1.0f,0.0f,0.0f,1.0f });
	debugRender->CreateCylinder(
		bossBodyCollision.capsule.start,
		bossBodyCollision.capsule.radius,
		bossBodyCollision.height,
		{ 0.0f,1.0f,0.0f,1.0f });

	if (attackParam.isAttack)
	{
		debugRender->CreateCylinder(
			bossBodyCollision.capsule.start,
			bossBodyCollision.attackRadius,
			bossBodyCollision.attackHeight,
			{ 1.0f,0.0f,0.0f,1.0f });
	}
}


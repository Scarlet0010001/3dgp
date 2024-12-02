#include "boss.h"
#include "bullet_straight.h"
#include "bullet_manager.h"
#include "shader.h"
#include"user.h"
#include "texture.h"
#include "operators.h"
#include "collision.h"
#include "Graphics.h"

Boss::Boss()
{
	Graphics& graphics = Graphics::Instance();
	//キャラクターモデル
	model = std::make_unique<gltf_model>(graphics.GetDevice().Get(),
		"Resources/glTF-Sample-Models-master/2.0/DamagedHelmet/DamagedHelmet.glb", true);
		//"Resources/Boss/glb/white_crow.glb", true);
	for (auto& node : animated_nodes)
	{
		node = model->nodes;
	}
	blended_animated_nodes = model->nodes;

	//skill_manager = std::make_unique<SkillManager>();
	//UI
	//ui = std::make_unique<PlayerUI>();

	//arm = model->find_nodes("lowerarm_l");

	Initialize();

}

void Boss::Initialize()
{
	//パラメーター初期化
	position = { 0.0f, 0.0f, 0.0f };
	velocity = { 0.0f, 0.0f, 0.0f };
	//Charactorクラスのパラメーター初期化
	charaParam = param.chara_init_param;

	TransitionIdleState();

	//体力初期化
	health = charaParam.maxHealth;

	position = { 0.0f,2.0f,10.0f };
	scale.x = scale.y = scale.z = 2.0f;

	charaParam.moveSpeed = 15.0f;
	state_duration = 2.0f;
	bossBodyCollision.capsule.start = position;
	bossBodyCollision.capsule.radius = 10;
	bossBodyCollision.height = 25;
	bossBodyCollision.capsule.end = bossBodyCollision.capsule.start;
	bossBodyCollision.capsule.end.y = bossBodyCollision.capsule.start.y + bossBodyCollision.height;

	damagedFunction = [=](int damage, float invincible, WINCE_TYPE type)->bool {return ApplyDamage(damage, invincible, type); };
}

void Boss::Update(float elapsedTime)
{
#if _DEBUG
	if (!isUpdate) return;
#endif

	(this->*act_update)(elapsedTime);


	UpdateInvicibleTimer(elapsedTime);

}

void Boss::Render_f(float elapsedTime)
{
	Graphics& graphics = Graphics::Instance();

	//ボスモデルのトランスフォーム更新
	transform = Math::calc_world_matrix(scale, orientation, position, Math::COORDINATE_SYSTEM::RHS_YUP);
	if (bossAnimation_transition != bossAnimation)
	{
		//bossAnimation_old = bossAnimation_transition;
		bossAnimation_transition = bossAnimation;
		transition_state = TransitionState::START;
	}
	bool nowLoop = FindLoopAnimation(bossAnimation);
	//ブレンドアニメーション
	if (transition_state > 0 && transition_time > 0.0f)
	{
		switch (transition_state)
		{
		case TransitionState::NONE:
			break;
		case TransitionState::START:
			model->animate(bossAnimation_old, time, animated_nodes[bossAnimation_old], FindLoopAnimation(bossAnimation_old));
			model->animate(bossAnimation, 0.0f, animated_nodes[bossAnimation], nowLoop);
			transition_state = TransitionState::TRANSITION;
			time = 0.0f;
			factor = 0.0f;

		case TransitionState::TRANSITION:
			factor = time / transition_time;
			model->blend_animations(animated_nodes[bossAnimation_old], animated_nodes[bossAnimation], factor, blended_animated_nodes);
			time += elapsedTime;
			if (factor > 1.0f)
			{
				//End of transition
				transition_state = TransitionState::NONE;
				time = 0;
			}
			break;
		}
		model->render(graphics.Get_DC().Get(), transform, blended_animated_nodes);
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
		model->animate(bossAnimation, time, animated_nodes[bossAnimation], nowLoop);
		model->render(graphics.Get_DC().Get(), transform, animated_nodes[bossAnimation]);
		bossAnimation_old = bossAnimation;

	}

	//デバッグGUI描画
	DebugDUI();
}

void Boss::Render_ui(float elapsedTime)
{
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
			//	load_data_file();
			//}
			//ImGui::Separator();
			//if (ImGui::Button("save"))
			//{
			//	save_data_file();
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
			ImGui::DragInt("hp", &health);
			ImGui::DragFloat("height", &charaParam.height);
			ImGui::DragFloat("turnspeed", &charaParam.turnSpeed, 0.1f);
			ImGui::DragFloat("boss_collision.radius", &bossBodyCollision.capsule.radius, 0.1f);
			ImGui::DragFloat("boss_collision.height", &bossBodyCollision.height, 0.1f);
			ImGui::DragFloat("sickle_.radius", &sickle_hand_colide.radius, 1);
		}
		ImGui::End();
	}
	//attack_skill_1->debug_gui("");
	//attack_skill_2->debug_gui("");
#endif
}

void Boss::CalcAttack_vs_Player(DirectX::XMFLOAT3 capsule_start, DirectX::XMFLOAT3 capsule_end, float colider_radius, AddDamageFunc damaged_func)
{
}

void Boss::OnDead()
{
}

void Boss::OnDamaged(WINCE_TYPE type)
{
}

bool Boss::FindLoopAnimation(BossAnimation PA)
{
	if (PA == BossAnimation::BOSS_IDLE
		//|| PA == BossAnimation::PLAYER_MOVE_FORWARD
		) return true;
	return false;
}

#include "boss.h"
#include "bullet_straight.h"
#include "bullet_homing.h"
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
	chargeEffect = std::make_unique<Effect>("Resources/Effect/Charge/charge.efkefc");

	model->cumulate_transforms(model->nodes, transform);
	for (auto& node : animated_nodes)
	{
		node = model->nodes;
	}
	blended_animated_nodes = model->nodes;

	turretNode = model->find_nodes("Bone_MGun_Main");

	// UIを初期化
	ui = std::make_unique<BossUI>();

	Initialize();

}

void Boss::Initialize()
{
	//パラメーターロード
	LoadDataFile();

	//パラメーター初期化
	position = { 0.0f, 41.0f, 30.0f };
	velocity = { 0.0f, 0.0f, 0.0f };
	scale.x = scale.y = scale.z = 10.0f;

	//Charactorクラスのパラメーター初期化
	charaParam = param.charaInitParam;
	charaParam.maxHealth = 1000;
	lineHealth = 700;

	TransitionIdleState();

	//体力初期化
	health = charaParam.maxHealth;

	stepOffset = 2.0f;  // キャラクターの歩幅のオフセット設定

	// キャラクターの移動速度を設定（歩行速度）
	charaParam.moveSpeed = WALK_SPEED;

	// ステートの継続時間を設定
	stateDuration = 2.0f;  // ステートが続く時間（例えば、アニメーションの継続時間など）

	// ランニング時の移動速度を設定
	param.runSpeed = RUN_SPEED;

	// ボスキャラクターの当たり判定（カプセル形状）の設定
	bossBodyCollision.capsule.start = position;  // カプセルの開始位置を現在位置に設定
	bossBodyCollision.capsule.radius = 5;  // カプセルの半径を設定
	bossBodyCollision.height = 10;  // ボスキャラクターの高さを設定
	bossBodyCollision.capsule.end = bossBodyCollision.capsule.start;  // カプセルの終了位置も開始位置に設定
	bossBodyCollision.capsule.end.y = bossBodyCollision.capsule.start.y + bossBodyCollision.height;  // 高さを加えてカプセルの終点を設定

	// 攻撃用のカプセルの範囲設定
	bossBodyCollision.attackRadius = 5.2f;  // 攻撃範囲の半径を設定
	bossBodyCollision.attackHeight = 10;  // 攻撃範囲の高さを設定

	// ダメージ処理関数をラムダ式で設定（ダメージ、無敵時間、攻撃タイプを引数にとる）
	damagedFunction = [=](int damage, float invincible, WINCE_TYPE type)->bool {
		return ApplyDamage(damage, invincible, type);  // ダメージを適用する関数を呼び出す
	};
}

void Boss::Update(float elapsedTime)
{
#if _DEBUG
	if (!isUpdate) return;  // デバッグビルドの場合、更新処理が無効化されているときはリターン
#endif

	// アクション更新関数を呼び出し（elapsedTimeを引数として渡す）
	(this->*act_update)(elapsedTime);

	// 無敵タイマーの更新
	UpdateInvicibleTimer(elapsedTime);

	// ボスキャラクターの当たり判定（カプセル形状）の位置更新
	bossBodyCollision.capsule.start = position;  // カプセルの開始位置を現在位置に設定
	bossBodyCollision.capsule.end = bossBodyCollision.capsule.start;  // カプセルの終了位置も開始位置と同じに設定
	bossBodyCollision.capsule.end.y = bossBodyCollision.capsule.start.y + bossBodyCollision.height;  // 高さを加えてカプセルの終点を設定

	// ボスのY座標が-10以下になった場合、位置をリセット
	if (position.y < -10.0f)
	{
		position.y = 50.0f;  // 位置をY = 50に設定（復活地点など）
	}

	// 状態タイマーを更新
	stateTimer += elapsedTime;

	//-----------------UI更新-----------------//
	// UIのHPバーを更新（現在のHPパーセンテージを設定）
	ui->SetHPPercent(GetHpPercent());
	// UIのその他の更新
	ui->Update(elapsedTime);

	// デバッグ用のプリミティブ（例えば、ボスの位置などの表示）の更新
	DebugPrimitiveUpdate();
}

void Boss::Render_f(float elapsedTime)
{
	Graphics& graphics = Graphics::Instance();  // グラフィックインスタンスの取得

	// ボスモデルのトランスフォーム更新（スケール、姿勢、位置を基にワールド行列を計算）
	transform = Math::calc_world_matrix(scale, orientation, position, Math::COORDINATE_SYSTEM::RHS_YUP);

	// アニメーションの遷移処理
	if (bossAnimation_transition != bossAnimation)
	{
		// 遷移状態がまだ開始されていない場合
		if (transition_state != TRANSITION_STATE::NONE)
		{
			bossAnimation_old = bossAnimation_transition;  // 前回のアニメーションを記録
			animated_nodes[ANIME_NODE::OLD_ANIMATION] = blended_animated_nodes;  // ブレンドされた古いアニメーションを保持
			transitionToTransition = true;  // 遷移フラグを設定
		}
		bossAnimation_transition = bossAnimation;  // 現在のアニメーションを遷移先に設定
		transition_state = TRANSITION_STATE::START;  // 遷移を開始
	}


	bool nowLoop = FindLoopAnimation(bossAnimation);  // 現在のアニメーションがループするか確認

	// ブレンドアニメーションの処理
	if (transition_state > 0 && transition_time > 0.0f)
	{
		switch (transition_state)
		{
		case TRANSITION_STATE::NONE:
			break;
		case TRANSITION_STATE::START:
			// 遷移の開始処理
			model->animate(bossAnimation_old, time, animated_nodes[ANIME_NODE::OLD_ANIMATION], FindLoopAnimation(bossAnimation_old));  // 古いアニメーションの再生
			model->animate(bossAnimation, 0.0f, animated_nodes[ANIME_NODE::NOW_ANIMATION], nowLoop);  // 新しいアニメーションを開始
			transition_state = TRANSITION_STATE::TRANSITION;  // 遷移状態に進む
			time = 0.0f;  // 時間のリセット
			factor = 0.0f;  // 遷移係数の初期化

		case TRANSITION_STATE::TRANSITION:
			factor = time / transition_time;  // 遷移進行度の計算
			model->blend_animations(animated_nodes[ANIME_NODE::OLD_ANIMATION], animated_nodes[ANIME_NODE::NOW_ANIMATION], factor, blended_animated_nodes);  // アニメーションのブレンド
			time += elapsedTime;  // 経過時間の更新
			if (factor > 1.0f)
			{
				// 遷移が終了した場合
				transition_state = TRANSITION_STATE::NONE;  // 遷移状態をリセット
				time = 0;  // 時間をリセット
			}
			break;
		}
		// レンダリング処理（遷移中）
		model->render(graphics.Get_DC().Get(), transform, blended_animated_nodes);
	}
	else
	{
		// 遷移がない場合、通常のアニメーション再生
		time += elapsedTime;  // 経過時間の更新
		if (model->animations.at(bossAnimation).duration < time)
		{
			if (nowLoop)
				time = 0;  // ループアニメーションの場合、時間をリセット
			else time = model->animations.at(bossAnimation).duration;  // ループしない場合、アニメーションの終了時間に設定
		}
		model->animate(bossAnimation, time, animated_nodes[ANIME_NODE::NOW_ANIMATION], nowLoop);  // 現在のアニメーションを再生
		model->render(graphics.Get_DC().Get(), transform, animated_nodes[ANIME_NODE::NOW_ANIMATION]);  // レンダリング
		bossAnimation_old = bossAnimation;  // 古いアニメーションを更新
	}
}

void Boss::RenderUI(float elapsedTime)
{
	//ボスのUI
	ui->Render();
}

void Boss::ShotBullet(ATTACK_TYPE type)
{
	BulletManager& bulletManager = BulletManager::Instance();
	
	DirectX::XMFLOAT3 shotPos{};
	if (type == ATTACK_TYPE::SHOT_S)
	{
		targetPoint_pos = target_pos;
		//発射位置(プレイヤーの腰あたり)
		model->fech_by_bone(bossAnimation, time, transform, turretNode, shotPos);
		//目標
		DirectX::XMFLOAT3 dir = Math::calc_vector_AtoB_normalize(shotPos,
			{ targetPoint_pos.x, targetPoint_pos.y + targetPoint_height, targetPoint_pos.z });

		BulletStraight* bullet = 
			new BulletStraight(&BulletManager::Instance(), Bullet::BULLET_MASTER::Enemy);
		bullet->Launch(dir, shotPos);
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
		if (damaged_func(attackParam.power, attackParam.invinsibleTime, WINCE_TYPE::SMALL))
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

bool Boss::ApplyDamage(int damage, float invincibleTime, WINCE_TYPE type)
{
	//ダメージが0の場合は健康状態を変更する必要がない
	if (damage == 0)return false;

	//死亡している場合は健康状態を変更しない
	if (health <= 0)return false;


	if (invincibleTimer > 0.0f)return false;

	//無敵時間設定
	invincibleTimer = invincibleTime;
	//ダメージ処理
	health -= damage;

	//死亡通知
	if (health <= 0)
	{
		OnDead();
	}
	else//ダメージ通知
	{
		if (health < lineHealth)
		{
			lineHealth -= 300;
			OnDamaged(type);
		}
	}

	//健康状態が変更した場合はtrueを返す
	return true;
}

void Boss::OnDead()
{
	//死亡状態へ移行
	TransitionDeadState();
}

void Boss::OnDamaged(WINCE_TYPE type)
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

void Boss::LoadDataFile()
{
	// JSONファイルからデータを読み込む
	std::filesystem::path path = filePath;
	path.replace_extension(".json");  // 拡張子を .json に変更

	// 指定したファイルが存在するか確認
	if (std::filesystem::exists(path.c_str()))
	{
		std::ifstream ifs;
		ifs.open(path);  // ファイルを開く

		// ファイルが正常に開けた場合、読み込みとデータをロード
		if (ifs)
		{
			cereal::JSONInputArchive o_archive(ifs);
			o_archive(param);
		}
	}
}

void Boss::SaveDataFile()
{
	// ベースクラスの初期化パラメーター情報を更新
	param.charaInitParam = charaParam;

	// JSONファイルにデータを保存
	std::filesystem::path path = filePath;
	path.replace_extension(".json");
	std::ofstream ifs;
	ifs.open(path);  // ファイルを開く

	// ファイルが正常に開けた場合、書き込みとデータを保存
	if (ifs)
	{
		cereal::JSONOutputArchive o_archive(ifs);
		o_archive(param);  
	}
}

bool Boss::FindLoopAnimation(BossAnimation BA)
{
	//ループさせたいアニメーションだったらtrue
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
			if (ImGui::CollapsingHeader("StateMachine", ImGuiTreeNodeFlags_DefaultOpen))
			{
				const char* stateItem[] = {
					"PLAYER_Idle",
					"PLAYER_Move",
					"PLAYER_Tackle",
					"PLAYER_Jump",
					"PLAYER_ShotStraight",
					"PLAYER_ShotHoming",
					"PLAYER_Damage",
					"PLAYER_Dead",
					"PLAYER_Down",
				};
				using StateUpdateFunc = void (Boss::*)();
				StateUpdateFunc stateUpdate[] = {
					&Boss::TransitionIdleState,
					&Boss::TransitionWalkState,
					&Boss::TransitionAttack_Tackle_State,
					&Boss::TransitionAttack_Jump_State,
					&Boss::TransitionAttack_ShotStraight_State,
					&Boss::TransitionDamageState,
					&Boss::TransitionDeadState,
				};
				static int item_current = 0;
				if (ImGui::Combo("state", &item_current, stateItem, IM_ARRAYSIZE(stateItem)))
				{
					(this->*stateUpdate[item_current])();
				}
			}

		}
		ImGui::End();
	}

	//UIの描画
	ui->DebugGUI();

#endif
}

void Boss::DebugPrimitiveUpdate()
{
	DebugRenderer* debugRender = Graphics::Instance().GetDebugRenderer();
	
	//通常の当たり判定用円柱を生成
	debugRender->CreateCylinder(
		bossBodyCollision.capsule.start,
		bossBodyCollision.capsule.radius,
		bossBodyCollision.height,
		{ 0.0f,1.0f,0.0f,1.0f });

	//攻撃フラグがオンなら攻撃当たり判定用円柱を生成
	if (attackParam.isAttack)
	{
		debugRender->CreateCylinder(
			bossBodyCollision.capsule.start,
			bossBodyCollision.attackRadius,
			bossBodyCollision.attackHeight,
			{ 1.0f,0.0f,0.0f,1.0f });
	}
}


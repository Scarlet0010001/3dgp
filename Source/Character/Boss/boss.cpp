#include "boss.h"
#include "Bullet/bullet_straight.h"
#include "Bullet/bullet_manager.h"
#include "Shader/shader.h"
#include "User/user.h"
#include "Sprite/texture.h"
#include "User/operators.h"
#include "Collision/collision.h"
#include "Graphics/graphics.h"
#include "magic_enum/include/magic_enum.hpp"

#include <filesystem>
#include <fstream>
#include <cereal/archives/json.hpp>

#include <stack>

Boss::Boss()
{
	Graphics& graphics = Graphics::Instance();
	//キャラクターモデルを読み込み
	model = std::make_unique<gltf_model>(graphics.GetDevice().Get(),
		"Resources/Character/Boss/RobotDog_main.glb", false);
	
	//チャージエフェクト読み込み
	chargeEffect = std::make_unique<Effect>("Resources/Effect/Charge/charge.efkefc");

	//初期のノード変換行列を累積して取得
	model->cumulate_transforms(model->nodes, transform);

	//アニメーションノード初期化
	for (auto& node : animatedNodes)
	{
		node = model->nodes;
	}
	//アニメーションブレンド用のノード初期化
	blendedAnimatedNodes = model->nodes;

	//砲塔ノードを取得
	turretNode = model->find_nodes("Bone_MGun_Main");

	//UI初期化
	ui = std::make_unique<BossUI>();

	//初期化処理
	Initialize();

}

void Boss::Initialize()
{
	//パラメーターロード
	LoadDataFile();

	//ボスの初期位置・速度・スケールを設定
	position = INIT_POSITION;
	velocity = {};
	scale.x = scale.y = scale.z = BOSS_SCALE;

	//キャラクターパラメーターを初期化
	charaParam = param.charaInitParam;
	lineHealth = INIT_LINE_HEALTH;

	//待機状態に遷移
	TransitionIdleState();

	//体力を最大値で初期化
	health = charaParam.maxHealth;

	//歩行速度を設定
	charaParam.moveSpeed = WALK_SPEED;

	//ステート継続時間を設定
	stateDuration = STATE_DURATION;

	//走行時の速度を設定
	param.runSpeed = RUN_SPEED;

	//当たり判定（カプセル）の設定
	bossBodyCollision.capsule.start = position;				//カプセル始点を現在位置に設定
	bossBodyCollision.capsule.radius = BODY_RADIUS;			//半径を設定
	bossBodyCollision.height = BODY_HEIGHT;					//カプセルの高さを設定
	
	//カプセルの終了位置も開始位置に設定
	bossBodyCollision.capsule.end = bossBodyCollision.capsule.start;
	//高さを加えてカプセルの終点を設定
	bossBodyCollision.capsule.end.y = bossBodyCollision.capsule.start.y + bossBodyCollision.height;

	//攻撃時の当たり判定の設定
	bossBodyCollision.attackRadius = ATTACK_RADIUS;
	bossBodyCollision.attackHeight = BODY_HEIGHT;

	// ダメージ処理関数をラムダ式で設定（ダメージ、無敵時間、攻撃タイプを引数にとる）
	damagedFunction = [=](int damage, float invincible, WINCE_TYPE type)->bool {
		return ApplyDamage(damage, invincible, type);  // ダメージを適用する関数を呼び出す
	};
}

void Boss::Update(float elapsedTime)
{
#if _DEBUG
	if (!isUpdate) return;  //デバッグビルドの場合、更新処理が無効化されているときはリターン
#endif

	// アクション更新関数を呼び出し
	(this->*actUpdate)(elapsedTime);

	//無敵時間の更新
	UpdateInvicibleTimer(elapsedTime);

	//当たり判定カプセル位置の更新
	bossBodyCollision.capsule.start = position;
	bossBodyCollision.capsule.end = position;
	bossBodyCollision.capsule.end.y += bossBodyCollision.height;

	//Y座標がしきい値より下に落ちたら位置を復帰
	if (position.y < LIMIT_Y)
	{
		position.y = RESPAWN_Y;
	}

	//ステートタイマーを更新
	stateTimer += elapsedTime;

	//-----------------UI更新-----------------//
	//現在のHP割合を設定
	ui->SetHPPercent(GetHpPercent());
	//UI自体の更新処理
	ui->Update(elapsedTime);

	//デバッグプリミティブの更新処理
	DebugPrimitiveUpdate();
}

void Boss::Render_f(float elapsedTime)
{
	//グラフィックインスタンスの取得
	Graphics& graphics = Graphics::Instance();

	//ボスモデルのトランスフォーム更新（スケール、姿勢、位置を基にワールド行列を計算）
	transform = Math::CalcWorldMatrix(scale, orientation, position, Math::COORDINATE_SYSTEM::RHS_YUP);

	//アニメーションの遷移チェック
	if (bossAnimation_transition != bossAnimation)
	{
		if (transitionState != TRANSITION_STATE::NONE)
		{
			// 現在の遷移アニメーションを保存
			bossAnimation_old = bossAnimation_transition;
			animatedNodes[ToInt(ANIME_NODE::OLD_ANIMATION)] = blendedAnimatedNodes;
			transitionToTransition = true;
		}
		//新しいアニメーションへの遷移開始
		bossAnimation_transition = bossAnimation;
		transitionState = TRANSITION_STATE::START;
	}

	//現在のアニメーションがループするか判定
	bool isLoop = FindLoopAnimation(bossAnimation);

	//ブレンドアニメーション処理
	if (transitionState > TRANSITION_STATE::NONE && transitionTime > 0.0f)
	{
		switch (transitionState)
		{
		case TRANSITION_STATE::NONE:
			break;
		case TRANSITION_STATE::START:
			// 直前のアニメーションを設定
			model->animate(ToInt(bossAnimation_old), time, animatedNodes[ToInt(ANIME_NODE::OLD_ANIMATION)], FindLoopAnimation(bossAnimation_old));  // 古いアニメーションの再生
			
			//新しいアニメーションを0秒の状態から設定
			model->animate(ToInt(bossAnimation), 0.0f, animatedNodes[ToInt(ANIME_NODE::NOW_ANIMATION)], isLoop);  // 新しいアニメーションを開始
			
			//遷移ステートを「移行中」に設定
			transitionState = TRANSITION_STATE::TRANSITION;
			time = 0.0f;	//時間のリセット
			factor = 0.0f;  //遷移係数の初期化
			break;

		case TRANSITION_STATE::TRANSITION:
			//アニメーション遷移のブレンド率を計算
			factor = time / transitionTime;

			//旧アニメーションと新アニメーションをブレンド
			model->blend_animations(animatedNodes[ToInt(ANIME_NODE::OLD_ANIMATION)], animatedNodes[ToInt(ANIME_NODE::NOW_ANIMATION)], factor, blendedAnimatedNodes);
			
			//経過時間を加算
			time += elapsedTime;

			//遷移完了判定
			if (factor > FACTOR_MAX)
			{
				//遷移終了処理
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
		if (model->animations.at(ToInt(bossAnimation)).duration < time)
		{
			if (isLoop)
			{
				time = 0;  //ループする場合は最初に戻す
			}
			else
			{
				// ループしない場合は最後のフレームで停止
				time = model->animations.at(ToInt(bossAnimation)).duration;
			}
		}

		//アニメーションを適用
		model->animate(ToInt(bossAnimation), time, animatedNodes[ToInt(ANIME_NODE::NOW_ANIMATION)], isLoop);

		//モデルを描画
		model->render(graphics.Get_DC().Get(), transform, animatedNodes[ToInt(ANIME_NODE::NOW_ANIMATION)]);
		
		//前回のアニメーションを更新
		bossAnimation_old = bossAnimation;
	}
}

void Boss::RenderUI(float elapsedTime)
{
	//ボスのUI
	ui->Render();
}

void Boss::ShotBullet(ATTACK_TYPE type)
{
	//弾マネージャーのインスタンス取得
	BulletManager& bulletManager = BulletManager::Instance();
	
	//弾の発射位置
	DirectX::XMFLOAT3 shotPos{};
	if (type == ATTACK_TYPE::SHOT_S)
	{
		//ターゲットの現在位置を記録
		targetPointPos = targetPos;
		
		//タレットボーンの位置を取得し、発射位置を決定
		model->fech_by_bone(ToInt(bossAnimation), time, transform, turretNode, shotPos);
		
		//ターゲット方向へのベクトルを計算
		DirectX::XMFLOAT3 dir = Math::CalcVectorAtoBNormalize(shotPos,
			{ targetPointPos.x, targetPointPos.y + targetPointHeight, targetPointPos.z });

		//直進弾を生成
		BulletStraight* bullet = 
			new BulletStraight(&BulletManager::Instance(), Bullet::BULLET_MASTER::ENEMY);
		
		//発射処理
		bullet->Launch(dir, shotPos);
	}
}

void Boss::CalcAttack_vs_Player(Capsule capsule_collider, float collider_height, AddDamageFunc damaged_func)
{
	//攻撃フラグが有効でなければ処理を終了
	if (!attackParam.isAttack)
		return;

	//カメラインスタンスを取得
	Camera &camera = Camera::Instance();

	//ボスの攻撃判定とプレイヤーの当たり判定の衝突判定
	if (Collision::CylinderVsCylinder(
		bossBodyCollision.capsule.start, bossBodyCollision.attackRadius, bossBodyCollision.attackHeight,
		capsule_collider.start, capsule_collider.radius, collider_height))
	{
		//ダメージを与えられたか確認
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
	//現在位置から目標地点までの距離を計算
	float direction = Math::CalcVectorAtoBLength(position, target);
	
	//指定された時間内に移動するための速度を返す
	return direction / time;
}

bool Boss::ApplyDamage(int damage, float invincibleTime, WINCE_TYPE type)
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
		//一定ライン以下になるたびに怯む
		if (health < lineHealth)
		{
			//ラインHPを次の段階へ移行
			lineHealth -= LINE_HEALTH_DECREASE;
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
		//怯みダメージ
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
	std::ofstream ofs;
	ofs.open(path);  // ファイルを開く

	// ファイルが正常に開けた場合、書き込みとデータを保存
	if (ofs)
	{
		cereal::JSONOutputArchive o_archive(ofs);
		o_archive(param);  
	}
}

bool Boss::FindLoopAnimation(BOSS_ANIMATION BA)
{
	//ループさせたいアニメーションだったらtrue
	if (BA == BOSS_ANIMATION::BOSS_IDLE
		|| BA == BOSS_ANIMATION::BOSS_WALK
		|| BA == BOSS_ANIMATION::BOSS_RUN
		) return true;
	return false;
}

void Boss::DebugDUI()
{
#if USE_IMGUI
	ImguiMenuBar("Character", "boss", displayImgui);
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
					bossAnimation = static_cast<BOSS_ANIMATION>(item_current);
				}
				ImGui::SliderFloat("transitionTime", &transitionTime, 0.0f, 5.0f);

			}
			if (ImGui::CollapsingHeader("StateMachine", ImGuiTreeNodeFlags_DefaultOpen))
			{
				const char* stateItem[] = {
					"PLAYER_Idle",
					"PLAYER_Move",
					"PLAYER_Tackle",
					"PLAYER_Jump",
					"PLAYER_ShotStraight",
					"PLAYER_Damage",
					"PLAYER_Dead",
					"PLAYER_Down",
				};
				using StateUpdateFunc = void (Boss::*)();
				StateUpdateFunc stateUpdate[] = {
					&Boss::TransitionIdleState,
					&Boss::TransitionWalkState,
					&Boss::TransitionAttackTackleState,
					&Boss::TransitionAttackJumpState,
					&Boss::TransitionAttackShotStraightState,
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
		DEBUG_COLLIDER_COLOR);

	//攻撃フラグがオンなら攻撃当たり判定用円柱を生成
	if (attackParam.isAttack)
	{
		debugRender->CreateCylinder(
			bossBodyCollision.capsule.start,
			bossBodyCollision.attackRadius,
			bossBodyCollision.attackHeight,
			DEBUG_ATTACK_COLOR);
	}
}
#pragma once
#include "device.h"
#include "camera.h"
#include "character.h"
#include "gltf_model.h"

#include "primitive.h"
#include <cereal/cereal.hpp>
class Boss :
    public Character
{
private:
	//==============================================================
	// 
	// 構造体、列挙型
	// 
	//==============================================================

	enum  BossAnimation
	{
		IDLE,
		//AIRBORNE,
		//ATTACK,
		//DAMAGE,
		//DEAD,
		//DIE,
		//DOWN,
		//DOWNDEAD,
		//FALL,
		//GROGGY_END,
		//GROGGY_LOOP,
		//GROGGY_START,
		//RUN,
		//SKILL_1,
		//SKILL_2_END,
		//SKILL_2_LOOP,
		//SKILL_2_START,
		//SKILL_3,
		//STAND,
		//STUN,
		//WALK

	};
	enum class State
	{
		IDLE,
		//AIRBORNE,
		//ATTACK,
		//DAMAGE,
		//DEAD,
		//DIE,
		//DOWN,
		//DOWNDEAD,
		//FALL,
		//GROGGY_END,
		//GROGGY_LOOP,
		//GROGGY_START,
		//RUN,
		//SKILL_1,
		//SKILL_2_END,
		//SKILL_2_LOOP,
		//SKILL_2_START,
		//SKILL_3,
		//STAND,
		//STUN,
		//WALK

	};

public:
	//==============================================================
	// 
	// public関数
	// 
	//==============================================================

	Boss();
	~Boss() {};

	//初期化
	void initialize();

	//更新
	void update(float elapsed_time, Camera* camera);

	//描画処理
	//ディファードでレンダリングするオブジェクト
	void render_d(float elapsed_time, Camera* camera);
	//フォワードレンダリングするオブジェクト
	void render_f(float elapsed_time, Camera* camera);
	//シャドウレンダリングするオブジェクト
	void render_s(float elapsed_time, Camera* camera);
	//UIの描画
	void render_ui(float elapsed_time);

	//デバッグ用GUI描画
	void debug_gui();

	//プレイヤーの攻撃との当たり判定
	void calc_attack_vs_player(DirectX::XMFLOAT3 capsule_start, DirectX::XMFLOAT3 capsule_end, float colider_radius, AddDamageFunc damaged_func);

	//攻撃対象の位置を取得
	void set_location_of_attack_target(DirectX::XMFLOAT3 target) { target_pos = target; }

	BodyCollision get_body_collision() { return boss_body_collision; }

	//カメラがボスを見るときに注視するポイント
	DirectX::XMFLOAT3 get_gazing_point() { return DirectX::XMFLOAT3(position.x, position.y + (chara_param.height + 3), position.z); }


};


#pragma once
#include "device.h"
#include "camera.h"
#include "character.h"

#include "audio.h"
#include "effect.h"

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
		BOSS_IDLE,
		BOSS_WALK,
		BOSS_RUN,
		BOSS_JUMP,
		BOSS_MISSILE,
		BOSS_HIT,
		BOSS_DEAD,
		BOSS_WAKEUP,
		BOSS_CHARGE,
		BOSS_ANIME_COUNT,
	};
	enum class STATE
	{
		IDLE,
		WALK,
		JUMP,
		TACKLE,
		SHOT_S,
		SHOT_H,
		DAMAGE,
		DEAD,
		WAKEUP,
		CHARGE,
	};
	enum class ATTACK_TYPE
	{
		TACKLE,
		JUMP,
		SHOT_S,
		SHOT_H,
		MAX_COUNT
	};

	struct BossParam
	{
		//基底クラスのパラメーター
		CharacterParam chara_init_param;
		float run_speed;

		//タックル攻撃のパラメーター
		AttackParam tackleParam;
		//踏みつけ攻撃のパラメーター
		AttackParam stompParam;

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(
				cereal::make_nvp("chara_param", chara_init_param),
				cereal::make_nvp("run_speed", run_speed),
				cereal::make_nvp("tackleParam", tackleParam),
				cereal::make_nvp("stompParam", stompParam)
			);
		}
	};

public:
	struct BodyCollision
	{
		Capsule capsule;
		float height;

		float attackRadius;
		float attackHeight;
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
	void Initialize();

	//更新
	void Update(float elapsedTime);

	//描画処理
	//フォワードレンダリングするオブジェクト
	void Render_f(float elapsedTime);
	//UIの描画
	void Render_ui(float elapsedTime);

	//デバッグ用GUI描画
	void DebugDUI();

	//デバッグ用当たり判定
	void DebugPrimitiveUpdate();

	//プレイヤーの攻撃との当たり判定
	void CalcAttack_vs_Player(Capsule capsule_collider, float collider_height, AddDamageFunc damaged_func);

	//距離と時間で速度を計算する
	float CalcMoveSpeed(DirectX::XMFLOAT3 target, float time);

	//攻撃対象の位置を取得
	void SetLocationOfAttackTarget(DirectX::XMFLOAT3 target) { target_pos = target; }
	void SetAttackTarget_height(float target) { targetPoint_height = target; }

	BodyCollision GetBodyCollision() { return bossBodyCollision; }

	//カメラがボスを見るときに注視するポイント
	DirectX::XMFLOAT3 GetGazingPoint() { return DirectX::XMFLOAT3(position.x, position.y + (charaParam.height + 3), position.z); }


private:
	/*--------------------状態遷移------------------------*/

	//			移動系				//
	void TransitionIdleState();//待機
	void TransitionWalkState();//歩行

	//			攻撃系				//
	void TransitionAttack_Tackle_State();//近接攻撃
	void TransitionAttack_Jump_State();//ジャンプ攻撃
	void TransitionAttack_ShotStraight_State();//射撃
	void TransitionAttack_ShotHoming_State();//ホーミングミサイル

	//			ダウン系			//
	void TransitionDamageState();
	void TransitionDeadState();
	void TransitionDownState();

	/*---------------状態更新------------------------*/

	//			移動系				//
	void UpdateIdleState(float elapsedTime);//待機
	void UpdateWalkState(float elapsedTime);//歩行

	//			攻撃系				//
	void UpdateAttack_Tackle_State(float elapsedTime);//近接攻撃
	void UpdateAttack_Jump_State(float elapsedTime);//ジャンプ攻撃
	void UpdateAttack_ShotStraight_State(float elapsedTime);//射撃
	void UpdateAttack_ShotHoming_State(float elapsedTime);//ホーミングミサイル

	//			ダウン系			//
	void UpdateDamageState(float elapsedTime);
	void UpdateDeadState(float elapsedTime);
	void UpdateDownState(float elapsedTime);

	//攻撃方法選択
	void AttackRoutine(float elapsedTime);
	void SelectAttackTypeShort();

	void SelectAttackTypeLong();

	//射撃
	void ShotBullet(ATTACK_TYPE type);

	void LookAt_turret(std::vector<gltf_model::node>& nodes);

	//ダメージを受ける処理
	bool ApplyDamage(int damage, float invincibleTime, WINCE_TYPE type)override;

	void OnDead() override;
	void OnDamaged(WINCE_TYPE type) override;

	//データファイル
	void LoadDataFile();
	void SaveDataFile();
	const char* filePath = "Resources/Character/Boss/boss_param.json";

	//---------------------------変数---------------------------//
	typedef void (Boss::* ActUpdate)(float elapsedTime);
	ActUpdate act_update = &Boss::UpdateIdleState;
	std::unique_ptr<gltf_model> model;
	//std::unique_ptr<BossUi> ui;

	gltf_model::node doorNode;
	gltf_model::node turretNode;
	//gltf_model::node turretHeadNode;
	
	//テストでworld行列にしている
	DirectX::XMFLOAT3  turretWorldForward = { 0, 0, 1 };

	float actionTime = 0;
	bool displayImgui = false;

	BossAnimation bossAnimation = BOSS_IDLE;
	BossAnimation bossAnimation_transition = BOSS_IDLE;
	BossAnimation bossAnimation_old = BOSS_IDLE;

	//ループアニメーションの検索
	bool FindLoopAnimation(BossAnimation BA);

	std::vector<gltf_model::node> lookAt_nodes;

	//ステートのタイマー
	float stateTimer;
	//怯むHPライン
	int32_t lineHealth;
	//次のステート移行時間
	float stateDuration;
	//攻撃までの猶予時間
	float attackResponderTimer;
	//攻撃対象
	DirectX::XMFLOAT3 target_pos;
	DirectX::XMFLOAT3 targetPoint_pos{};
	float targetPoint_height = 0.0f;
	DirectX::XMFLOAT3 shot_pos;
	bool isJump = false;

	//エフェクト
	std::unique_ptr<Effect> chargeEffect = nullptr;

	int rapidCount = 0;
	bool isBackJump = false;

	STATE state;

	BossParam param;
	AttackParam attackParam;
	BodyCollision bossBodyCollision;
	Camera::CameraShakeParam tackleCameraShake;//カメラシェイク

#if _DEBUG
	bool isUpdate = true;
	bool isRender = true;
#endif
	//==============================================================
	// 
	// 定数
	// 
	//==============================================================

	//歩くスピード
	const float WALK_SPEED = 6;
	//走るスピード
	const float RUN_SPEED = 30;
	//加速スピード
	const float ACCELERATION_NORMAL_SPEED = 1.5f;
	const float ACCELERATION_JUMP_SPEED = 25.0f;
	//通常攻撃の射程
	const float ATTACK_ACTION_LENGTH = 17;
	//通常攻撃のクールタイム
	const float NORMAL_ATTACK_COOLTIME = 1;

	const float ATTACK_RESPONDER_TIME = 3.0f;

	//連射数
	const int RAPID_MAX = 15;

	//連射間隔の時間
	const float RAPIDFIRE_TIME = 0.5f;

	//ジャンプの攻撃チャージ時間
	const float CHARGE_JUMP_TIME = 2.5f;

	//ダメージを受けたときのスタン時間
	const float DAMAGE_STUN_DURATION = 3.0f;

	public:
		AddDamageFunc damagedFunction;

};


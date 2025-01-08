#pragma once
#include "device.h"
#include "camera.h"
#include "character.h"

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
		BOSS_ANIME_COUNT,
	};
	enum class State
	{
		IDLE,
		WALK,
		RUN,
		JUMP,
		TACKLE,
		SHOT_S,
		SHOT_H,
		DAMAGE,
		DEAD,
	};
	enum class ATTACK_TYPE
	{
		NORMAL,
		SHOT_S,
		SHOT_H,
		MAX_COUNT
	};
	struct BodyCollision
	{
		Capsule capsule;
		float height;
	};


	struct BossParam
	{
		//基底クラスのパラメーター
		CharacterParam chara_init_param;
		float run_speed;

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(
				cereal::make_nvp("chara_param", chara_init_param),
				cereal::make_nvp("run_speed", run_speed)
			);
		}
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
	//ディファードでレンダリングするオブジェクト
	//void render_d(float elapsed_time, Camera* camera);
	//フォワードレンダリングするオブジェクト
	void Render_f(float elapsedTime);
	//シャドウレンダリングするオブジェクト
	//void render_s(float elapsed_time, Camera* camera);
	//UIの描画
	void Render_ui(float elapsedTime);

	//デバッグ用GUI描画
	void DebugDUI();

	//デバッグ用当たり判定
	void DebugPrimitiveUpdate();

	//プレイヤーの攻撃との当たり判定
	void CalcAttack_vs_Player(DirectX::XMFLOAT3 capsule_start, DirectX::XMFLOAT3 capsule_end, float colider_radius, AddDamageFunc damaged_func);

	//攻撃対象の位置を取得
	void SetLocationOfAttackTarget(DirectX::XMFLOAT3 target) { target_pos = target; }

	//BodyCollision get_body_collision() { return boss_body_collision; }

	//カメラがボスを見るときに注視するポイント
	DirectX::XMFLOAT3 GetGazingPoint() { return DirectX::XMFLOAT3(position.x, position.y + (charaParam.height + 3), position.z); }


private:
	/*--------------------状態遷移------------------------*/

	//			移動系				//
	void TransitionIdleState();//待機
	void TransitionWalkState();//歩行
	void TransitionRunState();//走り

	//			攻撃系				//
	void TransitionAttack_Tackle_State();//近接攻撃
	void TransitionAttack_ShotStraight_State();//射撃
	void TransitionAttack_ShotHoming_State();//ホーミングミサイル
	//void TransitionSkill_1_State();

	//			ダウン系			//
	void TransitionDamageState();
	void TransitionDeadState();
	void TransitionDownState();

	/*---------------状態更新------------------------*/

	//			移動系				//
	void UpdateIdleState(float elapsedTime);//待機
	void UpdateWalkState(float elapsedTime);//歩行
	void UpdateRunState(float elapsedTime);//走り

	//			攻撃系				//
	void UpdateAttack_Tackle_State(float elapsedTime);//近接攻撃
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

	void LookAt_turret();

	void OnDead() override;
	void OnDamaged(WINCE_TYPE type) override;

	//データファイル
	//void load_data_file();
	//void save_data_file();
	//const char* file_path = "./resources/Data/boss_param.json";

	// 変数
	typedef void (Boss::* ActUpdate)(float elapsedTime);
	ActUpdate act_update = &Boss::UpdateIdleState;
	std::unique_ptr<gltf_model> model;
	//std::unique_ptr<BossUi> ui;

	gltf_model::node arm;
	gltf_model::node turretNode;
	DirectX::XMFLOAT3  turretLocalForward = { 0, 0, 1 };
	Capsule sickle_hand_colide;

	float actionTime = 0;
	bool displayImgui = false;

	BossAnimation bossAnimation = BOSS_IDLE;
	BossAnimation bossAnimation_transition = BOSS_IDLE;
	BossAnimation bossAnimation_old = BOSS_IDLE;

	//ループアニメーションの検索
	bool FindLoopAnimation(BossAnimation BA);

	//ブレンドアニメーション
	std::vector<gltf_model::node> animated_nodes[BOSS_ANIME_COUNT];


	//ステートのタイマー
	float stateTimer;
	//アイドル状態のままでいる時間
	float state_duration;
	//攻撃までの猶予時間
	float attackResponderTimer;
	//攻撃対象
	DirectX::XMFLOAT3 target_pos;
	DirectX::XMFLOAT3 tackle_pos{};
	DirectX::XMFLOAT3 shot_pos;

	State state;

	BossParam param;
	AttackParam AttackParam;
	BodyCollision bossBodyCollision;

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
	float WALK_SPEED = 5;
	//走るスピード
	float RUN_SPEED = 12;
	//通常攻撃の射程
	float ATTACK_ACTION_LENGTH = 17;
	//通常攻撃のクールタイム
	float NORMAL_ATTACK_COOLTIME = 1;

	float ATTACK_RESPONDER_TIME = 3.0f;
	//ダメージを受けたときのスタン時間
	float DAMAGE_STUN_DURATION = 0.7f;

	public:
		AddDamageFunc damagedFunction;

};


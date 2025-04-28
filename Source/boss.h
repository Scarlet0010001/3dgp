#pragma once
#include "device.h"
#include "camera.h"
#include "character.h"

#include "audio.h"
#include "effect.h"
#include "Boss_UI.h"

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

	//アニメーション
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

	//ステート
	enum class STATE
	{
		IDLE,
		WALK,
		JUMP,
		TACKLE,
		SHOT_S,
		DAMAGE,
		DEAD,
		WAKEUP,
		CHARGE,
	};

	//攻撃属性
	enum class ATTACK_TYPE
	{
		TACKLE,
		JUMP,
		SHOT_S,
		SHOT_H,
		MAX_COUNT
	};

	//ボスパラメータ
	struct BossParam
	{
		//基底クラスのパラメーター
		CharacterParam charaInitParam;

		//走る速度
		float runSpeed;

		//タックル攻撃のパラメーター
		AttackParam tackleParam;
		//踏みつけ攻撃のパラメーター
		AttackParam stompParam;

		//シリアライズ
		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(
				cereal::make_nvp("charaParam", charaInitParam),
				cereal::make_nvp("runSpeed", runSpeed),
				cereal::make_nvp("tackleParam", tackleParam),
				cereal::make_nvp("stompParam", stompParam)
			);
		}
	};

public:
	//ボスの当たり判定
	struct BodyCollision
	{
		//カプセル
		Capsule capsule;

		//高さ
		float height;

		//攻撃当たり判定半径
		float attackRadius;
		//攻撃当たり判定高さ
		float attackHeight;
	};

public:
	//==============================================================
	// 
	// public関数
	// 
	//==============================================================

	//コンストラクタ
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
	void RenderUI(float elapsedTime);
	//デバッグ用GUI描画
	void DebugDUI();

	//デバッグ用当たり判定
	void DebugPrimitiveUpdate();

	//プレイヤーの攻撃との当たり判定
	void CalcAttack_vs_Player(Capsule capsule_collider, float collider_height, AddDamageFunc damaged_func);

	//距離と時間で速度を計算する
	float CalcMoveSpeed(DirectX::XMFLOAT3 target, float time);

	//攻撃対象の位置を設定
	void SetLocationOfAttackTarget(DirectX::XMFLOAT3 target) { target_pos = target; }
	
	//攻撃対象の高さを設定
	void SetAttackTarget_height(float target) { targetPoint_height = target; }

	//ボス当たり判定を取得
	BodyCollision GetBodyCollision() { return bossBodyCollision; }

	//カメラがボスを見るときに注視するポイント
	DirectX::XMFLOAT3 GetGazingPoint() { return DirectX::XMFLOAT3(position.x, position.y + (charaParam.height + 3), position.z); }

	//ループアニメーションの検索
	bool FindLoopAnimation(BossAnimation BA);

private:
	/*--------------------状態遷移------------------------*/

	//			移動系				//
	void TransitionIdleState();									//待機
	void TransitionWalkState();								//歩行

	//			攻撃系				//
	void TransitionAttack_Tackle_State();				//近接攻撃
	void TransitionAttack_Jump_State();				//ジャンプ攻撃
	void TransitionAttack_ShotStraight_State();	//射撃
	void TransitionAttack_ShotHoming_State();	//誘導ミサイル

	//			ダメージ系			//
	void TransitionDamageState();							//ダメージ
	void TransitionDeadState();								//死亡

	/*---------------状態更新------------------------*/

	//			移動系				//
	void UpdateIdleState(float elapsedTime);									//待機
	void UpdateWalkState(float elapsedTime);									//歩行

	//			攻撃系				//
	void UpdateAttack_Tackle_State(float elapsedTime);				//近接攻撃
	void UpdateAttack_Jump_State(float elapsedTime);					//ジャンプ攻撃
	void UpdateAttack_ShotStraight_State(float elapsedTime);	//射撃
	void UpdateAttack_ShotHoming_State(float elapsedTime);	//誘導ミサイル

	//			ダメージ系			//
	void UpdateDamageState(float elapsedTime);							//ダメージ
	void UpdateDeadState(float elapsedTime);									//死亡

	//攻撃方法判定
	void AttackRoutine(float elapsedTime);

	//近距離の攻撃方法選択
	void SelectAttackTypeShort();

	//遠距離の攻撃方法選択
	void SelectAttackTypeLong();

	//射撃
	void ShotBullet(ATTACK_TYPE type);

	//ダメージ処理
	bool ApplyDamage(int damage, float invincibleTime, WINCE_TYPE type)override;

	//死亡時の処理
	void OnDead() override;

	//ダメージ別の判定
	void OnDamaged(WINCE_TYPE type) override;


	//jsonデータファイルのロードとセーブ
	void LoadDataFile();
	void SaveDataFile();

	//jsonファイル名
	const char* filePath = "Resources/Character/Boss/boss_param.json";

	//---------------------------変数---------------------------//
// ボスのアクション更新関数（現在のステートの更新処理を指す関数ポインタ）
	typedef void (Boss::* ActUpdate)(float elapsedTime);
	ActUpdate act_update = &Boss::UpdateIdleState;

	// ボスの 3D モデル（GLTF）
	std::unique_ptr<gltf_model> model;
	// ボス専用 UI
	std::unique_ptr<BossUI> ui;

	// 砲塔（タレット）のノード
	gltf_model::node turretNode;

	// ボスのアニメーション（現在、遷移中、前回の状態を記録）
	BossAnimation bossAnimation = BOSS_IDLE;
	BossAnimation bossAnimation_transition = BOSS_IDLE;
	BossAnimation bossAnimation_old = BOSS_IDLE;

	// ステート関連のタイマー
	float stateTimer;						// 現在のステート経過時間
	float stateDuration;					// 次のステートに移行するまでの時間
	float attackResponderTimer;	// 攻撃までの猶予時間

	// HP 関連
	int32_t lineHealth;  // ボスが怯むHPライン

	// ターゲット情報
	DirectX::XMFLOAT3 target_pos;        // 目標位置
	DirectX::XMFLOAT3 targetPoint_pos{}; // 目標の胴体位置
	float targetPoint_height = 0.0f;     // 目標位置からの高さ
	DirectX::XMFLOAT3 shot_pos;          // 弾を撃つ位置

	// 行動フラグ
	bool isJump = false;   // ジャンプ中かどうか
	bool isBackJump = false; // バックステップ中かどうか

	// エフェクト
	std::unique_ptr<Effect> chargeEffect = nullptr; // チャージ攻撃のエフェクト

	// 連射関連
	int rapidCount = 0;  // 連射のカウント

	// ボスの現在のステート
	STATE state;

	// ボスのパラメータ
	BossParam param;
	// 攻撃のパラメータ
	AttackParam attackParam;
	// ボスの当たり判定（体のコリジョン情報）
	BodyCollision bossBodyCollision;
	// タックル時のカメラシェイク情報
	Camera::CameraShakeParam tackleCameraShake;

	bool displayImgui = false; // ImGui デバッグウィンドウを表示するか
#if _DEBUG
	bool isUpdate = true;  // 更新処理を行うか
	bool isRender = true;  // 描画処理を行うか
#endif	//==============================================================
	// 
	// 定数
	// 
	//==============================================================

	// 移動速度
	const float WALK_SPEED = 6;		//歩くスピード
	const float RUN_SPEED = 30;		//走るスピード

	//加速度
	const float ACCELERATION_NORMAL_SPEED = 1.5f;	// 通常の加速度
	const float ACCELERATION_JUMP_SPEED = 25.0f;		// ジャンプ時の加速度
	
	// 攻撃関連
	
	const float ATTACK_ACTION_LENGTH = 17;			//通常攻撃の射程
	const float NORMAL_ATTACK_COOLTIME = 1;		//通常攻撃のクールタイム
	const float ATTACK_RESPONDER_TIME = 2.0f;	// 近距離以上の射程時の判定時間

	// 連射関連
	const int RAPID_MAX = 10;					// 最大連射回数
	const float RAPIDFIRE_TIME = 0.5f;	//連射間隔

	const float CHARGE_JUMP_TIME = 2.5f;	//ジャンプ攻撃のチャージ時間

	//ダメージを受けたときのスタン時間
	const float DAMAGE_STUN_DURATION = 3.0f;

public:
	// 被ダメージ時のコールバック関数
	AddDamageFunc damagedFunction;

};


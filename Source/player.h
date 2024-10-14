#pragma once
#include "device.h"
#include "camera.h"
#include "character.h"
#include "gltf_model.h"

#include "primitive.h"
#include <cereal/cereal.hpp>

//プレイヤー :final このクラスの継承ができないことを明示する
class Player final :
    public Character
{
public:
    Player();
    ~Player()override;

	//初期化処理
	void Initialize();
	//更新処理
	void Update(float elapsed_time);
	//描画処理
//ディファードでレンダリングするオブジェクト
	void Render_d(float elapsed_time);
	//フォワードレンダリングするオブジェクト
	void Render_f(float elapsed_time);
	//シャドウレンダリングするオブジェクト
	void Render_s(float elapsed_time);
	//UI描画
	void RenderUI(float elapsed_time);
	//デバッグ用GUI描画
	void DebugGUI();

	//プレイヤーの腰当たりの位置
	DirectX::XMFLOAT3 GetWaistPosition() { return DirectX::XMFLOAT3(position.x, position.y + charaParam.height / 2, position.z); }
	//カメラがプレイヤーを見るときに注視するポイント
	DirectX::XMFLOAT3 GetGazingPoint() { return DirectX::XMFLOAT3(position.x, position.y + (charaParam.height + 3), position.z); }

	//プレイヤーのコリジョンと敵の当たり判定
	void CalcCollision_vs_Enemy(Capsule capsule_collider, float colider_height);

	//プレイヤーの攻撃と敵の当たり判定
	void CalcAttack_vs_Enemy(Capsule collider, AddDamageFunc damaged_func);

	//スキルと敵の当たり判定
	void JudgeSkillCollision(Capsule object_colider, AddDamageFunc damaged_func);

private:
	//-------------構造体、列挙型--------------//
	//アニメーション
	enum  PlayerAnimation
	{
		PLAYER_IDLE,//待機
		PLAYER_RUN,//走り
		PLAYER_ROLL,//回避
		PLAYER_JUMP,//ジャンプ
		PLAYER_DAMAGE_FRONT,//前から被ダメ
		PLAYER_ATK_SPRING_SLASH,//前回転切り
		PLAYER_PULL_SLASH,//敵を引き付けて斬る
		PLAYER_ATK_GROUND,//地面に手を付けて口寄せみたいな
		PLAYER_MAGIC_BUFF,//バフ
		PLAYER_MAGIC_SLASH_UP,//空中に巻き上げ斬る
		PLAYER_MAGIC_BULLET,//小さい魔法弾打つような
		PLAYER_ATK_FORWARD_SLASH,//前進斬り
		PLAYER_ATK_AIR,//ジャンプして地面に魔法うつ
		PLAYER_ATK_COMBO1,//コンボ2-1
		PLAYER_ATK_COMBO2,//コンボ2-2
		PLAYER_ATK_COMBO3,//コンボ2-3
		PLAYER_ATK_DODGE_BACK,//後方に回避しながら魔法
	};
	PlayerAnimation playerAnimation = PLAYER_IDLE;


	//ステート
	enum class State
	{
		IDLE,
		MOVE,
		ROLL,
		JUMP,
		SHOT,
		LANDING,
		FRONT_DAMAGE,
		NORMAL_ATTACK,
		SKILL,

	};

	const float JUST_GURD_TIME = 3.0f;

	struct PlayerParam
	{
		//基底クラスのパラメーター
		CharacterParam charaInitParam;
		//ジャンプスピード
		float jumpSpeed = 21;
		//回避速度
		float avoidanceSpeed = 50;
		//debug用タイマー
		int avoidanceTimer = 0;
		//浮遊度
		float floatingValue = 10.0f;
		//剣エフェクトの速度
		float swordSwingSpeed = 1500.0f;
		//コンボ1攻撃のパラメーター
		AttackParam combo_1;
		//コンボ2のパラメーター
		AttackParam combo_2;
		//コンボ3のパラメーター
		AttackParam combo_3;


		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(
				cereal::make_nvp("charaParam", charaInitParam),
				cereal::make_nvp("jumpSpeed", jumpSpeed),
				cereal::make_nvp("avoidanceSpeed", avoidanceSpeed),
				cereal::make_nvp("floatingValue", floatingValue),
				cereal::make_nvp("swordSwingSpeed", swordSwingSpeed),
				cereal::make_nvp("attack_combo_1", combo_1),
				cereal::make_nvp("attack_combo_2", combo_2),
				cereal::make_nvp("attack_combo_3", combo_3)
			);
		}
	};

	struct ShieldParam
	{
		bool isShield;//ガード状態か
		bool shieldable;//ガード可能状態か
		bool isBreakShield;//シールドが破壊状態か
		float shieldTime;
		float justGuardTime;
		float recastShieldTime;
		float shieldHp;//耐久
		float recastRate;
		float SHIELD_HP_MAX = 50;

	};

private:

	//------------遷移--------------//
	void TransitionIdleState();//待機
	void TransitionMoveState();//走り
	void TransitionAvoidanceState();//回避
	void TransitionJumpState();//ジャンプ
	void TransitionShotState();//射撃


	//--------各ステートのアップデート--------//r_はルートモーション付き
	void UpdateIdleState(float elapsedTime);//待機
	void UpdateMoveState(float elapsedTime);//走り
	void UpdateAvoidanceState(float elapsedTime);//回避
	void UpdateJumpState(float elapsedTime);//ジャンプ
	void UpdateShotState(float elapsedTime);//射撃


	//更新関数の関数ポインタの定義
	typedef void (Player::* ActUpdate)(float elapsedTime);

	//プレイヤーの移動入力処理
	bool InputMove(float elapsedTime);

	//制限付きの移動（攻撃中などの移動入力）
	bool InputMove(float elapsedTime, float restrictionMove, float restrictionTurn);
	const DirectX::XMFLOAT3 GetMoveVec(Camera* camera) const;
	//ジャンプ入力処理
	void InputJump();
	//回避入力
	void InputAvoidance();

	//着地したか
	void OnLanding()override;
	//死亡したときの処理
	void OnDead() override;
	//ダメージを受けた時の処理
	void OnDamaged(WINCE_TYPE type) override;
	//ダメージを受ける処理
	bool ApplyDamage(int damage, float invincible_time, WINCE_TYPE type)override;
	//ルートモーション
	//void RootMotion(DirectX::XMFLOAT3 dir, float speed);
	//void RootMotionManual(DirectX::XMFLOAT3 dir, float speed);

	//落下速度を落とす
	bool Floating();

	//浮遊する
	bool Flying();

	//--------------------変数--------------------------
	//関数ポインタの宣言
	ActUpdate p_update = &Player::UpdateIdleState;

	PlayerParam param;
	State state;

	GamePad* gamePad;
	Mouse* mouse;
	Camera* camera;

	std::unique_ptr <gltf_model> model;

	//現何回ジャンプしてるか
	int jump_count = 0;
	//ジャンプ可能回数
	const int jump_limit = 1;

	bool displayPlayerImgui = false;


	//------------------攻撃関連--------------------------

	AttackParam attackParam;


	DirectX::XMFLOAT3 forward;


	//------------------デバッグ-------------------------
	bool isFallDawn = false;
public:
	//ダメージを受けたときに呼ばれる *関数を呼ぶのはダメージを与えたオブジェクト
	AddDamageFunc damagedFunction;
	Capsule collider;


};


#pragma once
#include "device.h"
#include "camera.h"
#include "character.h"
//#include "gltf_model.h"

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
	void Update(float elapsedTime);
	//描画処理
//ディファードでレンダリングするオブジェクト
	void Render_d(float elapsedTime);
	//フォワードレンダリングするオブジェクト
	void Render_f(float elapsedTime);
	//シャドウレンダリングするオブジェクト
	void Render_s(float elapsedTime);
	//UI描画
	void RenderUI(float elapsedTime);
	//デバッグ用GUI描画
	void DebugGUI();

	//プレイヤーの腰当たりの位置
	DirectX::XMFLOAT3 GetWaistPosition() { return DirectX::XMFLOAT3(position.x, position.y + charaParam.height / 2, position.z); }
	//カメラがプレイヤーを見るときに注視するポイント
	DirectX::XMFLOAT3 GetGazingPoint() { return DirectX::XMFLOAT3(position.x, position.y + (charaParam.height + 1.5f), position.z); }


	//プレイヤーのコリジョンと敵の当たり判定
	void CalcCollision_vs_Enemy(Capsule capsule_collider, float colider_height);

	//プレイヤーの攻撃と敵の当たり判定
	void CalcAttack_vs_Enemy(Capsule capsule_collider, float colider_height, AddDamageFunc damaged_func);

	//スキルと敵の当たり判定
	void JudgeSkillCollision(Capsule object_colider, AddDamageFunc damaged_func);

private:
	//-------------構造体、列挙型--------------//
	//アニメーション
	enum  PlayerAnimation
	{
		PLAYER_IDLE,//待機

		PLAYER_MOVE_FORWARD,//走り前
		PLAYER_MOVE_LEFT,//走り左
		PLAYER_MOVE_RIGHT,//走り右
		PLAYER_MOVE_BACK,//走り後ろ

		PLAYER_JUMP_START,//ジャンプ始め
		PLAYER_JUMP,//ジャンプ途中
		PLAYER_JUMP_END,//ジャンプ終わり

		PLAYER_WING_START,//飛行変形
		PLAYER_WING,//飛行
		PLAYER_WING_END,//地上変形

		PLAYER_SHOT_IDLE, //射撃前
		PLAYER_SHOT_FORWARD,//射撃前
		PLAYER_SHOT_LEFT,//射撃左
		PLAYER_SHOT_RIGHT,//射撃右
		PLAYER_SHOT_BACK,//射撃後ろ

		PLAYER_ATTACK_01,//近接01
		PLAYER_ATTACK_02,//近接02
		PLAYER_ATTACK_03,//近接03
		PLAYER_POWER_L,//強攻撃左
		PLAYER_POWER_R,//強攻撃右

		PLAYER_ANIME_COUNT,
	};
	PlayerAnimation playerAnimation = PLAYER_IDLE;
	PlayerAnimation playerAnimation_transition = PLAYER_IDLE;
	PlayerAnimation playerAnimation_old = PLAYER_IDLE;

	//ループアニメーションの検索
	bool FindLoopAnimation(PlayerAnimation playerAnimation);

	//ステート
	enum class STATE
	{
		IDLE,
		MOVE,
		JUMP,
		BOOST,
		WING,
		SHOT,
		LEFT_ATTACK,
		RIGHT_ATTACK,
		DAMAGE,
		DIE,
		ROLL,
		SKILL,

	};

	const float JUST_GURD_TIME = 3.0f;
	const float MAX_BOOST_TIMER = 10.0f;

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
		//飛行速度
		float wingSpeed = 40;
		//ブースト
		float boostTimer = 10.0f;
		//浮遊度
		float floatingValue = 1.5f;
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
				cereal::make_nvp("wingSpeed", wingSpeed),
				cereal::make_nvp("boostTimer", boostTimer),
				cereal::make_nvp("floatingValue", floatingValue),
				cereal::make_nvp("swordSwingSpeed", swordSwingSpeed),
				cereal::make_nvp("attack_combo_1", combo_1),
				cereal::make_nvp("attack_combo_2", combo_2),
				cereal::make_nvp("attack_combo_3", combo_3)
			);
		}
	};

private:

	//------------遷移--------------//
	void TransitionIdleState();//待機
	void TransitionMoveState();//走り
	void TransitionWingState();//飛行
	//void TransitionAvoidanceState();//回避
	void TransitionJumpState();//ジャンプ
	void TransitionLandingState();//着地
	void TransitionShotState();//射撃
	void TransitionCombo_01_01_State();//近接コンボ１
	void TransitionCombo_01_02_State();//近接コンボ２
	void TransitionCombo_01_03_State();//近接コンボ３
	void TransitionCombo_PowerL_State();//強攻撃左
	void TransitionCombo_PowerR_State();//強攻撃右


	//--------各ステートのアップデート--------//r_はルートモーション付き
	void UpdateIdleState(float elapsedTime);//待機
	void UpdateMoveState(float elapsedTime);//走り
	void UpdateWingState(float elapsedTime);//飛行
	//void UpdateAvoidanceState(float elapsedTime);//回避
	void UpdateJumpState(float elapsedTime);//ジャンプ
	void UpdateLandingState(float elapsedTime);//着地
	void UpdateShotState(float elapsedTime);//射撃
	void UpdateCombo_01_01_State(float elapsedTime);//近接コンボ１
	void UpdateCombo_01_02_State(float elapsedTime);//近接コンボ２
	void UpdateCombo_01_03_State(float elapsedTime);//近接コンボ３
	void UpdateCombo_PowerL_State(float elapsedTime);//強攻撃左
	void UpdateCombo_PowerR_State(float elapsedTime);//強攻撃右


	//更新関数の関数ポインタの定義
	typedef void (Player::* ActUpdate)(float elapsedTime);

	void Move(float vx, float vz, float speed)override;
	void Move(float vx, float vy, float vz, float speed);

	void BoostUpdate(float elapsedTime);

	//プレイヤーの移動入力処理
	bool InputMove(float elapsedTime);
	//プレイヤーの飛行入力処理
	bool InputMoveWing(float elapsedTime);

	//制限付きの移動（攻撃中などの移動入力）
	bool InputMove(float elapsedTime, float restrictionMove, float restrictionTurn);
	
	bool InputMove(float elapsedTime, float move_speed);
	
	const DirectX::XMFLOAT3 GetMoveVec(Camera* camera, bool wing = false) const;

	//ジャンプ入力処理
	void InputJump();
	//回避入力
	void InputAvoidance();

	//飛行入力
	void InputWing();

	//射撃入力
	void InputShot();

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

	//垂直速力更新処理
	 void UpdateVerticalVelocity(float elapsed_frame)override;

	 //----------<ファイル>------------//
	 void LoadDataFile();
	 void SaveDataFile();
	 const char* filePath = "Resources/Character/Player/player_param.json";

private:
	//--------------------変数--------------------------
	//関数ポインタの宣言
	ActUpdate p_update = &Player::UpdateIdleState;

	PlayerParam param;
	STATE state;

	GamePad* gamePad;
	Mouse* mouse;
	Camera* camera;

	std::unique_ptr <gltf_model> model;

	//当たり判定ノード
	enum LR
	{
		LEFT,
		RIGHT,
		COUNT,
	};
	gltf_model::node beamSaber[LR::COUNT];
	gltf_model::node lowerArm[LR::COUNT];
	DirectX::XMFLOAT3 beamSaber_position[LR::COUNT]{};
	DirectX::XMFLOAT3 lowerArm_position[LR::COUNT]{};
	DirectX::XMFLOAT3 attackCollision_position[LR::COUNT]{};

	//float anime_time = 0.0f;

	//現何回ジャンプしてるか
	int jumpCount = 0;
	//ジャンプ可能回数
	const int jumpLimit = 1;

	bool isHover = false;
	bool isBoost = false;

	bool displayPlayerImgui = false;

	//------------------攻撃関連--------------------------

	AttackParam attackParam;
	bool nextCombo = false;

	DirectX::XMFLOAT3 forward;


	//------------------デバッグ-------------------------
public:
	//ダメージを受けたときに呼ばれる *関数を呼ぶのはダメージを与えたオブジェクト
	AddDamageFunc damagedFunction;
	Capsule collider;

private:
	void DebugPrimitiveUpdate();

};


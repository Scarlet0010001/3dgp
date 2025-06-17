#pragma once
#include "device.h"
#include "camera.h"
#include "character.h"
#include "player_UI.h"
#include "radial_blur.h"
#include "glitch_chromatic_aberration.h"

#include "effect.h"
#include "audio.h"

#include "primitive.h"
#include <cereal/cereal.hpp>

//ブーストゲージ最大量
#define BOOST_MAX (10.0f)

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
	//フォワードレンダリングするオブジェクト
	void Render_f(float elapsedTime);
	//UI描画
	void RenderUI(float elapsedTime);
	//デバッグ用GUI描画
	void DebugGUI();
	//デバッグプリミティブ更新
	void DebugPrimitiveUpdate();

	//プレイヤーの腰当たりの位置
	DirectX::XMFLOAT3 GetWaistPosition() { return DirectX::XMFLOAT3(position.x, position.y + charaParam.height / 2, position.z); }
	//カメラがプレイヤーを見るときに注視するポイント
	DirectX::XMFLOAT3 GetGazingPoint() { return DirectX::XMFLOAT3(position.x, position.y + (charaParam.height + 1.5f), position.z); }
	
	//攻撃パラメータ取得
	AttackParam GetAttackParam() { return attackParam; }

	//HPパーセンテージ取得
	float GetBoostPercent() const { return param.boostTimer <= 0 ? 0.0f : static_cast<float>(param.boostTimer) / BOOST_MAX; }

	//ボス座標設定
	void SetBossPosition(DirectX::XMFLOAT3 p) { bossPosition = p; }

	//ラジアルブラー取得
	RadialBlur::radial_blur_constants GetRadialBlur() { return player_radialBlur_constant; }
	
	//色収差取得
	Glitch_CA::glitch_CA_constants GetGlitch_CA() { return player_glitch_CA_constant; }
	
	//プレイヤーのコリジョンと敵の当たり判定
	void CalcCollision_vs_Enemy(Capsule capsule_collider, float colider_height);

	//プレイヤーの攻撃と敵の当たり判定
	void CalcAttack_vs_Enemy(Capsule capsule_collider, float colider_height, AddDamageFunc damaged_func);

	//ステージ制限壁判定
	void CalcLimitWall(float limit_xz, float y, bool isKill = false);

private:
	//-------------構造体、列挙型--------------//
	//アニメーション
	enum PlayerAnimation
	{
		PLAYER_IDLE,			//待機

		PLAYER_MOVE_FORWARD,	//走り前
		PLAYER_MOVE_LEFT,		//走り左
		PLAYER_MOVE_RIGHT,		//走り右
		PLAYER_MOVE_BACK,		//走り後ろ

		PLAYER_JUMP_START,		//ジャンプ始め
		PLAYER_JUMP,			//ジャンプ途中
		PLAYER_JUMP_END,		//ジャンプ終わり

		PLAYER_WING_START,		//飛行変形
		PLAYER_WING,			//飛行
		PLAYER_WING_END,		//地上変形

		PLAYER_SHOT_IDLE,		//射撃前
		PLAYER_SHOT_FORWARD,	//射撃前
		PLAYER_SHOT_LEFT,		//射撃左
		PLAYER_SHOT_RIGHT,		//射撃右
		PLAYER_SHOT_BACK,		//射撃後ろ

		PLAYER_ATTACK_01,		//コンボ01
		PLAYER_ATTACK_02,		//コンボ02
		PLAYER_ATTACK_03,		//コンボ03
		PLAYER_POWER_L,			//強攻撃左
		PLAYER_POWER_R,			//強攻撃右

		PLAYER_DAMAGE,			//被弾
		PLAYER_DEAD,			//死亡

		PLAYER_ANIME_COUNT,		//アニメーションの総数
	};

	//アニメーションの現在の状態
	PlayerAnimation playerAnimation = PlayerAnimation::PLAYER_IDLE;
	//アニメーションの遷移先の状態
	PlayerAnimation playerAnimation_transition = PlayerAnimation::PLAYER_IDLE;
	//直前のアニメーションの状態
	PlayerAnimation playerAnimation_old = PlayerAnimation::PLAYER_IDLE;

	//ループアニメーションの検索
	bool FindLoopAnimation(PlayerAnimation playerAnimation);

	//ステート
	enum class STATE
	{
		IDLE,			//待機
		MOVE,			//走り
		JUMP,			//ジャンプ
		BOOST,			//ブースト
		WING,			//飛行
		SHOT,			//射撃
		LEFT_ATTACK,	//左手攻撃
		RIGHT_ATTACK,	//右手攻撃
		DAMAGE,			//被弾
		DEAD,			//死亡
	};

	struct PlayerParam
	{
		//基底クラスのパラメーター
		CharacterParam charaInitParam;
		//ジャンプスピード
		float jumpSpeed = 21;
		//回避速度
		float avoidanceSpeed = 50;
		//回避用タイマー
		float avoidanceTimer = 0.0f;
		//飛行速度
		float wingSpeed = 40;
		//ブースト
		float boostTimer = BOOST_MAX;
		//浮遊度
		float floatingValue = 1.5f;
		//攻撃時の移動速度
		float attackMoveSpeed =10.0f;
		//コンボ1のパラメーター
		AttackParam combo_1;
		//コンボ2のパラメーター
		AttackParam combo_2;
		//コンボ3のパラメーター
		AttackParam combo_3;

		//シリアライズ
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
				cereal::make_nvp("attackMoveSpeed", attackMoveSpeed),
				cereal::make_nvp("attack_combo_1", combo_1),
				cereal::make_nvp("attack_combo_2", combo_2),
				cereal::make_nvp("attack_combo_3", combo_3)
			);
		}
	};

private:

	//------------遷移--------------//
	void TransitionIdleState();				//待機
	void TransitionMoveState();				//走り
	void TransitionWingState();				//飛行

	void TransitionAvoidanceState();		//回避
	void TransitionJumpState();				//ジャンプ
	void TransitionLandingState();			//着地

	void TransitionShotState();				//射撃
	void TransitionCombo_01_01_State();		//近接コンボ１
	void TransitionCombo_01_02_State();		//近接コンボ２
	void TransitionCombo_01_03_State();		//近接コンボ３

	void TransitionDamageState();			//ダメージ
	void TransitionDeadState();				//死亡


	//--------各ステートのアップデート--------//
	void UpdateIdleState(float elapsedTime);			//待機
	void UpdateMoveState(float elapsedTime);			//走り
	void UpdateWingState(float elapsedTime);			//飛行

	void UpdateAvoidanceState(float elapsedTime);		//回避
	void UpdateJumpState(float elapsedTime);			//ジャンプ
	void UpdateLandingState(float elapsedTime);			//着地

	void UpdateShotState(float elapsedTime);			//射撃
	void UpdateCombo_01_01_State(float elapsedTime);	//近接コンボ１
	void UpdateCombo_01_02_State(float elapsedTime);	//近接コンボ２
	void UpdateCombo_01_03_State(float elapsedTime);	//近接コンボ３

	void UpdateDamageState(float elapsedTime);			//ダメージ
	void UpdateDeadState(float elapsedTime);			//死亡


	//更新関数の関数ポインタの定義
	typedef void (Player::* ActUpdate)(float elapsedTime);

	//移動ベクトルと速度設定
	void Move(float vx, float vz, float speed)override;
	void Move(float vx, float vy, float vz, float speed);

	//ブーストの更新処理
	void BoostUpdate(float elapsedTime);

	//プレイヤーの移動入力処理
	bool InputMove(float elapsedTime);
	//プレイヤーの飛行入力処理
	bool InputMoveWing(float elapsedTime);

	//制限付きの移動（攻撃中などの移動入力）
	bool InputMove(float elapsedTime, float restrictionMove, float restrictionTurn);
	
	bool InputMove(float elapsedTime, float move_speed);
	
	//入力ベクトル算出
	const DirectX::XMFLOAT3 GetMoveVec(Camera* camera, bool wing = false) const;

	//ラジアルブラー
	void ShaderUpdate(float elapsedTime);

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

	//軌跡更新処理
	void TrailUpdate();

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

	//プレイヤーパラメーター
	PlayerParam param;

	//現在の状態
	STATE state;

	//それぞれのインスタンス保存用
	GamePad* gamePad;
	Mouse* mouse;
	Camera* camera;

	//SE
	enum PLAYER_SE
	{
		SE_SABER = 0,		//サーベル音
		SE_LASER = 1,		//射撃音
		SE_BOOST =2,		//ブースト音
		SE_DAMAGE = 3,	//被弾音
	};
	std::shared_ptr<audio> audios[8];

	//UI
	std::unique_ptr<PlayerUI> ui;

	//モデル
	std::unique_ptr <gltf_model> model;

	//ラジアルブラー
	RadialBlur::radial_blur_constants player_radialBlur_constant{}; 
	float radialTimer = 0.0f;	//ラジアルブラータイマー

	//色収差
	Glitch_CA::glitch_CA_constants player_glitch_CA_constant{};
	float glitch_CATimer = 0.0f;//色収差タイマー
	bool isGlitch_CA = false;		//色収差オンオフ

	//左手右手
	enum LR
	{
		LEFT,	//左手
		RIGHT,	//右手
		COUNT,	//要素の数（enumの終端）
	};
	//当たり判定ノード
	gltf_model::node beamSaber[LR::COUNT];	//サーベルの先端
	gltf_model::node lowerArm[LR::COUNT];	//腕の先端

	DirectX::XMFLOAT3 beamSaber_position[LR::COUNT]{};			//サーベルの先端位置
	DirectX::XMFLOAT3 lowerArm_position[LR::COUNT]{};			//腕の先端位置
	DirectX::XMFLOAT3 attackCollision_position[LR::COUNT]{};	//サーベルの当たり判定位置
	
	//軌跡
	enum class TRAIL
	{
		LOWER_ARM,	//腕の先端
		BEAM_SABER,	//サーベルの先端
		COUNT,		//要素の数（enumの終端）
	};
	static const int MAX_POLYGON = 12;

	struct TrailParam
	{
		DirectX::XMFLOAT3 trailPositions[ToInt(TRAIL::COUNT)][MAX_POLYGON];	//軌跡の保存座標
		DirectX::XMFLOAT4 color[MAX_POLYGON];
	};
	TrailParam trailAttack[ToInt(LR::COUNT)];
	//軌跡を初期化判定
	bool resetTrail = false;

	//ボス座標
	DirectX::XMFLOAT3 bossPosition{};

	//加速度状態
	enum ACCELERATION_STATE
	{
		MOVE,					//走り
		AVOIDANCE,				//ブースト
		WING,					//飛行
		ACCELERATION_COUNT,		//総数
	};
	float accelerationState[ACCELERATION_STATE::ACCELERATION_COUNT]{
		1.5f,			//走り
		50.0f,			//ブースト
		25.0f			//飛行
	};

	//現何回ジャンプしてるか
	int jumpCount = 0;
	//ジャンプ可能回数
	const int jumpLimit = 1;

	//ブーストのオンオフ
	bool isBoost = false;

	//エフェクト
	std::unique_ptr<Effect> slashEffect = nullptr;

	//Imguiのオンオフ
	bool displayPlayerImgui = false;

	//------------------攻撃関連--------------------------

	AttackParam attackParam;

	//先行入力判定
	bool nextCombo = false;

	//前方方向
	DirectX::XMFLOAT3 forward;

	//------------------デバッグ-------------------------
public:
	//ダメージを受けたときに呼ばれる *関数を呼ぶのはダメージを与えたオブジェクト
	AddDamageFunc damagedFunction;
	
	//当たり判定用カプセル
	Capsule collider;
	
private:
	//--------------------定数--------------------------//
	//着地ステートに偏移する速度
	const float LANDING_SPEED = 30.0f;
};


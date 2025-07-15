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
class PLAYER final :
    public Character
{
public:
	//コンストラクタとデストラクタ
    PLAYER();
    ~PLAYER()override;

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
	RadialBlur::radialBlurConstants GetRadialBlur() { return player_RadialBlurConstant; }
	
	//色収差取得
	Glitch_CA::Glitch_CA_constants GetGlitch_CA() { return player_Glitch_CA_Constant; }
	
	//プレイヤーのコリジョンと敵の当たり判定
	void CalcCollision_vs_Enemy(Capsule capsule_collider, float colider_height);

	//プレイヤーの攻撃と敵の当たり判定
	void CalcAttack_vs_Enemy(Capsule capsule_collider, float colider_height, AddDamageFunc damaged_func);

private:
	//-------------構造体、列挙型--------------//
	//アニメーション
	enum class PlayerAnimation
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

	//コンボ
	enum class COMBO
	{
		NONE = -1,
		ATTACK01 = 0,
		ATTACK02,
		ATTACK03,
		COUNT,
	};

	struct PlayerParam
	{
		//基底クラスのパラメーター
		CharacterParam charaInitParam;
		//ジャンプスピード
		float jumpSpeed = 21;
		//回避速度
		float boostSpeed = 50;
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
				cereal::make_nvp("boostSpeed", boostSpeed),
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
	void TransitionIdleState();			//待機
	void TransitionMoveState();			//走り
	void TransitionWingState();			//飛行

	void TransitionBoostState();		//回避
	void TransitionJumpState();			//ジャンプ
	void TransitionLandingState();		//着地

	void TransitionShotState();			//射撃
	void TransitionCombo01State();		//近接コンボ１
	void TransitionCombo02State();		//近接コンボ２
	void TransitionCombo03State();		//近接コンボ３

	void TransitionDamageState();		//ダメージ
	void TransitionDeadState();			//死亡


	//--------各ステートのアップデート--------//
	void UpdateIdleState(float elapsedTime);			//待機
	void UpdateMoveState(float elapsedTime);			//走り
	void UpdateWingState(float elapsedTime);			//飛行

	void UpdateBoostState(float elapsedTime);			//回避
	void UpdateJumpState(float elapsedTime);			//ジャンプ
	void UpdateLandingState(float elapsedTime);			//着地

	void UpdateShotState(float elapsedTime);			//射撃
	void UpdateComboState(float elapsedTime);			//近接コンボ

	void UpdateDamageState(float elapsedTime);			//ダメージ
	void UpdateDeadState(float elapsedTime);			//死亡


	//更新関数の関数ポインタの定義
	typedef void (PLAYER::* ActUpdate)(float elapsedTime);

	//移動ベクトルと速度設定
	void Move(float vx, float vz, float speed)override;
	void Move(float vx, float vy, float vz, float speed);

	//ブーストの更新処理
	void BoostUpdate(float elapsedTime);

	//軌跡更新処理
	void TrailUpdate();

	//プレイヤーの移動入力処理
	bool InputMove(float elapsedTime);

	//プレイヤーの飛行入力処理
	bool InputMoveWing(float elapsedTime);

	//制限付きの移動（攻撃中などの移動入力）
	bool InputMove(float elapsedTime, float restrictionMove, float restrictionTurn);
		
	//入力ベクトル算出
	const DirectX::XMFLOAT3 GetMoveVec(Camera* camera, bool wing = false) const;

	//ラジアルブラー
	void ShaderUpdate(float elapsedTime);

	//ジャンプ入力処理
	void InputJump();

	//回避入力
	void InputBoost();

	//飛行入力
	void InputWing();

	//射撃入力
	void InputShot();
	
	//先行入力チェック
	void CheckPreInput(COMBO combo);

	//着地したか
	void OnLanding()override;

	//死亡したときの処理
	void OnDead() override;

	//ダメージを受けた時の処理
	void OnDamaged(WINCE_TYPE type) override;

	//ダメージを受ける処理
	bool ApplyDamage(int damage, float invincible_time, WINCE_TYPE type)override;

	//垂直速力更新処理
	 void UpdateVerticalVelocity(float elapsed_frame)override;

	 //----------<ファイル>------------//
	 void LoadDataFile();
	 void SaveDataFile();
	 const char* filePath = "Resources/Character/PLAYER/player_param.json";

private:
	//--------------------変数--------------------------
	//関数ポインタの宣言
	ActUpdate pUpdate = &PLAYER::UpdateIdleState;

	//プレイヤーパラメーター
	PlayerParam param;

	//現在の状態
	STATE state;

	//それぞれのインスタンス保存用
	GamePad* gamePad;
	Mouse* mouse;
	Camera* camera;

	//SE
	enum class PLAYER_SE
	{
		SE_SABER = 0,		//サーベル音
		SE_LASER = 1,		//射撃音
		SE_BOOST =2,		//ブースト音
		SE_DAMAGE = 3,		//被弾音
	};
	std::shared_ptr<audio> audios[8];

	//UI
	std::unique_ptr<PlayerUI> ui;

	//モデル
	std::unique_ptr <gltf_model> model;

	//ラジアルブラー
	RadialBlur::radialBlurConstants player_RadialBlurConstant{}; 
	float radialTimer = 0.0f;	//ラジアルブラータイマー

	//色収差
	Glitch_CA::Glitch_CA_constants player_Glitch_CA_Constant{};
	float glitch_CATimer = 0.0f;//色収差タイマー
	bool isGlitch_CA = false;		//色収差オンオフ


	//ボス座標
	DirectX::XMFLOAT3 bossPosition{};

	//加速度状態
	enum class ACCELERATION_STATE
	{
		MOVE,					//走り
		BOOST,					//ブースト
		WING,					//飛行
		ACCELERATION_COUNT,		//総数
	};
	float accelerationState[ToInt(ACCELERATION_STATE::ACCELERATION_COUNT)]{
		1.5f,			//走り
		50.0f,			//ブースト
		25.0f			//飛行
	};

	//現在何回ジャンプしてるか
	int jumpCount = 0;
	//ジャンプ可能回数
	const int jumpLimit = 1;

	//ブーストのオンオフ
	bool isBoost = false;

	//斬撃エフェクト
	std::unique_ptr<Effect> slashEffect = nullptr;

	//Imguiのオンオフ
	bool displayPlayerImgui = false;

	//------------------攻撃関連--------------------------

	//攻撃に関する各種パラメータ構造体
	AttackParam attackParam;

	//左手右手
	enum class LR
	{
		LEFT,	//左手
		RIGHT,	//右手
		COUNT,	//要素の数（enumの終端）
	};
	//当たり判定ノード
	gltf_model::node beamSaber[ToInt(LR::COUNT)];	//サーベルの先端
	gltf_model::node lowerArm[ToInt(LR::COUNT)];	//腕の先端

	DirectX::XMFLOAT3 beamSaberPosition[ToInt(LR::COUNT)]{};		//サーベルの先端位置
	DirectX::XMFLOAT3 lowerArmPosition[ToInt(LR::COUNT)]{};			//腕の先端位置
	DirectX::XMFLOAT3 attackCollisionPosition[ToInt(LR::COUNT)]{};	//サーベルの当たり判定位置

	//軌跡
	enum class TRAIL
	{
		LOWER_ARM,	//腕の先端
		BEAM_SABER,	//サーベルの先端
		COUNT,		//要素の数（enumの終端）
	};
	//最大ポリゴン数
	static const int MAX_POLYGON = 12;

	//軌跡パラメータ
	struct TrailParam
	{
		DirectX::XMFLOAT3 trailPositions[ToInt(TRAIL::COUNT)][MAX_POLYGON];	//軌跡の保存座標配列
		DirectX::XMFLOAT4 color[MAX_POLYGON];	//各頂点ごとのカラー情報
	};
	//左右の攻撃の軌跡配列
	TrailParam trailAttack[ToInt(LR::COUNT)];
	//軌跡を初期化するかどうかのフラグ
	bool resetTrail = false;

	//現在のコンボ数
	COMBO nowCombo = COMBO::NONE;

	//攻撃時のフレームパラメータ
	struct AttackFlameParam
	{
		float startFlame = 0.0f;
		float endFlame = 0.0f;
		float preInputFlame = 0.0f;
	};
	AttackFlameParam attackFlameParam[ToInt(COMBO::COUNT)];
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
	
	//コンボ攻撃の時間定数
	//START	　：攻撃フラグ開始フレーム
	//END	　：攻撃フラグ終了フレーム
	//PREINPUT：先行入力があれば偏移開始するフレーム

	//コンボ01
	const float ATTACK01_START = 0.023f;
	const float ATTACK01_END = 0.15f;
	const float ATTACK01_PREINPUT = 0.173f;
	//コンボ02
	const float ATTACK02_START = 0.03f;
	const float ATTACK02_END = 0.175f;
	const float ATTACK02_PREINPUT = 0.2f;
	//コンボ03
	const float ATTACK03_START = 0.325f;
	const float ATTACK03_END = 0.65f;
	const float ATTACK03_PREINPUT = 1.0f;

	//着地ステートに偏移する速度
	const float LANDING_SPEED = 30.0f;

	//身体当たり判定の大きさ
	const float COLLIDER_RADIUS = 1.0f;
	//攻撃当たり判定の大きさ
	const float ATTACK_RADIUS = 1.5f;

	//ボスの初期座標
	const DirectX::XMFLOAT3 INIT_POSITION = { 0.0f, 37.0f, 0.0f };
	//プレイヤーモデルのスケール倍率
	const float PLAYER_SCALE = 2.0f;

	//PLAYER_WING_START中の重力影響率
	const float WING_GRAVITY_SCALE = 2.0f;

	const DirectX::XMFLOAT4 DEBUG_ATTACK_COLOR = { 1.0f,0.0f,0.0f,1.0f };	//デバッグ時の攻撃当たり判定の色
	const DirectX::XMFLOAT4 DEBUG_COLLIDER_COLOR = { 0.0f,1.0f,0.0f,1.0f };	//デバッグ時の身体当たり判定の色

	const float CHARGE_SPEED = 3.0f;			//ブーストの回復速度
	const float BOOST_MIN_THRESHOLD = 2.5f;		//ブーストのしきい値

	const float GLITCH_MAX_DURATION = 0.03f;	//グリッチ効果の最大持続時間
	const float GLITCH_SHIFT_AMOUNT = 0.015f;	//RGBシフトの移動量
	const float GLITCH_CENTER_X = 0.5f;			//色収差中心のX座標
	const float GLITCH_CENTER_Y = 0.5f;			//色収差中心のY座標
	
	//射撃音のボリューム
	const float SOUND_VOLUME_LASER = 0.3f;

	//サーベル音のボリューム
	const float SOUND_VOLUME_SABER = 1.0f;

	//被弾音のボリューム
	const float SOUND_VOLUME_DAMAGE = 0.5f;
};

#pragma once
#include <DirectXMath.h>
#include "damage_func.h"
#include "gltf_model.h"

class Character
{
public:
	//----------------------------------------------------------------
	// 構造体、列挙型
	//----------------------------------------------------------------
	struct CharacterParam
	{
		//キャラクターの半径
		float radius = 1.0f;
		//キャラの縦の大きさ
		float height = 2.6f;
		//最大体力
		int maxHealth = 1000;
		//摩擦力
		float friction = 1.0f;
		//空気抵抗
		float airControl = 0.3f;
		//加速度
		float acceleration = 1.5f;
		//最大速度
		float maxMoveSpeed = 30.0f;
		//回転速度
		float turnSpeed = DirectX::XMConvertToRadians(720);
		//移動速度
		float moveSpeed = 5.0f;

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(
				cereal::make_nvp("radius", radius),
				cereal::make_nvp("height", height),
				cereal::make_nvp("max_health", maxHealth),
				cereal::make_nvp("friction", friction),
				cereal::make_nvp("air_control", airControl),
				cereal::make_nvp("acceleration", acceleration),
				cereal::make_nvp("max_move_speed", maxMoveSpeed),
				cereal::make_nvp("turn_speed", turnSpeed),
				cereal::make_nvp("move_speed", moveSpeed)
			);
		}

	};

	Character() {}
	virtual ~Character() {}

	//---------------------------------------------------------------
	//セッターとゲッター
	//---------------------------------------------------------------

	//位置取得
	const DirectX::XMFLOAT3& GetPosition() const { return position; }
	//位置設定
	void SetPosition(const DirectX::XMFLOAT3& position) { this->position = position; }
	//胴体部分位置取得
	const DirectX::XMFLOAT3& GetTargetPosition() const { return { position.x, position.y + charaParam.height,position.z }; }
	// 回転取得
	const DirectX::XMFLOAT3& GetAngle() const { return angle; }
	//回転設定
	void SetAngle(const DirectX::XMFLOAT3& angle) { this->angle = angle; }
	// スケール取得
	const DirectX::XMFLOAT3& GetScale() const { return scale; }
	//スケール設定
	void SetScale(const DirectX::XMFLOAT3& scale) { this->scale = scale; }
	
	//velocity取得
	const DirectX::XMFLOAT3& GetVelocity() const { return velocity; }
	//velocityセット
	void SetVelocity(const DirectX::XMFLOAT3& v) { this->velocity = v; }
	//orientation取得
	const DirectX::XMFLOAT4& GetOrientation() const { return orientation; }
	// 半径
	float GetRadius() const { return charaParam.radius; }
	// HP
	int GetHealth() const { return health; }
	// 最大HP
	int GetMaxHealth() const { return charaParam.maxHealth; }
	//体の正面と進行方面との角度
	float GetTurnAngle() const { return turnAngle; }
	//HPパーセンテージ
	float GetHpPercent() const { return health <= 0 ? 0.0f : static_cast<float>(health) / static_cast<float>(charaParam.maxHealth); }
	// 地面判定
	bool GetIsGround() const { return isGround; }
	// 地面判定
	bool GetIsDead() const { return isDead; }
	//高さ取得
	float GetHeight() const { return charaParam.height; }
	//トランスフォームのゲッター
	const DirectX::XMFLOAT4X4& GetTransform() const { return transform; }
	
	//衝撃を与える
	void AddImpulse(const DirectX::XMFLOAT3& impulse);
	//ダメージを与える
	virtual bool ApplyDamage(int damage, float invincibleTime, WINCE_TYPE type);

protected:
	//移動
	virtual void Move(float vx, float vz, float speed);

	//回転処理（オイラー）
	void Turn(float elapsedTime, float vx, float vz, float speed);
	//回転処理（クォータニオン使用）
	void Turn(float elapsedTime, DirectX::XMFLOAT3 move_vec, float speed, DirectX::XMFLOAT4& orien);
	
	//ジャンプ処理
	void Jump(float speed);

	//速力処理更新
	void UpdateVelocity(float elapsedTime, DirectX::XMFLOAT3& position);
	
	//着地時に呼ばれる処理
	virtual void OnLanding() {}

	//死亡時に呼ばれる処理
	virtual void OnDead() {}

	//ダメージ時に呼ばれる処理
	virtual void OnDamaged(WINCE_TYPE type) {}

	//無敵タイマーの更新
	void UpdateInvicibleTimer(float elapsedTime);

	//-----------変数--------------//

	DirectX::XMFLOAT3	position = { 0, 0, 0 };		//座標
	DirectX::XMFLOAT3	angle = { 0, 0, 0 };		//角度
	DirectX::XMFLOAT3	scale = { 1, 1, 1 };		//サイズ
	DirectX::XMFLOAT4 orientation{ 0,0,0,1 };		//クォータニオンで表した回転
	DirectX::XMFLOAT4X4	transform = {				//ワールド行列
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
	};

	//キャラクターのパラメータ
	CharacterParam charaParam;

	//段差を乗り越えられる最大の高さ
	float stepOffset = 2.0f;

	//現在の速度ベクトル
	DirectX::XMFLOAT3 velocity = { 0, 0, 0 };

	//接地判定フラグ
	bool isGround = false;
	//死亡フラグ
	bool isDead = false;

	//坂を登るときの速度補正係数
	float slopeRate = 1.0f;

	//無敵時間
	float invincibleTimer = 0.0f;

	//移動方向ベクトル
	float moveVec_x = 0.0f;
	float moveVec_y = 0.0f;
	float moveVec_z = 0.0f;

	//体力
	int32_t health;

	//ブレンドアニメーション
	enum class ANIME_NODE
	{
		NOW_ANIMATION = 0,		//現在のアニメーション
		OLD_ANIMATION,			//前のアニメーション
		NODE_COUNT				//個数
	};

	//現在と前回のアニメーション情報
	std::vector<gltf_model::node> animatedNodes[ToInt(ANIME_NODE::NODE_COUNT)];
	
	//ブレンド後のアニメーション結果
	std::vector<gltf_model::node> blendedAnimatedNodes;

	//アニメーションの再遷移中かどうか
	bool transitionToTransition = false;

	//アニメーション再生時間
	float time = 0.0f;

	//アニメーションブレンド係数
	float factor = 0.0f;

	//アニメーション遷移にかける時間
	float transitionTime = 0.11f;

	//アニメーションの遷移状態
	enum class TRANSITION_STATE
	{
		NONE,			//遷移なし
		START,			//遷移開始
		TRANSITION		//ブレンド中
	}transitionState;

	//壁との押し戻し反力
	float vsWallRayPower = 5.0f;
	//坂の法線
	DirectX::XMFLOAT3 slopeNormal = {};

	//重力加速度
	float gravity = -1.0f;
	//体の向きを調整する回転角
	float turnAngle = 0.0f;

	//-----------変数--------------//
	//Y軸下の制限
	const float LIMIT_Y = -10.0f;

	//下に行き過ぎた時のリスポーンy座標
	const float RESPAWN_Y = 50.0f;

	//-----------プライベート関数--------------//
private:
	//垂直速力更新処理
	virtual void UpdateVerticalVelocity(float elapsed_frame);
	//垂直移動更新処理
	void UpdateVerticalMove(float elapsedTime, DirectX::XMFLOAT3& position);
	//水平速力更新処理
	void UpdateHorizontalVelocity(float elapsed_frame);
	//水平移動更新処理
	void UpdateHorizontalMove(float elapsedTime, DirectX::XMFLOAT3& position);

};


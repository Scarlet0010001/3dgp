#pragma once
#include <DirectXMath.h>
#include "damage_func.h"

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
	//高さ取得
	float GetHeight() const { return charaParam.height; }
	//トランスフォームのゲッター
	const DirectX::XMFLOAT4X4& GetTransform() const { return transform; }
	//衝撃を与える
	void AddImpulse(const DirectX::XMFLOAT3& impulse);
	//ダメージを与える
	virtual bool ApplyDamage(int damage, float invincibleTime, WINCE_TYPE type);

protected:
	void Move(float vx, float vz, float speed);
	void Turn(float elapsed_time, float vx, float vz, float speed);//オイラー
	void Turn(float elapsed_time, DirectX::XMFLOAT3 move_vec, float speed, DirectX::XMFLOAT4& orien);//クォータニオン
	//ジャンプ処理
	void Jump(float speed);
	//速力処理更新
	void UpdateVelocity(float elapsed_time, DirectX::XMFLOAT3& position);
	virtual void OnLanding() {}
	//死亡したときに呼ばれる
	virtual void OnDead() {}
	virtual void OnDamaged(WINCE_TYPE type) {}
	void UpdateInvicibleTimer(float elapsed_time);

	//-----------変数--------------//

	DirectX::XMFLOAT3	position = { 0, 0, 0 };
	DirectX::XMFLOAT3	angle = { 0, 0, 0 };
	DirectX::XMFLOAT3	scale = { 1, 1, 1 };
	DirectX::XMFLOAT4 orientation{ 0,0,0,1 };
	DirectX::XMFLOAT4X4	transform = {
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
	};

	CharacterParam charaParam;

	float stepOffset = 0.7f;
	DirectX::XMFLOAT3 velocity = { 0, 0, 0 };
	//地面に当たっているか
	bool isGround = false;

	float invincibleTimer = 0.0f;
	float moveVec_x = 0.0f;
	float moveVec_z = 0.0f;

	//体力
	int32_t health;


	float vs_wall_ray_power = 5.0f;
	//坂の法線
	DirectX::XMFLOAT3 slopeNormal = {};
	float gravity = -1.0f;
	float turnAngle;			//体を向けるときの回転角
	//-----------プライベート関数--------------//
private:
	//垂直速力更新処理
	void UpdateVerticalVelocity(float elapsed_frame);
	//垂直移動更新処理
	void UpdateVerticalMove(float elapsed_time, DirectX::XMFLOAT3& position);
	//水平速力更新処理
	void UpdateHorizontalVelocity(float elapsed_frame);
	//水平移動更新処理
	void UpdateHorizontalMove(float elapsed_time, DirectX::XMFLOAT3& position);

};


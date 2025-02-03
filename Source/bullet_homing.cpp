#include "bullet_homing.h"
#include "bullet_manager.h"
#include "user.h"

BulletHoming::BulletHoming(BulletManager* manager, BULLET_MASTER MasterType)
    :Bullet(manager)
{
    Graphics& graphics = Graphics::Instance();
    model = std::make_unique<gltf_model>(graphics.GetDevice().Get(),
        "Resources/Bullet/Bullet.glb");
    type = BULLET_TYPE::Missile;
    masterType = MasterType;
    //表示サイズを調整
    scale.x = scale.y = scale.z = 1.0f;

    BulletManager::Instance().Setting();

    animated_nodes = model->nodes;

}

void BulletHoming::Update(float elapsedTime)
{
    //寿命処理
    lifeTimer -= elapsedTime;
    if (lifeTimer <= 0.0f) Destroy();

    //移動
    float speed = this->speed * elapsedTime;

    Move(elapsedTime, speed);

    //オブジェクト行列を更新
    UpdateTransform();

    //モデル行列更新
    transform = Math::calc_world_matrix(scale, angle, position, Math::COORDINATE_SYSTEM::RHS_YUP);

}

void BulletHoming::Render(float elapsedTime)
{
    Graphics& graphics = Graphics::Instance();
    model->render(graphics.Get_DC().Get(), transform, animated_nodes);

    DrawDebugPrimitive();

}

void BulletHoming::Launch(const DirectX::XMFLOAT3& direction, const DirectX::XMFLOAT3& position,
    const DirectX::XMFLOAT3& target)
{
    this->direction = direction;
    this->position = position;
    this->target = target;
    
}

void BulletHoming::Move(float elapsedTime, float speed)
{
    DirectX::XMFLOAT3 velocity =
    {
        direction.x * speed,
        direction.y * speed,
        direction.z * speed
    };

        position.x += velocity.x;
        position.y += velocity.y;
        position.z += velocity.z;

        //旋回
        float turnSpeed = this->turnSpeed * elapsedTime;

		//ターゲットまでのベクトルを算出
		DirectX::XMVECTOR Position = DirectX::XMLoadFloat3(&position);
		DirectX::XMVECTOR Target = DirectX::XMLoadFloat3(&target);
		DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(Target, Position);

		//ゼロベクトルでないなら回転処理
		DirectX::XMVECTOR LengthSq = DirectX::XMVector3LengthSq(Vec);
		float lengthSq;
		DirectX::XMStoreFloat(&lengthSq, LengthSq);

		if (lengthSq > 0.0001f)
		{
			//ターゲットまでのベクトルを単位ベクトル化
			Vec = DirectX::XMVector3Normalize(Vec);

			//向いている方向ベクトルを算出
			DirectX::XMVECTOR Direction =
				DirectX::XMLoadFloat3(&direction);

			//前方方向ベクトルと
			//ターゲットまでのベクトルの内積(角度)を算出
			DirectX::XMVECTOR Dot =
				DirectX::XMVector3Dot(Direction, Vec);

			float dot;
			DirectX::XMStoreFloat(&dot, Dot);

			//２つの単位ベクトルの角度が小さいほど
			//1.0に近づくという性質を利用して回転速度を調整する
			float rot = 1.0f - dot;
			if (rot > turnSpeed)
			{
				rot = turnSpeed;
			}

			//回転角度があるなら回転処理をする
			if (fabsf(rot) > 0.0001f)
			{
				//回転軸を算出
				DirectX::XMVECTOR Axis =
					DirectX::XMVector3Cross(Direction, Vec);

				//回転軸と回転量から回転行列を算出
				DirectX::XMMATRIX Rotation =
					DirectX::XMMatrixRotationAxis(Axis, rot);

				//現在の行列を回転させる
				DirectX::XMMATRIX Transform =
					DirectX::XMLoadFloat4x4(&transform);
				Transform = DirectX::XMMatrixMultiply(
					Transform, Rotation);

				//回転後の前方方向を取り出し、単位ベクトル化する
				Direction = DirectX::XMVector3Normalize(Transform.r[2]);
				DirectX::XMStoreFloat3(&direction, Direction);

			}

		}

}

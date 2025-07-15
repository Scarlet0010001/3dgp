#include "bullet_straight.h"
#include "bullet_manager.h"
#include "User/user.h"

BulletStraight::BulletStraight(BulletManager* manager, BULLET_MASTER MasterType)
    :Bullet(manager)
{
    Graphics& graphics = Graphics::Instance();
    model = std::make_unique<gltf_model>(graphics.GetDevice().Get(),
        "Resources/Bullet/Bullet.glb");
    masterType = MasterType;

    if (masterType == BULLET_MASTER::PLAYER)
    {
        bulletEffect =
            std::make_unique<Effect>("Resources/Effect/Bullet/playerBullet.efkefc");
    }
    else
    {
        bulletEffect =
            std::make_unique<Effect>("Resources/Effect/Bullet/bossBullet.efkefc");
    }

    type = BULLET_TYPE::STRAIGHT;
    //表示サイズを調整
    scale.x = scale.y = scale.z = 1.0f;

    BulletManager::Instance().Setting();

    animatedNodes = model->nodes;
}

void BulletStraight::Update(float elapsedTime)
{
    //寿命処理
    lifeTimer -= elapsedTime;
    if (lifeTimer <= 0.0f) Destroy();
    //移動
    float speed = this->speed * elapsedTime;

    bulletEffect->SetPosition(handle, position);
    Move(elapsedTime, speed);

    //オブジェクト行列を更新
    UpdateTransform();

    //モデル行列更新
    transform = Math::CalcWorldMatrix(scale, angle, position, Math::COORDINATE_SYSTEM::RHS_YUP);
}

void BulletStraight::Render(float elapsedTime)
{
    Graphics& graphics = Graphics::Instance();
    //model->render(graphics.Get_DC().Get(), transform, animated_nodes);
    
    DrawDebugPrimitive();
}

void BulletStraight::Launch(const DirectX::XMFLOAT3& direction, const DirectX::XMFLOAT3& position)
{
    this->direction = direction;
    this->position = position;
    handle = bulletEffect->Play(position, 3.0f);

}

void BulletStraight::Move(float elapsedTime, float speed)
{
    DirectX::XMFLOAT3 velocity =
    {
        direction.x * speed,
        direction.y * speed,
        direction.z * speed
    };

    //壁との当たり判定
#if 0
    float velocityLengthXZ = sqrtf(
        velocity.x * velocity.x + velocity.z * velocity.z);
    if (velocityLengthXZ > 0.0f)
    {
        //水平移動値
        float mx = velocity.x;
        float mz = velocity.z;

        //レイの開始位置と終点位置
        DirectX::XMFLOAT3 start =
        { position.x, position.y, position.z };
        DirectX::XMFLOAT3 end =
        { position.x + mx, position.y, position.z + mz };

        //レイキャストによる壁判定
        HitResult hit;
        if (StageManager::Instance().RayCast(start, end, hit, ATTACK_TYPE::SHOT, masterType))
        {
            if (hit.type == STAGE_TYPE::BREAK)
            {
                life = 0;
            }
            else
            {
                //壁までのベクトル
                DirectX::XMVECTOR Start =
                    DirectX::XMLoadFloat3(&start);
                DirectX::XMVECTOR End =
                    DirectX::XMLoadFloat3(&end);
                DirectX::XMVECTOR Vec =
                    DirectX::XMVectorSubtract(End, Start);

                //壁の法線
                DirectX::XMVECTOR Normal =
                    DirectX::XMLoadFloat3(&hit.normal);

                //入射ベクトルを法線に射影
                DirectX::XMVECTOR Dot =
                    DirectX::XMVector3Dot(Vec, Normal);

                //射影ベクトルの計算
                DirectX::XMVECTOR Projection =
                    DirectX::XMVectorMultiply(Normal, Dot);

                //補正位置の計算
                DirectX::XMVECTOR ReVec =
                    DirectX::XMVectorSubtract(Vec, Projection);

                //補正された終点位置
                DirectX::XMVECTOR RePosEnd =
                    DirectX::XMVectorAdd(Start, ReVec);


                // 反射ベクトルの計算
                DirectX::XMVECTOR Reflection =
                    DirectX::XMVector3Reflect(
                        DirectX::XMLoadFloat3(&direction),
                        Normal);

                //方向ベクトル
                DirectX::XMVECTOR ReVecN =
                    DirectX::XMVector3Normalize(Reflection);

                DirectX::XMStoreFloat3(&direction, ReVecN);

                //補正された終点位置の座標を取得
                DirectX::XMFLOAT3 RePosition;
                DirectX::XMStoreFloat3(&RePosition, RePosEnd);

                //補正された終点位置を新しい位置とする
                position.x = RePosition.x;
                position.y = RePosition.y; // 床の上に位置するように調整
                position.z = RePosition.z;

                life -= 1;
                reflection->Play(position);
            }
        }
#endif

    position.x += velocity.x;
    position.y += velocity.y;
    position.z += velocity.z;

}

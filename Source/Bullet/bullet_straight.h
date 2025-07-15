#pragma once
#include "bullet.h"
#include "effect.h"

class BulletStraight :
    public Bullet
{
public:
    BulletStraight(BulletManager* manager, BULLET_MASTER MasterType);
    ~BulletStraight() 
    {
        bulletEffect->Stop(handle);
    };

    //更新処理
    void Update(float elapsedTime)override;

    //描画処理
    void Render(float elapsedTime)override;

    //発射
    void Launch(const DirectX::XMFLOAT3& direction, const DirectX::XMFLOAT3& position);

private:
    //挙動
    void Move(float elapsedTime, float speed);

private:
    //エフェクト
    std::unique_ptr<Effect> bulletEffect = nullptr;
    Effekseer::Handle handle;
    std::vector<gltf_model::node> animatedNodes{};

};


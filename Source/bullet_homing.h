#pragma once
#include "bullet.h"

class BulletHoming :
    public Bullet
{
public:
    BulletHoming(BulletManager* manager, BULLET_MASTER MasterType);
    ~BulletHoming() {};

    //XVˆ—
    void Update(float elapsedTime)override;

    //•`‰æˆ—
    void Render(float elapsedTime)override;

    //”­Ë
    void Launch(
        const DirectX::XMFLOAT3& direction,
        const DirectX::XMFLOAT3& position,
        const DirectX::XMFLOAT3& target);

private:
    //‹““®
    void Move(float elapsedTime, float speed);

private:
    std::vector<gltf_model::node> animated_nodes{};

};


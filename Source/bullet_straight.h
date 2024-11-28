#pragma once
#include "bullet.h"
#include "gltf_model.h"

class BulletStraight :
    public Bullet
{
public:
    BulletStraight(BulletManager* manager, BULLET_MASTER MasterType);
    ~BulletStraight() {};

    //XVˆ—
    void Update(float elapsedTime)override;

    //•`‰æˆ—
    void Render(float elapsedTime)override;

    //”­Ë
    void Launch(const DirectX::XMFLOAT3& direction, const DirectX::XMFLOAT3& position);

private:
    //‹““®
    void Move(float elapsedTime, float speed);

private:
    std::vector<gltf_model::node> animated_nodes{};

};


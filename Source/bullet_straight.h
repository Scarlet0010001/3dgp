#pragma once
#include "bullet.h"
#include "gltf_model.h"

class BulletStraight :
    public Bullet
{
public:
    BulletStraight(BulletManager* manager, int MasterType);
    ~BulletStraight() {};

    //XVˆ—
    void Update(float elapsedTime)override;

    //•`‰æˆ—
    void Render(float elapsedTime)override;

    //”­Ë
    void Launch(const DirectX::XMFLOAT3& direction, const DirectX::XMFLOAT3& position);

protected:
    //‹““®
    void Move(float elapsedTime, float speed);
private:
    std::unique_ptr<gltf_model> model = nullptr;
    float speed = 50.0f;
    float lifeTimer = 3.0f;

    std::vector<gltf_model::node> animated_nodes{};

};


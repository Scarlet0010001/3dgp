#pragma once
#include "graphics.h"

//前方宣言
class BulletManager;


class Bullet
{
public:
    enum class BULLET_TYPE
    {
        TYPE_NONE = -1,
        STRAIGHT = 0,
        MISSILE,
    };
    enum class BULLET_MASTER
    {
        MASTER_NONE = -1,
        PLAYER = 0,
        ENEMY,
        COUNT,
    };

    Bullet(BulletManager* manager);
    virtual ~Bullet() {}

    //更新処理
    virtual void Update(float elapsedTime) = 0;

    //描画処理
    virtual void Render(float elapsedTime) = 0;

    //デバッグプリミティブ描画
    virtual void DrawDebugPrimitive();

    //破棄
    void Destroy();

    //位置取得
    const DirectX::XMFLOAT3& GetPosition() const { return position; }
    //方向取得
    const DirectX::XMFLOAT3& GetDirection() const { return direction; }
    //スケール取得
    const DirectX::XMFLOAT3& GetScale() const { return scale; }
    //半径取得
    float GetRadius()const { return radius; }
    //親取得
    BULLET_MASTER GetMasterType()const { return masterType; }
    //タイプ取得
    BULLET_TYPE GetType()const { return type; }

    //スケール設定
    void SetScale(const DirectX::XMFLOAT3& s)  { scale = s; }
    //速度設定
    void SetSpeed(const float& s)  { speed = s; }
    //生存時間設定
    void SetLifeTime(const float& s)  { lifeTimer = s; }
    //スケール設定
    void SetRadius(const float& r)  { radius = r; }
    //生存時間設定
    void SetTarget(const DirectX::XMFLOAT3& t)  { target = t; }
    //スケール設定
    void SetTurnSpeed(const float& t)  { turnSpeed = t; }


protected:
    //行列更新処理
    void UpdateTransform();

public:

protected:
    BulletManager* manager = nullptr;

    DirectX::XMFLOAT3 position = { 0,0,0 };
    DirectX::XMFLOAT3 angle{};//おいてるだけ
    DirectX::XMFLOAT3 scale = { 1,1,1 };

    DirectX::XMFLOAT3 direction = { 0,0,1 };
    
    BULLET_TYPE type = BULLET_TYPE::TYPE_NONE;
    BULLET_MASTER masterType = BULLET_MASTER::MASTER_NONE;

    std::unique_ptr<gltf_model> model = nullptr;
    float speed = 100.0f;
    float lifeTimer = 3.0f;
    float radius = 1.0f;
    DirectX::XMFLOAT3 target{};
    float turnSpeed = DirectX::XMConvertToRadians(180);

    DirectX::XMFLOAT4X4 transform = {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };

};
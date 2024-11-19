#pragma once
#include "graphics.h"

//前方宣言
class BulletManager;

class Bullet
{
public:
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
    //
    float GetRadius()const { return radius; }

    enum BULLET_TYPE
    {
        Straight = 0,
        Missile,
    };
    enum BULLET_MASTER
    {
        Player = 0,
        Enemy,
    };
protected:
    //行列更新処理
    void UpdateTransform();

public:
    int type = -1;

protected:
    BulletManager* manager = nullptr;

    DirectX::XMFLOAT3 position = { 0,0,0 };
    DirectX::XMFLOAT3 angle{};//おいてるだけ
    DirectX::XMFLOAT3 scale = { 1,1,1 };

    DirectX::XMFLOAT3 direction = { 0,0,1 };

    int masterType = 0;

    float radius = 1.0f;

    int life = 0;

    DirectX::XMFLOAT4X4 transform = {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };

};


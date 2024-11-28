#pragma once
#include <vector>
#include "bullet.h"
#include <set>

class BulletManager
{
public:
    BulletManager();
    ~BulletManager();

    static BulletManager& Instance()
    {
        static BulletManager instance;
        return instance;
    }

    //XVˆ—
    void Update(float elapsedTime);

    //•`‰æˆ—
    void Render(float elapsedTime);

    //ƒfƒoƒbƒOƒvƒŠƒ~ƒeƒBƒu•`‰æ
    void DrawDebugPrimitive();

    //ƒfƒoƒbƒOGUI•`‰æ
    void DebugGUI();

    //’eŠÛ“o˜^
    void Register(Bullet* bullet);

    //’eŠÛİ’è
    void Setting();

    //’eŠÛíœ
    void Remove(Bullet* bullet);

    //’eŠÛ‘Síœ
    void Clear();

    //’eŠÛ”æ“¾
    int GetBulletCount()const
    {
        return static_cast<int>(bullets.size());
    }

    //’eŠÛæ“¾
    Bullet* GetBullet(int index)
    {
        return bullets.at(index);
    }


private:
    //’eŠÛ‚Æ’eŠÛ‚Æ‚ÌÕ“Ëˆ—
    void CollisionBulletVsBullet();

    //Effect* miniexplosion = nullptr;
    std::vector<Bullet*> bullets;

    std::set<Bullet*> removes;

    struct BulletParam
    {
        DirectX::XMFLOAT3 scale = { 1,1,1 };
        float speed = 100.0f;
        float lifeTimer = 3.0f;
        float radius = 1.0f;
        
        DirectX::XMFLOAT3 target = { 0,0,0 };
        float turnSpeed = DirectX::XMConvertToRadians(180);
    };
    BulletParam P_param;
    BulletParam E_param;

    Bullet* setting{};

    //--------------ImGui--------------//
    bool displayBulletImgui = false;
    
};

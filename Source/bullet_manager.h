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

    //’eŠÛ“o˜^
    void Register(Bullet* bullet);

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
};

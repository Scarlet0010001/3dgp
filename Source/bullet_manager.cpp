#include "bullet_manager.h"

BulletManager::BulletManager()
{
}

BulletManager::~BulletManager()
{
    Clear();
}

void BulletManager::Update(float elapsedTime)
{
    //更新処理
    for (Bullet* bullet : bullets)
    {
        bullet->Update(elapsedTime);
    }

    //破棄処理
    for (Bullet* bullet : removes)
    {
        //std::vectorから要素を削除する場合は
        //イテレータで削除しなければならない
        std::vector<Bullet*>::iterator it =
            std::find(bullets.begin(),
                bullets.end(), bullet);

        if (it != bullets.end())
        {
            bullets.erase(it);
        }

        //弾丸の破棄処理
        delete bullet;
    }
    //破棄リストをクリア
    removes.clear();

    //弾丸同士の衝突処理
    CollisionBulletVsBullet();

}

void BulletManager::Render(float elapsedTime)
{
    for (Bullet* bullet : bullets)
    {
        bullet->Render(elapsedTime);
    }
}

void BulletManager::DrawDebugPrimitive()
{
    for (Bullet* bullet : bullets)
    {
        bullet->DrawDebugPrimitive();
    }

}

void BulletManager::Register(Bullet* bullet)
{
    bullets.emplace_back(bullet);
}

void BulletManager::Remove(Bullet* bullet)
{
    //破棄リストに追加
    removes.insert(bullet);
}

void BulletManager::Clear()
{
    for (Bullet* bullet : bullets)delete bullet;
    bullets.clear();
}

void BulletManager::CollisionBulletVsBullet()
{
    /*
    size_t projectileCount = projectiles.size();
    for (int i = 0; i < projectileCount; i++)
    {
        Projectile* projectileA = projectiles.at(i);
        for (int j = 1 + i; j < projectileCount; j++)
        {
            Projectile* projectileB = projectiles.at(j);
            DirectX::XMFLOAT3 outPosition;
            if (Collision::IntersectSphereVsSphere(
                projectileA->GetPosition(),
                projectileA->GetRadius(),
                projectileB->GetPosition(),
                projectileB->GetRadius(),
                outPosition))
            {
                miniexplosion->Play(projectileA->GetPosition());
                projectileA->Destroy();
                projectileB->Destroy();
            }
        }
    }
    */

}

#include "bullet_manager.h"
#include "user.h"

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
    DebugGUI();
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
    setting = bullet;
    bullets.emplace_back(bullet);
}

void BulletManager::Setting()
{
    if (setting->GetMasterType() == Bullet::BULLET_MASTER::Player)
    {
        setting->SetScale(P_param.scale);
        setting->SetSpeed(P_param.speed);
        setting->SetLifeTime(P_param.lifeTimer);
        setting->SetRadius(P_param.radius);
        setting->SetTarget(P_param.target);
        setting->SetTurnSpeed(P_param.turnSpeed);
    }
    else
    {
        setting->SetScale(E_param.scale);
        setting->SetSpeed(E_param.speed);
        setting->SetLifeTime(E_param.lifeTimer);
        setting->SetRadius(E_param.radius);
        setting->SetTarget(E_param.target);
        setting->SetTurnSpeed(E_param.turnSpeed);
    }
    setting = nullptr;
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
    
    //size_t bulletCount = bullets.size();
    //for (int i = 0; i < bulletCount; i++)
    //{
    //    Projectile* projectileA = bullets.at(i);
    //    for (int j = 1 + i; j < bulletCount; j++)
    //    {
    //        Projectile* projectileB = projectiles.at(j);
    //        DirectX::XMFLOAT3 outPosition;
    //        if (Collision::IntersectSphereVsSphere(
    //            projectileA->GetPosition(),
    //            projectileA->GetRadius(),
    //            projectileB->GetPosition(),
    //            projectileB->GetRadius(),
    //            outPosition))
    //        {
    //            //miniexplosion->Play(projectileA->GetPosition());
    //            projectileA->Destroy();
    //            projectileB->Destroy();
    //        }
    //    }
    //}
    
}

void BulletManager::DebugGUI()
{
#ifdef USE_IMGUI
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
    imguiMenuBar("Bullet", "Bullet", displayBulletImgui);

    if (displayBulletImgui)
    {
        if (ImGui::Begin("Player", nullptr, ImGuiWindowFlags_None))
        {
            if (ImGui::CollapsingHeader("Param", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::DragFloat3("Scale", &P_param.scale.x);
                ImGui::DragFloat("speed:", &P_param.speed);
                ImGui::DragFloat("lifeTimer:", &P_param.lifeTimer);
                ImGui::DragFloat("radius:", &P_param.radius);
                ImGui::DragFloat3("target:", &P_param.target.x);
                ImGui::DragFloat("turnSpeed:", &P_param.turnSpeed);
            }
        }
        ImGui::End();

        if (ImGui::Begin("Enemy", nullptr, ImGuiWindowFlags_None))
        {
            if (ImGui::CollapsingHeader("Param", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::DragFloat3("Scale", &E_param.scale.x);
                ImGui::DragFloat("speed:", &E_param.speed);
                ImGui::DragFloat("lifeTimer:", &E_param.lifeTimer);
                ImGui::DragFloat("radius:", &E_param.radius);
                ImGui::DragFloat3("target:", &E_param.target.x);
                ImGui::DragFloat("turnSpeed:", &E_param.turnSpeed);
            }
        }
        ImGui::End();

    }

#endif // USE_IMGUI
}

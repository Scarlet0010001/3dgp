#include "bullet_manager.h"
#include "collision.h"
#include "camera.h"
#include "user.h"

#include <filesystem>
#include <fstream>
#include <cereal/archives/json.hpp>

BulletManager::BulletManager()
{
}

BulletManager::~BulletManager()
{
    Clear();
}

void BulletManager::Initialize()
{
    //パラメーターロード
    LoadDataFile(Bullet::BULLET_MASTER::PLAYER);
    LoadDataFile(Bullet::BULLET_MASTER::ENEMY);

    hitEffect = std::make_unique<Effect>("Resources/Effect/Hit/hit.efkefc");
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
    setting = bullet;
    bullets.emplace_back(bullet);
}

void BulletManager::Setting()
{
    if (setting->GetMasterType() == Bullet::BULLET_MASTER::PLAYER)
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

void BulletManager::CollisionBullet(PLAYER* player, Boss*boss)
{
    //インスタンス取得
    Camera& camera = Camera::Instance();


    size_t bulletCount = bullets.size();
    for (int i = 0; i < bulletCount; i++)
    {
        Bullet* bulletA = bullets.at(i);
        DirectX::XMFLOAT3 outPosition;

        for (int j = 1 + i; j < bulletCount; j++)
        {
            Bullet* bulletB = bullets.at(j);
            if (Collision::SphereVsSphere(
                bulletA->GetPosition(),
                bulletA->GetRadius(),
                bulletB->GetPosition(),
                bulletB->GetRadius()))
            {
                //弾をどちらも削除
                bulletA->Destroy();
                bulletB->Destroy();
            }
        }

        //プレイヤーとの当たり判定
        if (bulletA->GetMasterType() == Bullet::BULLET_MASTER::ENEMY &&
            Collision::SphereVsCylinder(
                bulletA->GetPosition(),
                bulletA->GetRadius(),
                player->collider.start,
                player->collider.radius,
                player->GetHeight()))
        {
            AttackParam attackParam = E_param.attackParam;
            //攻撃対象に与えるダメージ量と無敵時間
            if (player->damagedFunction(attackParam.power, attackParam.invinsibleTime, WINCE_TYPE::SMALL))
            {
                //カメラシェイク
                camera.SetCameraShake(attackParam.cameraShake);

                //ヒットストップ
                camera.SetHitStop(attackParam.hitStop);
            }
            bulletA->Destroy();
        }
        //ボスとの当たり判定
        if (bulletA->GetMasterType() == Bullet::BULLET_MASTER::PLAYER &&
            Collision::SphereVsCylinder(
                bulletA->GetPosition(),
                bulletA->GetRadius(),
                boss->GetBodyCollision().capsule.start,
                boss->GetBodyCollision().capsule.radius,
                boss->GetBodyCollision().height))
        {
            AttackParam attackParam = P_param.attackParam;

            //攻撃対象に与えるダメージ量と無敵時間
            if (boss->damagedFunction(attackParam.power, attackParam.invinsibleTime, WINCE_TYPE::SMALL))
            {
                //カメラシェイク
                camera.SetCameraShake(attackParam.cameraShake);

                //ヒットストップ
                camera.SetHitStop(attackParam.hitStop);

                //ヒットエフェクト再生
                hitEffect->Play(bulletA->GetPosition());

            }
            bulletA->Destroy();

        }

    }
}

void BulletManager::LoadDataFile(Bullet::BULLET_MASTER type)
{
    // JSONファイルのパスを設定（拡張子を .json に変更）
    std::filesystem::path path = filePath[ToInt(type)];
    path.replace_extension(".json");

    // ファイルが存在するかチェック
    if (std::filesystem::exists(path.c_str()))
    {
        std::ifstream ifs;
        ifs.open(path);

        // ファイルが正常に開けた場合、JSONからデータを読み込む
        if (ifs)
        {
            cereal::JSONInputArchive o_archive(ifs);

            // 弾丸パラメータをデシリアライズ
            if (type == Bullet::BULLET_MASTER::PLAYER)
            {
                o_archive(P_param);
            }
            else if (type == Bullet::BULLET_MASTER::ENEMY)
            {
                o_archive(E_param);
            }
        }
    }
}

void BulletManager::SaveDataFile(Bullet::BULLET_MASTER type)
{
    // JSONファイルのパスを設定（拡張子を .json に変更）
    std::filesystem::path path = filePath[ToInt(type)];
    path.replace_extension(".json");

    // JSONファイルを書き込み用に開く
    std::ofstream ifs;
    ifs.open(path);

    // ファイルが正常に開けた場合、データを保存
    if (ifs)
    {
        cereal::JSONOutputArchive o_archive(ifs);

        // 各弾丸パラメータをデシリアライズ
        if (type == Bullet::BULLET_MASTER::PLAYER)
        {
            o_archive(P_param);
        }
        else if (type == Bullet::BULLET_MASTER::ENEMY)
        {
            o_archive(E_param);
        }
    }
}

void BulletManager::DebugGUI()
{
#ifdef USE_IMGUI
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
    ImguiMenuBar("Bullet", "PLAYER", displayPlayerImgui);

    if (displayPlayerImgui)
    {
        if (ImGui::Begin("PlayerBullet", nullptr, ImGuiWindowFlags_None))
        {
            if (ImGui::CollapsingHeader("Param", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::DragFloat3("Scale", &P_param.scale.x);
                ImGui::DragFloat("speed:", &P_param.speed);
                ImGui::DragFloat("lifeTimer:", &P_param.lifeTimer);
                ImGui::DragFloat("radius:", &P_param.radius);
                ImGui::DragFloat3("target:", &P_param.target.x);
                ImGui::DragFloat("turnSpeed:", &P_param.turnSpeed);

                if (ImGui::CollapsingHeader("attack_param"))
                {
                    ImGui::DragInt("power", &P_param.attackParam.power, 0.1f);
                    ImGui::DragFloat("invinsible_time", &P_param.attackParam.invinsibleTime, 0.1f);

                    ImGui::Text("camera_shake");
                    ImGui::DragFloat("shake_x", &P_param.attackParam.cameraShake.max_X_shake, 0.1f);
                    ImGui::DragFloat("shake_y", &P_param.attackParam.cameraShake.max_Y_shake, 0.1f);
                    ImGui::DragFloat("time", &P_param.attackParam.cameraShake.time, 0.1f);
                    ImGui::DragFloat("smmoth", &P_param.attackParam.cameraShake.shakeSmoothness, 0.1f, 0.1f, 1.0f);

                    ImGui::Text("hit_stop");
                    ImGui::DragFloat("stop_time", &P_param.attackParam.hitStop.time, 0.1f);
                }
            }
            if (ImGui::Button("load"))
            {
                LoadDataFile(Bullet::BULLET_MASTER::PLAYER);
            }
            ImGui::Separator();
            if (ImGui::Button("save"))
            {
                SaveDataFile(Bullet::BULLET_MASTER::PLAYER);
            }
        }
        ImGui::End();
    }
    ImguiMenuBar("Bullet", "ENEMY", displayBossImgui);
    if (displayBossImgui)
    {
        if (ImGui::Begin("EnemyBullet", nullptr, ImGuiWindowFlags_None))
        {
            if (ImGui::CollapsingHeader("Param", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::DragFloat3("Scale", &E_param.scale.x);
                ImGui::DragFloat("speed:", &E_param.speed);
                ImGui::DragFloat("lifeTimer:", &E_param.lifeTimer);
                ImGui::DragFloat("radius:", &E_param.radius);
                ImGui::DragFloat3("target:", &E_param.target.x);
                ImGui::DragFloat("turnSpeed:", &E_param.turnSpeed);

                if (ImGui::CollapsingHeader("attack_param"))
                {
                    ImGui::DragInt("power", &E_param.attackParam.power, 0.1f);
                    ImGui::DragFloat("invinsible_time", &E_param.attackParam.invinsibleTime, 0.1f);

                    ImGui::Text("camera_shake");
                    ImGui::DragFloat("shake_x", &E_param.attackParam.cameraShake.max_X_shake, 0.1f);
                    ImGui::DragFloat("shake_y", &E_param.attackParam.cameraShake.max_Y_shake, 0.1f);
                    ImGui::DragFloat("time", &E_param.attackParam.cameraShake.time, 0.1f);
                    ImGui::DragFloat("smmoth", &E_param.attackParam.cameraShake.shakeSmoothness, 0.1f, 0.1f, 1.0f);

                    ImGui::Text("hit_stop");
                    ImGui::DragFloat("stop_time", &E_param.attackParam.hitStop.time, 0.1f);
                }

            }
        }
        if (ImGui::Button("load"))
        {
            LoadDataFile(Bullet::BULLET_MASTER::ENEMY);
        }
        ImGui::Separator();
        if (ImGui::Button("save"))
        {
            SaveDataFile(Bullet::BULLET_MASTER::ENEMY);
        }
        ImGui::End();

    }

#endif // USE_IMGUI
}

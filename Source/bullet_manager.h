#pragma once
#include <vector>
#include "bullet.h"
#include "player.h"
#include "Boss.h"
#include <set>
#include <cereal/cereal.hpp>

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

    //更新処理
    void Update(float elapsedTime);

    //描画処理
    void Render(float elapsedTime);

    //デバッグプリミティブ描画
    void DrawDebugPrimitive();

    //デバッグGUI描画
    void DebugGUI();

    //弾丸登録
    void Register(Bullet* bullet);

    //弾丸設定
    void Setting();

    //弾丸削除
    void Remove(Bullet* bullet);

    //弾丸全削除
    void Clear();

    //弾丸数取得
    int GetBulletCount()const
    {
        return static_cast<int>(bullets.size());
    }

    //弾丸取得
    Bullet* GetBullet(int index)
    {
        return bullets.at(index);
    }

    //弾丸と弾丸との衝突処理
    void CollisionBullet(Player* player, Boss* boss);

private:

    //データファイル
    void LoadDataFile();
    void SaveDataFile();
    const char* filePath = "Resources/Bullet/bullet_param.json";

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
        
        AttackParam attackParam;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                cereal::make_nvp("scale", scale),
                cereal::make_nvp("speed", speed),
                cereal::make_nvp("lifeTimer", lifeTimer),
                cereal::make_nvp("radius", radius),
                cereal::make_nvp("target", target),
                cereal::make_nvp("turnSpeed", turnSpeed),
                cereal::make_nvp("attackParam", attackParam)
            );
        }

    };
    BulletParam P_param;
    BulletParam E_param;


    Bullet* setting{};

    //--------------ImGui--------------//
    bool displayPlayerImgui = false;
    bool displayBossImgui = false;
    
};
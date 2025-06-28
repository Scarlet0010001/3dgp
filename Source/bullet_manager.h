#pragma once
#include <vector>
#include "bullet.h"
#include "player.h"
#include "Boss.h"

#include "effect.h"

#include <set>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/unordered_map.hpp>

#include <unordered_map>

namespace DirectX
{
    template<class T>
    void serialize(T& archive, DirectX::XMFLOAT2& v)
    {
        archive(
            cereal::make_nvp("x", v.x),
            cereal::make_nvp("y", v.y));
    }

    template<class T>
    void serialize(T& archive, DirectX::XMFLOAT3& v)
    {
        archive(
            cereal::make_nvp("x", v.x),
            cereal::make_nvp("y", v.y),
            cereal::make_nvp("z", v.z)
        );
    }

    template<class T>
    void serialize(T& archive, DirectX::XMFLOAT4& v)
    {
        archive(cereal::make_nvp("x", v.x),
            cereal::make_nvp("y", v.y),
            cereal::make_nvp("z", v.z),
            cereal::make_nvp("w", v.w)
        );
    }

    template<class T>
    void serialize(T& archive, DirectX::XMFLOAT4X4& m)
    {
        archive(
            cereal::make_nvp("_11", m._11), cereal::make_nvp("_12", m._12),
            cereal::make_nvp("_13", m._13), cereal::make_nvp("_14", m._14),
            cereal::make_nvp("_21", m._21), cereal::make_nvp("_22", m._22),
            cereal::make_nvp("_23", m._23), cereal::make_nvp("_24", m._24),
            cereal::make_nvp("_31", m._31), cereal::make_nvp("_32", m._32),
            cereal::make_nvp("_33", m._33), cereal::make_nvp("_34", m._34),
            cereal::make_nvp("_41", m._41), cereal::make_nvp("_42", m._42),
            cereal::make_nvp("_43", m._43), cereal::make_nvp("_44", m._44)
        );
    }
};

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
    void Initialize();
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
    void CollisionBullet(PLAYER* player, Boss* boss);

private:

    //データファイルのセーブとロード
    //typeは誰が撃ったか
    void LoadDataFile(Bullet::BULLET_MASTER type);
    void SaveDataFile(Bullet::BULLET_MASTER type);

    //保存するjsonファイルの名前
    const char* filePath[ToInt(Bullet::BULLET_MASTER::COUNT)]{
        "Resources/Bullet/playerBullet_param.json",
        "Resources/Bullet/bossBullet_param.json"
    };

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

    //ヒットエフェクト
    std::unique_ptr<Effect> hitEffect = nullptr;

    //--------------ImGui--------------//
    bool displayPlayerImgui = false;
    bool displayBossImgui = false;
    
};
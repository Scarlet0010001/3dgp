#pragma once
#include "scene.h"
#include "camera.h"
#include "player.h"

#include "light_manager.h"
#include "deferred_renderer.h"

#include "sky_map.h"

class SceneGame :
    public Scene
{
public:
    SceneGame();
    ~SceneGame()override {}
    //シーン初期化
    void Initialize() override;
    //シーン終了処理
    void Finalize() override;
    //シーンアップデート
    void Update(float elapsedTime) override;
    //シーン描画
    void Render(float elapsedTime) override;

    //クリ時の更新
    //void clear_update(float elapsedTime);

    //デバッグ描画
    void DebugGui();

    //シーンリセット
    void SceneReset();

protected:
    struct SCENE_CONSTANTS
    {
        //DirectX::XMFLOAT4X4 view_projection; //ビュー・プロジェクション変換行列
        //DirectX::XMFLOAT4 light_direction; //ライトの向き
        //DirectX::XMFLOAT4 camera_position;
    };
    std::unique_ptr<Constants<SCENE_CONSTANTS>> scene_constants{};


private:

    //カメラ
    Camera* camera = nullptr;
    //プレイヤー
    std::unique_ptr<Player> player = nullptr;
    //キャラ初期位置
    DirectX::XMFLOAT3 charaPos{};
    //平行光
    std::shared_ptr<DirLight> dirLight = nullptr;
    //ディファードレンダー
    std::unique_ptr<DeferredRenderer> deferred = nullptr;
    /*
    //ボス
    std::unique_ptr<Boss> boss = nullptr;
    //スカイボックス
    std::unique_ptr<SkyBox> skybox = nullptr;
    //操作説明UI（仮）
    std::unique_ptr<SpriteBatch> operation_ui = nullptr;

    std::unique_ptr<Tutorial> tutorial = nullptr;
    */
    //スカイマップ
    std::unique_ptr<SkyMap> skymap;

    std::unique_ptr<framebuffer> framebuffers[8];

    std::unique_ptr<fullscreen_quad> bit_block_transfer;

    //タイトルに戻る　※テスト用
    bool displayImgui = false;

};


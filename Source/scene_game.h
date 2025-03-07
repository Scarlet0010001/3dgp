#pragma once
#include "scene.h"
#include "camera.h"
#include "player.h"
#include "boss.h"

#include "light_manager.h"
#include "deferred_renderer.h"

#include "audio.h"
#include "sky_map.h"
#include "radial_blur.h"
#include "glitch_chromatic_aberration.h"


class SceneGame :
    public Scene
{
public:
    //コンストラクタ
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

    //キャラクターの当たり判定
    void JudgeCollision();

private:

    //カメラ
    Camera* camera = nullptr;
    //プレイヤー
    std::unique_ptr<Player> player = nullptr;
    //ボス
    std::unique_ptr<Boss> boss = nullptr;
    //キャラ初期位置
    DirectX::XMFLOAT3 charaPos{};
    //平行光
    std::shared_ptr<DirLight> dirLight = nullptr;
    //ディファードレンダー
    std::unique_ptr<DeferredRenderer> deferred = nullptr;

    //IBL
    struct IBL_constants
    {
        DirectX::XMFLOAT4 lightRoti = { 3.0f,3.0f,3.0f,0.0f }; //光の輝き
        DirectX::XMFLOAT4 iblIntencity = { 2.0f,0.0f,0.0f,0.0f };
    };
    std::unique_ptr<Constants<IBL_constants>> IBL_constant{};

    //トーンマップ
    Microsoft::WRL::ComPtr<ID3D11PixelShader> toneMapPixelShader;

    //操作説明UI（仮）
    //std::unique_ptr<SpriteBatch> operation_ui = nullptr;

    //std::unique_ptr<Tutorial> tutorial = nullptr;

    //BGM
    std::shared_ptr<audio> audios[8];

    //スカイマップ
    std::unique_ptr<SkyMap> skymap;

    //ラジアルブラー
    std::unique_ptr<RadialBlur> radialBlur;

    //色収差
    std::unique_ptr<Glitch_CA> glitch_CA;

    //フレームバッファ
    std::unique_ptr<framebuffer> framebuffers[8];

    //画面転送用フルスクリーンクワッド
    std::unique_ptr<fullscreen_quad> bit_block_transfer;

    //勝利と敗北画像
    std::unique_ptr<SpriteBatch> spriteVictory = nullptr;
    std::unique_ptr<SpriteBatch> spriteLose = nullptr;

    //ゲーム終了判定
    bool isEnd = false;

    //ヒットストップのオンオフ
    bool isHitStop = false;

    //Imguiのオンオフ　※テスト用
    bool displayImgui = false;
    bool IBLImgui = false;
};


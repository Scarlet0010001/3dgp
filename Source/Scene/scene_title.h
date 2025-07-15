#pragma once
#include "scene.h"
#include "Sprite/sprite_batch.h"
#include "Audio/audio.h"

class SceneTitle :
    public Scene
{
public:
    SceneTitle() {}
    ~SceneTitle()override {}

    void Initialize()override;

    //終了化
    void Finalize() override;

    //更新処理
    void Update(float elapsedTime)override;

    //描画処理
    void Render(float elapsedTime) override;

private:
    enum class TITLE_MENU
    {
        GAME_START,
        EXIT
    };
    TITLE_MENU selectedMenuState;

    //タイトル背景
    std::unique_ptr<SpriteBatch> spriteTitleBack = nullptr;

    const DirectX::XMFLOAT4 select{ 1.0f,0,0,1.0f };
    DirectX::XMFLOAT4 colorMenu[2]{};

    std::unique_ptr<SpriteBatch> spriteStart = nullptr;
    std::unique_ptr<SpriteBatch> spriteExit = nullptr;

    std::shared_ptr<audio> audios[8];

};


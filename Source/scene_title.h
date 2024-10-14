#pragma once
#include "scene.h"
#include "sprite_batch.h"

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

};


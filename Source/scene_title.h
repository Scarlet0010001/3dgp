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

    //�I����
    void Finalize() override;

    //�X�V����
    void Update(float elapsedTime)override;

    //�`�揈��
    void Render(float elapsedTime) override;

private:
    enum class TITLE_MENU
    {
        GAME_START,
        EXIT
    };
    TITLE_MENU selectedMenuState;

    //�^�C�g���w�i
    std::unique_ptr<SpriteBatch> spriteTitleBack = nullptr;

};


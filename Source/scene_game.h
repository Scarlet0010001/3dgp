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
    //�V�[��������
    void Initialize() override;
    //�V�[���I������
    void Finalize() override;
    //�V�[���A�b�v�f�[�g
    void Update(float elapsedTime) override;
    //�V�[���`��
    void Render(float elapsedTime) override;

    //�N�����̍X�V
    //void clear_update(float elapsedTime);

    //�f�o�b�O�`��
    void DebugGui();

    //�V�[�����Z�b�g
    void SceneReset();

protected:
    struct SCENE_CONSTANTS
    {
        //DirectX::XMFLOAT4X4 view_projection; //�r���[�E�v���W�F�N�V�����ϊ��s��
        //DirectX::XMFLOAT4 light_direction; //���C�g�̌���
        //DirectX::XMFLOAT4 camera_position;
    };
    std::unique_ptr<Constants<SCENE_CONSTANTS>> scene_constants{};


private:

    //�J����
    Camera* camera = nullptr;
    //�v���C���[
    std::unique_ptr<Player> player = nullptr;
    //�L���������ʒu
    DirectX::XMFLOAT3 charaPos{};
    //���s��
    std::shared_ptr<DirLight> dirLight = nullptr;
    //�f�B�t�@�[�h�����_�[
    std::unique_ptr<DeferredRenderer> deferred = nullptr;
    /*
    //�{�X
    std::unique_ptr<Boss> boss = nullptr;
    //�X�J�C�{�b�N�X
    std::unique_ptr<SkyBox> skybox = nullptr;
    //�������UI�i���j
    std::unique_ptr<SpriteBatch> operation_ui = nullptr;

    std::unique_ptr<Tutorial> tutorial = nullptr;
    */
    //�X�J�C�}�b�v
    std::unique_ptr<SkyMap> skymap;

    std::unique_ptr<framebuffer> framebuffers[8];

    std::unique_ptr<fullscreen_quad> bit_block_transfer;

    //�^�C�g���ɖ߂�@���e�X�g�p
    bool displayImgui = false;

};


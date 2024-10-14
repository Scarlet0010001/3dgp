#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include "constant.h"
#include "user.h"

#include <cereal/cereal.hpp>

class Camera
{
public:
    static Camera& Instance()
    {
        static Camera instance;
        return instance;
    }
private:
    //--------< �萔/�\���� >--------//
    struct SCENE_CONSTANTS
    {
        DirectX::XMFLOAT4X4 view;                //�r���[�s��
        DirectX::XMFLOAT4X4 projection;          //�v���W�F�N�V�����s��
        DirectX::XMFLOAT4X4 view_projection;     //�r���[�E�v���W�F�N�V�����ϊ��s��
        DirectX::XMFLOAT4 light_color;       //���C�g�̐F
        DirectX::XMFLOAT4 light_direction;       //���C�g�̌���
        DirectX::XMFLOAT4 camera_position;
        DirectX::XMFLOAT4 avatar_position;
        DirectX::XMFLOAT4 avatar_direction;
        DirectX::XMFLOAT2 resolution;
        float time;
        float delta_time;
    };
public:
    //------�J�����V�F�C�N-------//
    struct CameraShakeParam
    {
        float max_X_shake = 0.0f;//���h��ő�l�@�����͂�Degree�l��
        float max_Y_shake = 0.0f;//�c�h��ő�l�@�����͂�Degree�l��
        float time = 0.0f;//�h��鎞��
        float shakeSmoothness = 1.0f;//�h����̊��炩��

        // �V���A���C�Y
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                cereal::make_nvp("max_x_shake", max_X_shake),
                cereal::make_nvp("max_y_shake", max_Y_shake),
                cereal::make_nvp("time", time),
                cereal::make_nvp("shake_smoothness", shakeSmoothness)
            );
        }
    };

    struct HitStopParam
    {
        float time = 0.0f;//�~�߂鎞��
        float stoppingStrength = 5.0f;//�~�߂鋭�x�i���S��0�ɂ��Ă��܂��ƃo�O�邽�߁j
        // �V���A���C�Y
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                cereal::make_nvp("time", time),
                cereal::make_nvp("stopping_strength", stoppingStrength)
            );
        }
    };

public:
    //--------<constructor/destructor>--------//
    //Camera(const char* post_effect_file_path);
    Camera();
    ~Camera() = default;

    //--------< �֐� >--------//
    void Update(float elapsedTime);
    //�Ώۂ�Ǐ]����
    void UpdateWithTracking(float elapsedTime);
    //���b�N�I���ΏۂɃJ������������
    void UpdateWithLockOn(float elapsedTime);
    //�R���g���[���[�̃X�e�B�b�N�ő���
    void ControlByGamePadStick(float elapsedTime);
    //void move_viewing_angle(bool is_move, float elapsed_time){};
    void CalcViewProjection(float elapsedTime);

    void DebugGui();
    //�q�b�g�X�g�b�v�֐�
    float HitStopUpdate(float elapsedTime);

    //--------<getter/setter>--------//
// �ΏۂƂ̋���
    void SetRange(float r) { range = r; }

    // ����Ώ�
    void SetLockOnTarget(const DirectX::XMFLOAT3& t) { lockOnTarget = t; }
    const DirectX::XMFLOAT3& GetLockOnTarget() const { return lockOnTarget; }
    // �ǔ�����Ώ�
    void SetTrakkingTarget(const DirectX::XMFLOAT3& t) { trakkingTarget = t; }
    const DirectX::XMFLOAT3& GetTrakkingTarget() const { return trakkingTarget; }

    // �p�x
    void SetAngle(const DirectX::XMFLOAT3& a) { angle = a; }
    void SetCameraOperateStop(bool s) { cameraOperateStop = s; }
    //����p�ݒ�
    void SetCapeVision(const float& cape) { this->capeVision = cape; }

    //�x������
    void SetAttendRate(float rate) { attendRate = rate; }
    //�ΏۂƂ̋���
    const float& GetRange() const { return range; }

    // ���̂�������
    const DirectX::XMFLOAT4& GetLightDirection() const { return lightDirection; }

    // view
    const DirectX::XMFLOAT4X4& GetView() const { return view; }
    // projection
    const DirectX::XMFLOAT4X4& GetProjection() const { return projection; }

    //���_�擾
    const DirectX::XMFLOAT3& GetEye()const { return eye; }
    //�O�����擾
    const DirectX::XMFLOAT3& GetForward()const { return Math::get_posture_forward(orientation); }
    //�E�����擾
    const DirectX::XMFLOAT3& GetRight()const { return Math::get_posture_right(orientation); }
    //�N�H�[�^�j�I���擾
    const DirectX::XMFLOAT4& GetOrientation()const { return orientation; }

    //�^�[�Q�b�g���ړ����Ă��邩�ǂ���
    void SetIsMove(bool m) { this->isMove = m; }
    //����p�擾
    const float& GetCapeVision()const { return capeVision; }

    //���b�N�I��
    const bool GetLockOn() const { return lockOn; }
    void SetLockOn() { lockOn = !lockOn; }

    //�J�����V�F�C�N
    void SetCameraShake(CameraShakeParam param);

    void SetHitStop(HitStopParam param);

    const DirectX::XMFLOAT4& Getcamera_position() { return sceneConstant.get()->data.camera_position; }

private:
    void CalcFreeTarget();

    void CameraShakeUpdate(float elapsedTime);

    //--------< �֐��|�C���^ >--------//
    typedef void (Camera::* p_Update)(float elapsedTime);
    p_Update p_update = &Camera::UpdateWithTracking;

private:
    //--------< �ϐ� >--------//
    std::unique_ptr<Constants<SCENE_CONSTANTS>> sceneConstant{};

    ////�����_����̋���
    float range;
    DirectX::XMFLOAT3 eye; //���_
    DirectX::XMFLOAT3 trakkingTarget;//�����_
    DirectX::XMFLOAT3 lockOnTarget;//�����_
    DirectX::XMFLOAT3 angle;
    DirectX::XMFLOAT4 orientation = { 0,0,0,1 };
    DirectX::XMFLOAT4 standardOrientation = { 0,0,0,1 };

    float lockOnRate = 6.0f;
    float sensitivityRate = 0.7f;
    bool isMove;
    float attendRate; // �����䗦
    float capeVision = 60.0f;//����p
    float rollSpeed = 90;//��]���x

    //�����x��
    float verticalRotationDegree = 0;
    //���s�x��
    float horizonRotationDegree = 0;

    DirectX::XMFLOAT4 lightColor = { 1.0f,1.0f, 1.0f,1.0f };
    DirectX::XMFLOAT4 lightDirection{ 1.0f,1.0f, 1.0f,1.0f };

    //�㉺�̌�����p�x����
    float maxAngle_X;
    float minAngle_X;

    //�}�E�X����
    bool isMouseOperation;

    //view
    DirectX::XMFLOAT4X4 view;
    //projection
    DirectX::XMFLOAT4X4 projection;
    //view_projection
    DirectX::XMFLOAT4X4 viewProjection;
    //�f�o�b�OGUI�\��
    bool displayCameraImgui = false;
    bool cameraOperateStop;


    //���b�N�I���t���O
    bool lockOn = false;
    float lockOnAngle;//���b�N�I���ΏۂƃJ�������ʃx�N�g���Ƃ̊p�x�i�J�����V�F�C�N�̎d�l���J�o�[���邽�߁j

    //------�J�����V�F�C�N-------//
    bool isCameraShake = false;//�J�����V�F�C�N��
    CameraShakeParam cameraShakeParam;

    //------�q�b�g�X�g�b�v-------//
    bool isHitStop = false;//�q�b�g�X�g�b�v��
    HitStopParam hitStopParam;

    //------�|�X�g�G�t�F�N�g-------//
    //std::shared_ptr<PostEffects> post_effect;

};


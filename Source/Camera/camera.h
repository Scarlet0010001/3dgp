#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include "Constant/constant.h"
#include "User/user.h"
#include "User/noise.h"
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
    //--------< 定数/構造体 >--------//
    struct SCENE_CONSTANTS
    {
        DirectX::XMFLOAT4X4 view;                           // ビュー行列（カメラの視点からシーンを表示するための変換行列）
        DirectX::XMFLOAT4X4 projection;                 // プロジェクション行列（視野の設定や遠近感の調整を行うための変換行列）
        DirectX::XMFLOAT4X4 view_projection;      // ビュー・プロジェクション変換行列（ビュー行列とプロジェクション行列を掛け合わせたもの）
        DirectX::XMFLOAT4 light_color;                   // ライトの色（RGBA形式でライトの色を定義）
        DirectX::XMFLOAT4 light_direction;            // ライトの向き（ライトがどの方向から照射されるかを示すベクトル）
        DirectX::XMFLOAT4 camera_position;         // カメラの位置（シーン内でカメラがどこに配置されているか）
        DirectX::XMFLOAT4 avatar_position;           // アバターの位置（アバターキャラクターの位置）
        DirectX::XMFLOAT4 avatar_direction;          // アバターの向き（アバターキャラクターが向いている方向を示すベクトル）
        DirectX::XMFLOAT2 resolution;                    // 解像度（スクリーンやビューの幅と高さ）
        float time;                                                       // 経過時間（シーン内の時間の経過を保持）
        float delta_time;                                            // 前フレームからの時間差（フレームごとの時間差を保持）
    };

public:
    enum class STATE
    {
        Tracking = 0,   // トラッキング状態（ターゲットの追跡）
        LockOn,           // ロックオン状態（ターゲットをロック）
        Wing,               // ウィング状態（特殊な動作やアクションの状態）
    } state;                // プレイヤーの状態（現在の状態を保持）
    //------カメラシェイク-------//
    struct CameraShakeParam
    {
        float max_X_shake = 0.0f;//横揺れ最大値　※入力はDegree値で
        float max_Y_shake = 0.0f;//縦揺れ最大値　※入力はDegree値で
        float time = 0.0f;//揺れる時間
        float shakeSmoothness = 1.0f;//揺れ方の滑らかさ
        bool onDistanceShake = false;

        // シリアライズ
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
        float time = 0.0f;//止める時間
        // シリアライズ
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                cereal::make_nvp("time", time)
            );
        }
    };

public:
    //--------<constructor/destructor>--------//
    Camera();
    ~Camera() = default;

    //初期化
    void Initialize();

    //--------< 関数 >--------//
    void Update(float elapsedTime);
    //対象を追従する
    void UpdateWithTracking(float elapsedTime);
    //ロックオン対象にカメラを向ける
    void UpdateWithLockOn(float elapsedTime);
    //飛行中プレイヤーの正面を写すように向ける
    void UpdateWithWing(float elapsedTime);
    //コントローラーのスティックで操作
    void ControlByGamePadStick(float elapsedTime);
    //マウスで操作
    void ControlByMouse(float elapsedTime);
    //void move_viewing_angle(bool is_move, float elapsed_time){};
    void CalcViewProjection(float elapsedTime);

    void DebugGui();
    //ヒットストップ関数
    bool HitStopUpdate(float elapsedTime);

    //--------<getter/setter>--------//
// 対象との距離
    void SetRange(float r) { range = r; }

    void SetShakeDistance(float r) { shakeDistance = r; }

    // 見る対象
    void SetLockOnTarget(const DirectX::XMFLOAT3& t) { lockOnTarget = t; }
    const DirectX::XMFLOAT3& GetLockOnTarget() const { return lockOnTarget; }
    
    // 追尾する対象
    DirectX::XMFLOAT3 newTrakkingTarget{};
    void SetTrakkingTarget(const DirectX::XMFLOAT3& t) {
        newTrakkingTarget = t;

	    trakkingTarget.x+= (newTrakkingTarget.x - trakkingTarget.x) * 0.2f;
        trakkingTarget .y+= (newTrakkingTarget.y - trakkingTarget.y) * 0.2f;
        trakkingTarget.z += (newTrakkingTarget.z- trakkingTarget.z) * 0.2f;
    }
    //追尾する対象を取得
    const DirectX::XMFLOAT3& GetTrakkingTarget() const { return trakkingTarget; }

    // 角度
    void SetAngle(const DirectX::XMFLOAT3& a) { angle = a; }
    void SetCameraOperateStop(bool s) { cameraOperateStop = s; }
    //視野角設定
    void SetCapeVision(const float& cape) { this->capeVision = cape; }

    //遅延時間
    void SetAttendRate(float rate) { attendRate = rate; }
    //対象との距離
    const float& GetRange() const { return range; }

    // 光のさす方向
    const DirectX::XMFLOAT4& GetLightDirection() const { return lightDirection; }

    // view
    const DirectX::XMFLOAT4X4& GetView() const { return view; }
    // projection
    const DirectX::XMFLOAT4X4& GetProjection() const { return projection; }

    //視点取得
    const DirectX::XMFLOAT3& GetEye()const { return eye; }
    //前方向取得
    const DirectX::XMFLOAT3& GetForward()const { return Math::GetPostureForward(orientation); }
    //右方向取得
    const DirectX::XMFLOAT3& GetRight()const { return Math::GetPostureRight(orientation); }
    //クォータニオン取得
    const DirectX::XMFLOAT4& GetOrientation()const { return orientation; }

    //playerクォータニオン取得
    const DirectX::XMFLOAT4& GetPlayerOrientation()const { return playerOrientation; }
    //playerクォータニオン設定
    void SetPlayerOrientation(DirectX::XMFLOAT4 orientation) { playerOrientation = orientation; }

    //視野角取得
    const float& GetCapeVision()const { return capeVision; }

    //ロックオン
    const bool GetLockOn() const { return lockOn; }
    void SetLockOn() { lockOn = !lockOn; }

    //カメラシェイク
    void SetCameraShake(CameraShakeParam param);
    void SetOnDistanceShake(bool onDistance) { cameraShakeParam.onDistanceShake = onDistance; }

    //ヒットストップの取得と設定
    void SetHitStop(HitStopParam param);
    bool GetHitStop() { return isHitStop; }

    //カメラ位置の取得
    const DirectX::XMFLOAT4& Getcamera_position() { return sceneConstant.get()->data.camera_position; }

private:
    //自由カメラ
    void CalcFreeTarget();

    //カメラシェイク更新
    void CameraShakeUpdate(float elapsedTime);

    //--------< 関数ポインタ >--------//
    typedef void (Camera::* p_Update)(float elapsedTime);
    p_Update pUpdate = &Camera::UpdateWithTracking;

private:
    //--------< 変数 >--------//
    std::unique_ptr<Constants<SCENE_CONSTANTS>> sceneConstant{};

    //// 注視点からの距離
    float range;                                                                            // 視点からターゲットまでの距離
    DirectX::XMFLOAT3 eye;                                                       // 視点の位置（カメラやプレイヤーの位置）
    DirectX::XMFLOAT3 trakkingTarget;                                   // 現在注視しているターゲット
    DirectX::XMFLOAT3 lockOnTarget;                                     // ロックオンしているターゲット
    DirectX::XMFLOAT3 oldLockOnTarget;                              // 前回ロックオンしていたターゲット
    DirectX::XMFLOAT3 wingTarget;                                       // 翼などのターゲット（特定の対象）
    DirectX::XMFLOAT3 angle;                                                 // 視点の角度（回転）
    DirectX::XMFLOAT4 orientation = { 0,0,0,1 };                  // 現在の姿勢（クォータニオン）
    DirectX::XMFLOAT4 standardOrientation = { 0,0,0,1 }; // 基準となる姿勢（クォータニオン）
    DirectX::XMFLOAT4 playerOrientation = { 0,0,0,1 };      // プレイヤーの姿勢（クォータニオン）

    float lockOnRate = 6.0f;                // ロックオンの速さ
    float sensitivityRate = 0.7f;         // 操作感度
    float attendRate;                          // 減衰比率
    float capeVision = 60.0f;             //視野角
    float mouseRollSpeed = 60;      //回転速度
    float stickRollSpeed = 300;       //回転速度

    //垂直遅延
    float verticalRotationDegree = 0;
    //平行遅延
    float horizonRotationDegree = 0;

    //rightの色と方向
    DirectX::XMFLOAT4 lightColor = { 1.0f,1.0f, 1.0f,1.0f };
    DirectX::XMFLOAT4 lightDirection{ 1.0f,1.0f, 1.0f,1.0f };

    //上下の向ける角度制限
    float maxAngle_X;
    float minAngle_X;

    //マウス操作
    bool isMouseOperation;

    //view
    DirectX::XMFLOAT4X4 view;
    //projection
    DirectX::XMFLOAT4X4 projection;
    //view_projection
    DirectX::XMFLOAT4X4 viewProjection;
    //デバッグGUI表示
    bool displayCameraImgui = false;
    bool cameraOperateStop;

    //デバッグ
    bool isFreeCamera = false;

    //ロックオンフラグ
    bool lockOn = false;
    float lockOnAngle;//ロックオン対象とカメラ正面ベクトルとの角度（カメラシェイクの仕様をカバーするため）

    //------カメラシェイク-------//
    bool isCameraShake = false;//カメラシェイク中
    CameraShakeParam cameraShakeParam;
    float shakeDistance = 0.0f;

    //------ヒットストップ-------//
    bool isHitStop = false;//ヒットストップ中
    HitStopParam hitStopParam;

    //------ポストエフェクト-------//
    //std::shared_ptr<PostEffects> post_effect;

};


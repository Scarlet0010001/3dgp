#include "framework.h"
#include "camera.h"
#include "graphics.h"
#include "stage_manager.h"
#include "operators.h"
#include "debug_renderer.h"

Camera::Camera()
	: range(20.0f)
	, eye(5, 5, 5)
	, angle(DirectX::XMConvertToRadians(0.0f), DirectX::XMConvertToRadians(-180.0f), 0)
	, trakkingTarget(0.0f, 0, 0)
	, lockOnTarget(30.0f, 10, 0)
	, lightDirection(0.6f, -1, 0.1f, 0.7f)
	, rollSpeed(200)
	, attendRate(8.0f)
	, maxAngle_X(DirectX::XMConvertToRadians(60))
	, minAngle_X(DirectX::XMConvertToRadians(-60))
	, view()
	, projection()
	, isMouseOperation(false)
{
	HRESULT hr{ S_OK };

	//----定数バッファ----//
	// カメラ関連
	{
		sceneConstant = std::make_unique<Constants<SCENE_CONSTANTS>>(Graphics::Instance().GetDevice().Get());
	}
	// orientationの初期化
	{
		DirectX::XMFLOAT3 n(0, 1, 0); // 軸（正規化）
		constexpr float angle = DirectX::XMConvertToRadians(0); //角度（ラジアン）
		orientation = {
			sinf(angle / 2) * n.x,
			sinf(angle / 2) * n.y,
			sinf(angle / 2) * n.z,
			cosf(angle / 2)
		};

		standardOrientation = {
			sinf(angle / 2) * n.x,
			sinf(angle / 2) * n.y,
			sinf(angle / 2) * n.z,
			cosf(angle / 2)
		};
	}
	// eyeの初期化
	{
		//カメラ回転値を回転行列に変換
		DirectX::XMMATRIX Transform = DirectX::XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z);
		//回転行列の前方向ベクトルを取り出す
		DirectX::XMVECTOR Front = Transform.r[2];
		DirectX::XMFLOAT3 front;
		DirectX::XMStoreFloat3(&front, Front);
		//注視点から後ろベクトル方向に一定距離離れたカメラ視点を求める
		eye.x = trakkingTarget.x - front.x * range;
		eye.y = trakkingTarget.y - front.y * range;
		eye.z = trakkingTarget.z - front.z * range;
	}

}

void Camera::Update(float elapsedTime)
{
	using namespace DirectX;
	// 任意のアップデートを実行
	(this->*p_update)(elapsedTime);

	//ロックオン時の挙動
	if (lockOn)
	{
		//ロックオン時のカメラの挙動
		UpdateWithLockOn(elapsedTime);
	}
	else
	{
		//ロックオンじゃないときは０に（前の値が残ってカメラシェイクの挙動バグる）
		lockOnAngle = 0;
		//ロックオンしていないときのカメラの挙動
		if (!cameraOperateStop)
		{
			//ControlByGamePadStick(elapsedTime);
			Mouse& mouse = Device::Instance().GetMouse();

			if (mouse.GetButton() & mouse.BTN_LEFT_CLICK)
			{
				float ax = mouse.GetCursorPosition().x - mouse.GetOldCursorPosition().x;
				float ay = mouse.GetCursorPosition().y - mouse.GetOldCursorPosition().y;

				//カメラ縦操作
				if (ay > 0.1f || ay < 0.1f)
				{
					angle.x = ay * DirectX::XMConvertToRadians(rollSpeed) * elapsedTime;
				}
				//カメラ横操作
				if (ax > 0.1f || ax < 0.1f)
				{
					angle.y = ax * DirectX::XMConvertToRadians(rollSpeed) * elapsedTime;
				}
				// XMVECTORクラスへ変換
				DirectX::XMVECTOR orientationVec = DirectX::XMLoadFloat4(&orientation);

				DirectX::XMVECTOR forward = Math::get_posture_forward_vec(orientation);
				DirectX::XMVECTOR up = { 0,1,0 };//カメラのY軸は常に（0,1,0）とする
				DirectX::XMVECTOR right = Math::get_posture_right_vec(orientation);

				//縦回転
				{
					//回転軸
					DirectX::XMVECTOR axis = DirectX::XMVector3Cross(forward, up);

					if (fabs(angle.x) > DirectX::XMConvertToRadians(0.1f))
					{
						//回転軸（axis）と回転角（axis）から回転クオータニオン（q）を求める
						DirectX::XMVECTOR q = DirectX::XMQuaternionRotationAxis(axis, angle.x);
						//矢印を徐々に目標座標に向ける
						DirectX::XMVECTOR  q2 = DirectX::XMQuaternionMultiply(orientationVec, q);
						orientationVec = DirectX::XMQuaternionSlerp(orientationVec, q2, sensitivityRate);
					}

				}

				//横回転
				{
					//回転軸
					DirectX::XMVECTOR axis = up;

					if (fabs(angle.y) > DirectX::XMConvertToRadians(0.1f))
					{
						//回転軸（axis）と回転角（axis）から回転クオータニオン（q）を求める
						DirectX::XMVECTOR q = DirectX::XMQuaternionRotationAxis(axis, angle.y);
						//矢印を徐々に目標座標に向ける
						DirectX::XMVECTOR  q2 = DirectX::XMQuaternionMultiply(orientationVec, q);
						orientationVec = DirectX::XMQuaternionSlerp(orientationVec, q2, sensitivityRate);
					}

				}
				//------------------------------------
				//カメラが上か下を向きすぎたときに補正する
				//------------------------------------

				//向きすぎと判断する値
				const float overdirection = 0.4f;
				if (Math::get_posture_up(orientation).y < overdirection)
				{
					if (Math::get_posture_forward(orientation).y < 0.0f)
					{
						float correction_rate = -5.0f;
						DirectX::XMVECTOR correct_angles_axis = DirectX::XMQuaternionRotationAxis(right, DirectX::XMConvertToRadians(correction_rate));
						orientationVec = DirectX::XMQuaternionMultiply(orientationVec, correct_angles_axis);
					}
					else
					{
						float correction_rate = 5.0f;
						DirectX::XMVECTOR correct_angles_axis = DirectX::XMQuaternionRotationAxis(right, DirectX::XMConvertToRadians(correction_rate));
						orientationVec = DirectX::XMQuaternionMultiply(orientationVec, correct_angles_axis);
					}
				}

				// orientationVecからorientationを更新
				XMStoreFloat4(&orientation, orientationVec);

			}
		}
	}

	//カメラのパラメータ設定(全アップデート共通)
	{
		//世界の上方向
		DirectX::XMFLOAT3 up(0, 1, 0);
		XMVECTOR up_vec = XMLoadFloat3(&up);
		//視線ベクトル
		XMVECTOR eye_vec = XMLoadFloat3(&eye);
		//注視対象
		XMVECTOR focus_vec = XMLoadFloat3(&trakkingTarget);
		//view行列
		XMMATRIX view_mat = XMMatrixLookAtLH(eye_vec, focus_vec, up_vec);
		XMStoreFloat4x4(&view, view_mat);

		// プロジェクション行列を作成
		float width = static_cast<float>(SCREEN_WIDTH);
		float height = static_cast<float>(SCREEN_HEIGHT);
		float aspect_ratio{ width / height };
		static DirectX::XMFLOAT2 nearFar = { 0.1f, 5000.0f };


		//DebugPrimitive
		{
			DebugRenderer* debugRender = Graphics::Instance().GetDebugRenderer();
			
			debugRender->CreateSphere(
				trakkingTarget, 1.0f, { 1.0f,1.0f,0.0f,1.0f });
			
		}
			//trackingtargetの位置に球を出す
#ifdef USE_IMGUI
		if (displayCameraImgui)
		{
			ImGui::Begin("main_camera");
			ImGui::DragFloat2("nearFar", &nearFar.x, 0.1f, 0.1f, 2000.0f);
			ImGui::End();
		}
#endif // USE_IMGUI
		XMMATRIX projection_mat = XMMatrixPerspectiveFovLH(XMConvertToRadians(capeVision), aspect_ratio, nearFar.x, nearFar.y); // P
		XMStoreFloat4x4(&projection, projection_mat);
	}
}

//追従更新
void Camera::UpdateWithTracking(float elapsedTime)
{
	//Y軸の回転値を-3.14~3.14に収まるようにする
	if (angle.y < -DirectX::XM_PI) { angle.y += DirectX::XM_2PI; }
	if (angle.y > DirectX::XM_PI) { angle.y -= DirectX::XM_2PI; }

	//カメラシェイク
	//CameraShakeUpdate(elapsedTime);

	// カメラ回転値を回転行列に変換
	// XMVECTORクラスへ変換
	DirectX::XMFLOAT3 forward = Math::get_posture_forward(orientation);

	// レイキャスト(ターゲットと壁)
	DirectX::XMFLOAT3 ray_target = trakkingTarget + DirectX::XMFLOAT3{ 0,-0.5,0 };//めり込まないよう少し下に下げる
	DirectX::XMFLOAT3 start = ray_target;
	DirectX::XMFLOAT3 end = ray_target - forward * DirectX::XMFLOAT3(range, range, range);
	HitResult hit;
	//StageManager::Instance().RayCast(start, end, hit);

	//hit.distance = (std::max)(hit.distance, 0.5f);
	//hit.distance = (std::min)(hit.distance, range);

	hit.distance = 20.0f;

	//注視点から後ろベクトル方向に一定距離離れたカメラ視点を求める
	DirectX::XMFLOAT3 pos;
	pos.x = trakkingTarget.x - forward.x * hit.distance;
	pos.y = trakkingTarget.y - forward.y * hit.distance;
	pos.z = trakkingTarget.z - forward.z * hit.distance;
	eye = Math::Lerp(eye, pos, attendRate * elapsedTime);

}


void Camera::UpdateWithLockOn(float elapsedTime)
{
	// XMVECTORクラスへ変換
// カメラの現在位置から、目標座標への方向を求める
	DirectX::XMFLOAT3 dir = lockOnTarget - eye;
	DirectX::XMVECTOR TargeVecNorm = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&dir));
	DirectX::XMVECTOR Forward = Math::get_posture_forward_vec(orientation);
	DirectX::XMVECTOR Up = { 0.0f,1.0f,0.0f };
	DirectX::XMVECTOR orientationVec = DirectX::XMLoadFloat4(&orientation);

	DirectX::XMVECTOR dot = DirectX::XMVector3Dot(Forward, TargeVecNorm);
	DirectX::XMStoreFloat(&lockOnAngle, dot);
	lockOnAngle = acosf(lockOnAngle);

	DirectX::XMFLOAT3 forward{};//forwardの値をfloat3に
	DirectX::XMFLOAT3 up{};//upの値をfloat3に
	DirectX::XMFLOAT3 d_vec{};//dの値をfloat3に
	DirectX::XMStoreFloat3(&forward, Forward);
	DirectX::XMStoreFloat3(&up, Up);
	DirectX::XMStoreFloat3(&d_vec, TargeVecNorm);

	//回転
	{
		//回転軸
		DirectX::XMVECTOR axis = Up;
		//回転角度がこの値を超えたときのみ計算
		const float extrapolated_angle = 5.0f;
		if (fabsf(lockOnAngle) > DirectX::XMConvertToRadians(extrapolated_angle))
		{
			float cross{ forward.x * d_vec.z - forward.z * d_vec.x };
			//クオータニオンは回転の仕方(どの向きに)
			if (cross < 0.0f)
			{
				//回転軸（axis）と回転角（axis）から回転クオータニオン（q）を求める
				DirectX::XMVECTOR q = DirectX::XMQuaternionRotationAxis(axis, lockOnAngle);
				//矢印を徐々に目標座標に向ける
				DirectX::XMVECTOR  q2 = DirectX::XMQuaternionMultiply(orientationVec, q);
				orientationVec = DirectX::XMQuaternionSlerp(orientationVec, q2, lockOnRate * elapsedTime);
			}
			else
			{
				//回転軸（axis）と回転角（axis）から回転クオータニオン（q）を求める
				DirectX::XMVECTOR q = DirectX::XMQuaternionRotationAxis(axis, -lockOnAngle);
				//矢印を徐々に目標座標に向ける
				DirectX::XMVECTOR  q2 = DirectX::XMQuaternionMultiply(orientationVec, q);
				orientationVec = DirectX::XMQuaternionSlerp(orientationVec, q2, lockOnRate * elapsedTime);
			}
		}


	}
	// orientationVecからorientationを更新
	XMStoreFloat4(&orientation, orientationVec);

}

void Camera::ControlByGamePadStick(float elapsedTime)
{
	GamePad& game_pad = Device::Instance().GetGamePad();
	float ax = game_pad.GetAxis_RX();
	float ay = game_pad.GetAxis_RY();
	//カメラ縦操作
	if (ay > 0.1f || ay < 0.1f)
	{
		angle.x = ay * DirectX::XMConvertToRadians(rollSpeed) * elapsedTime;
	}
	//カメラ横操作
	if (ax > 0.1f || ax < 0.1f)
	{
		angle.y = ax * DirectX::XMConvertToRadians(rollSpeed) * elapsedTime;
	}
	// XMVECTORクラスへ変換
	DirectX::XMVECTOR orientationVec = DirectX::XMLoadFloat4(&orientation);

	DirectX::XMVECTOR forward = Math::get_posture_forward_vec(orientation);
	DirectX::XMVECTOR up = { 0,1,0 };//カメラのY軸は常に（0,1,0）とする
	DirectX::XMVECTOR right = Math::get_posture_right_vec(orientation);

	//縦回転
	{
		//回転軸
		DirectX::XMVECTOR axis = DirectX::XMVector3Cross(forward, up);

		if (fabs(angle.x) > DirectX::XMConvertToRadians(0.1f))
		{
			//回転軸（axis）と回転角（axis）から回転クオータニオン（q）を求める
			DirectX::XMVECTOR q = DirectX::XMQuaternionRotationAxis(axis, angle.x);
			//矢印を徐々に目標座標に向ける
			DirectX::XMVECTOR  q2 = DirectX::XMQuaternionMultiply(orientationVec, q);
			orientationVec = DirectX::XMQuaternionSlerp(orientationVec, q2, sensitivityRate);
		}

	}

	//横回転
	{
		//回転軸
		DirectX::XMVECTOR axis = up;

		if (fabs(angle.y) > DirectX::XMConvertToRadians(0.1f))
		{
			//回転軸（axis）と回転角（axis）から回転クオータニオン（q）を求める
			DirectX::XMVECTOR q = DirectX::XMQuaternionRotationAxis(axis, angle.y);
			//矢印を徐々に目標座標に向ける
			DirectX::XMVECTOR  q2 = DirectX::XMQuaternionMultiply(orientationVec, q);
			orientationVec = DirectX::XMQuaternionSlerp(orientationVec, q2, sensitivityRate);
		}

	}
	//------------------------------------
	//カメラが上か下を向きすぎたときに補正する
	//------------------------------------

	//向きすぎと判断する値
	const float overdirection = 0.4f;
	if (Math::get_posture_up(orientation).y < overdirection)
	{
		if (Math::get_posture_forward(orientation).y < 0.0f)
		{
			float correction_rate = -5.0f;
			DirectX::XMVECTOR correct_angles_axis = DirectX::XMQuaternionRotationAxis(right, DirectX::XMConvertToRadians(correction_rate));
			orientationVec = DirectX::XMQuaternionMultiply(orientationVec, correct_angles_axis);
		}
		else
		{
			float correction_rate = 5.0f;
			DirectX::XMVECTOR correct_angles_axis = DirectX::XMQuaternionRotationAxis(right, DirectX::XMConvertToRadians(correction_rate));
			orientationVec = DirectX::XMQuaternionMultiply(orientationVec, correct_angles_axis);
		}
	}

	// orientationVecからorientationを更新
	XMStoreFloat4(&orientation, orientationVec);
}

void Camera::CameraShakeUpdate(float elapsedTime)
{
	DirectX::XMVECTOR orientationVec = DirectX::XMLoadFloat4(&orientation);
	DirectX::XMVECTOR standard_orientationVec = DirectX::XMLoadFloat4(&standardOrientation);


	//カメラシェイク効果中でありプレイヤーやロックオンなどのによるカメラへの力が加わっていない場合のみ揺らす
	if (isCameraShake && fabs(angle.x) <= 0 && fabs(angle.y) <= 0 && fabs(lockOnAngle) <= 0)
	{
		//時間更新
		cameraShakeParam.time -= elapsedTime;

		if (cameraShakeParam.time < 0.0f)
		{
			isCameraShake = false;
			cameraShakeParam.time = 0.0f;
			XMStoreFloat4(&orientation, standard_orientationVec);
			return;
		}

		//揺らす処理
		{
			DirectX::XMVECTOR forward = Math::get_posture_forward_vec(standardOrientation);
			DirectX::XMVECTOR up = { 0,1,0 };
			DirectX::XMVECTOR right = Math::get_posture_right_vec(standardOrientation);

		//	//縦回転
		//	if (cameraShakeParam.max_Y_shake > 0)
		//	{
		//		//任意の揺れ幅の最大値最小値の間でのランダム生成
		//		float shake = Noise::Instance().random_range(-cameraShakeParam.max_Y_shake, cameraShakeParam.max_Y_shake);
		//		shake = DirectX::XMConvertToRadians(shake);
		//		{
		//			//回転軸
		//			DirectX::XMVECTOR axis = right;

		//			if (fabs(shake) > DirectX::XMConvertToRadians(0.01f))
		//			{
		//				//回転軸（axis）と回転角（axis）から回転クオータニオン（q）を求める
		//				DirectX::XMVECTOR q = DirectX::XMQuaternionRotationAxis(axis, shake);

		//				standard_orientationVec = DirectX::XMQuaternionMultiply(standard_orientationVec, q);
		//			}

		//		}
		//	}

		//	//横揺れ
		//	if (cameraShakeParam.max_X_shake > 0)
		//	{
		//		//任意の揺れ幅の最大値最小値の間でのランダム生成
		//		float shake = Noise::Instance().random_range(-cameraShakeParam.max_X_shake, cameraShakeParam.max_X_shake);
		//		shake = DirectX::XMConvertToRadians(shake);
		//		{
		//			//回転軸
		//			DirectX::XMVECTOR axis = up;

		//			if (fabs(shake) > DirectX::XMConvertToRadians(0.01f))
		//			{
		//				//回転軸（axis）と回転角（axis）から回転クオータニオン（q）を求める
		//				DirectX::XMVECTOR q = DirectX::XMQuaternionRotationAxis(axis, shake);

		//				standard_orientationVec = DirectX::XMQuaternionMultiply(standard_orientationVec, q);
		//			}
		//		}
		//	}
		}
		orientationVec = DirectX::XMQuaternionSlerp(orientationVec, standard_orientationVec, cameraShakeParam.shakeSmoothness);
		// orientationVecからorientationを更新
		XMStoreFloat4(&orientation, orientationVec);

		//XMStoreFloat4(&standard_orientation, orientationVec);

	}
	else
	{
		//揺れがない間は基準値
		XMStoreFloat4(&standardOrientation, orientationVec);
	}

}

void Camera::CalcViewProjection(float elapsedTime)
{
	// ビュー行列/プロジェクション行列を作成
	DirectX::XMMATRIX V = XMLoadFloat4x4(&view);
	DirectX::XMMATRIX P = XMLoadFloat4x4(&projection);
	// 定数バッファにフェッチする
	XMStoreFloat4x4(&sceneConstant->data.view, V);
	XMStoreFloat4x4(&sceneConstant->data.projection, P);
	XMStoreFloat4x4(&sceneConstant->data.view_projection, V * P);

	sceneConstant->data.light_color = lightColor;
	sceneConstant->data.light_direction = lightDirection;
	sceneConstant->data.camera_position = DirectX::XMFLOAT4(eye.x, eye.y, eye.z, 0);
	sceneConstant->data.avatar_position = DirectX::XMFLOAT4(trakkingTarget.x, trakkingTarget.y, trakkingTarget.z, 0);
	sceneConstant->data.resolution = { SCREEN_WIDTH,SCREEN_HEIGHT };
	sceneConstant->data.time += elapsedTime;
	sceneConstant->data.delta_time = elapsedTime;
	//すべてのシェーダーで使う可能性があるものなので全部に転送
	sceneConstant->Bind(Graphics::Instance().Get_DC().Get(), 1, CB_FLAG::ALL);

}

void Camera::DebugGui()
{
#ifdef USE_IMGUI
	imguiMenuBar("Camera", "main_camera", displayCameraImgui);

	if (displayCameraImgui)
	{
		ImGui::Begin("main_camera");
		//カメラシェイク
		if (ImGui::CollapsingHeader("CameraShake"))
		{
			static CameraShakeParam debug_param;

			ImGui::DragFloat("shake_x", &debug_param.max_X_shake, 0.1f);
			ImGui::DragFloat("shake_y", &debug_param.max_Y_shake, 0.1f);
			ImGui::DragFloat("time", &debug_param.time, 0.1f);
			ImGui::DragFloat("smmoth", &debug_param.shakeSmoothness, 0.1f, 0.1f, 1.0f);
			ImGui::DragFloat4("orientation", &orientation.x);
			ImGui::DragFloat4("standard_orientation", &standardOrientation.x);
			if (ImGui::Button("camera_shake"))
			{
				SetCameraShake(debug_param);
			}
		}
		//カメラシェイク
		if (ImGui::CollapsingHeader("HitStop"))
		{
			static HitStopParam debug_param;

			ImGui::DragFloat("time", &debug_param.time, 0.1f);
			ImGui::DragFloat("stopping_strength", &debug_param.stoppingStrength, 0.1f);
			if (ImGui::Button("hit_stop"))
			{
				SetHitStop(debug_param);
			}
		}
		if (ImGui::CollapsingHeader("LockOn"))
		{
			ImGui::Checkbox("lock_on", &lockOn);
			ImGui::DragFloat3("lockon_target", &lockOnTarget.x);
		}


		if (ImGui::CollapsingHeader("Update"))
		{
			if (ImGui::Button("tracking")) p_update = &Camera::UpdateWithTracking;
		}
		if (ImGui::CollapsingHeader("Param"))
		{

			ImGui::DragFloat("range", &range, 0.2f);
			DirectX::XMFLOAT3 a = { DirectX::XMConvertToDegrees(angle.x),DirectX::XMConvertToDegrees(angle.y),DirectX::XMConvertToDegrees(angle.z) };
			DirectX::XMFLOAT3 up = Math::get_posture_up(orientation);
			DirectX::XMFLOAT3 forward = Math::get_posture_forward(orientation);
			DirectX::XMFLOAT3 right = Math::get_posture_forward(orientation);
			ImGui::DragFloat2("angle", &a.x, 0.1f);
			ImGui::DragFloat3("up", &up.x, 0.1f);
			ImGui::DragFloat3("forward", &forward.x, 0.1f);
			ImGui::DragFloat3("right", &right.x, 0.1f);
			ImGui::DragFloat("cape_vision", &capeVision, 0.1f);
			ImGui::DragFloat("attend_rate", &attendRate, 0.1f);
			ImGui::DragFloat("roll_speed", &rollSpeed, 0.1f);
			ImGui::DragFloat("lock_on_rate", &lockOnRate, 0.1f);
			angle = { DirectX::XMConvertToRadians(a.x),DirectX::XMConvertToRadians(a.y),DirectX::XMConvertToRadians(a.z) };
			ImGui::DragFloat3("target", &trakkingTarget.x, 0.1f);
			ImGui::DragFloat4("LightDirection", &lightDirection.x, 0.01f, -1, 1);
			ImGui::DragFloat("time", &sceneConstant->data.time, 0.01f, -1, 1);
			ImGui::DragFloat("delta_time", &sceneConstant->data.delta_time, 0.01f, -1, 1);
		}
		if (ImGui::CollapsingHeader("color_picker", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::ColorPicker3("light_color", &lightColor.x);
		}

		ImGui::End();
	}

#endif

}

float Camera::HitStopUpdate(float elapsedTime)
{
	float resultElapsedTime = elapsedTime;
	if (isHitStop)
	{
		//ヒットストップ時の経過時間処理(0だとバグるので処理はスロー)
		resultElapsedTime /= hitStopParam.stoppingStrength;

		//タイマー処理
		hitStopParam.time -= elapsedTime;
		if (hitStopParam.time < 0.0f)
		{
			isHitStop = false;
		}
	}

	return resultElapsedTime;
}

void Camera::SetCameraShake(CameraShakeParam param)
{
	//カメラシェイクON
	isCameraShake = true;
	//パラメーター設定
	cameraShakeParam = param;

}

void Camera::SetHitStop(HitStopParam param)
{
	isHitStop = true;
	hitStopParam = param;

}

void Camera::CalcFreeTarget()
{
	Mouse& mouse = Device::Instance().GetMouse();

	float ax{};
	float ay{};
	{
		if (mouse.GetButton() & (mouse.BTN_A | mouse.BTN_LEFT)) { ax = -1; }    //左移動
		if (mouse.GetButton() & (mouse.BTN_D | mouse.BTN_RIGHT)) { ax = 1; }	 //右移動
		if (mouse.GetButton() & (mouse.BTN_W | mouse.BTN_UP)) { ay = 1; }	 //前移動
		if (mouse.GetButton() & (mouse.BTN_S | mouse.BTN_DOWN)) { ay = -1; }    //後ろ移動
	}

}

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

	//----�萔�o�b�t�@----//
	// �J�����֘A
	{
		sceneConstant = std::make_unique<Constants<SCENE_CONSTANTS>>(Graphics::Instance().GetDevice().Get());
	}
	// orientation�̏�����
	{
		DirectX::XMFLOAT3 n(0, 1, 0); // ���i���K���j
		constexpr float angle = DirectX::XMConvertToRadians(0); //�p�x�i���W�A���j
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
	// eye�̏�����
	{
		//�J������]�l����]�s��ɕϊ�
		DirectX::XMMATRIX Transform = DirectX::XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z);
		//��]�s��̑O�����x�N�g�������o��
		DirectX::XMVECTOR Front = Transform.r[2];
		DirectX::XMFLOAT3 front;
		DirectX::XMStoreFloat3(&front, Front);
		//�����_������x�N�g�������Ɉ�苗�����ꂽ�J�������_�����߂�
		eye.x = trakkingTarget.x - front.x * range;
		eye.y = trakkingTarget.y - front.y * range;
		eye.z = trakkingTarget.z - front.z * range;
	}

}

void Camera::Update(float elapsedTime)
{
	using namespace DirectX;
	// �C�ӂ̃A�b�v�f�[�g�����s
	(this->*p_update)(elapsedTime);

	//���b�N�I�����̋���
	if (lockOn)
	{
		//���b�N�I�����̃J�����̋���
		UpdateWithLockOn(elapsedTime);
	}
	else
	{
		//���b�N�I������Ȃ��Ƃ��͂O�Ɂi�O�̒l���c���ăJ�����V�F�C�N�̋����o�O��j
		lockOnAngle = 0;
		//���b�N�I�����Ă��Ȃ��Ƃ��̃J�����̋���
		if (!cameraOperateStop)
		{
			//ControlByGamePadStick(elapsedTime);
			Mouse& mouse = Device::Instance().GetMouse();

			if (mouse.GetButton() & mouse.BTN_LEFT_CLICK)
			{
				float ax = mouse.GetCursorPosition().x - mouse.GetOldCursorPosition().x;
				float ay = mouse.GetCursorPosition().y - mouse.GetOldCursorPosition().y;

				//�J�����c����
				if (ay > 0.1f || ay < 0.1f)
				{
					angle.x = ay * DirectX::XMConvertToRadians(rollSpeed) * elapsedTime;
				}
				//�J����������
				if (ax > 0.1f || ax < 0.1f)
				{
					angle.y = ax * DirectX::XMConvertToRadians(rollSpeed) * elapsedTime;
				}
				// XMVECTOR�N���X�֕ϊ�
				DirectX::XMVECTOR orientationVec = DirectX::XMLoadFloat4(&orientation);

				DirectX::XMVECTOR forward = Math::get_posture_forward_vec(orientation);
				DirectX::XMVECTOR up = { 0,1,0 };//�J������Y���͏�Ɂi0,1,0�j�Ƃ���
				DirectX::XMVECTOR right = Math::get_posture_right_vec(orientation);

				//�c��]
				{
					//��]��
					DirectX::XMVECTOR axis = DirectX::XMVector3Cross(forward, up);

					if (fabs(angle.x) > DirectX::XMConvertToRadians(0.1f))
					{
						//��]���iaxis�j�Ɖ�]�p�iaxis�j�����]�N�I�[�^�j�I���iq�j�����߂�
						DirectX::XMVECTOR q = DirectX::XMQuaternionRotationAxis(axis, angle.x);
						//�������X�ɖڕW���W�Ɍ�����
						DirectX::XMVECTOR  q2 = DirectX::XMQuaternionMultiply(orientationVec, q);
						orientationVec = DirectX::XMQuaternionSlerp(orientationVec, q2, sensitivityRate);
					}

				}

				//����]
				{
					//��]��
					DirectX::XMVECTOR axis = up;

					if (fabs(angle.y) > DirectX::XMConvertToRadians(0.1f))
					{
						//��]���iaxis�j�Ɖ�]�p�iaxis�j�����]�N�I�[�^�j�I���iq�j�����߂�
						DirectX::XMVECTOR q = DirectX::XMQuaternionRotationAxis(axis, angle.y);
						//�������X�ɖڕW���W�Ɍ�����
						DirectX::XMVECTOR  q2 = DirectX::XMQuaternionMultiply(orientationVec, q);
						orientationVec = DirectX::XMQuaternionSlerp(orientationVec, q2, sensitivityRate);
					}

				}
				//------------------------------------
				//�J�������ォ���������������Ƃ��ɕ␳����
				//------------------------------------

				//���������Ɣ��f����l
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

				// orientationVec����orientation���X�V
				XMStoreFloat4(&orientation, orientationVec);

			}
		}
	}

	//�J�����̃p�����[�^�ݒ�(�S�A�b�v�f�[�g����)
	{
		//���E�̏����
		DirectX::XMFLOAT3 up(0, 1, 0);
		XMVECTOR up_vec = XMLoadFloat3(&up);
		//�����x�N�g��
		XMVECTOR eye_vec = XMLoadFloat3(&eye);
		//�����Ώ�
		XMVECTOR focus_vec = XMLoadFloat3(&trakkingTarget);
		//view�s��
		XMMATRIX view_mat = XMMatrixLookAtLH(eye_vec, focus_vec, up_vec);
		XMStoreFloat4x4(&view, view_mat);

		// �v���W�F�N�V�����s����쐬
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
			//trackingtarget�̈ʒu�ɋ����o��
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

//�Ǐ]�X�V
void Camera::UpdateWithTracking(float elapsedTime)
{
	//Y���̉�]�l��-3.14~3.14�Ɏ��܂�悤�ɂ���
	if (angle.y < -DirectX::XM_PI) { angle.y += DirectX::XM_2PI; }
	if (angle.y > DirectX::XM_PI) { angle.y -= DirectX::XM_2PI; }

	//�J�����V�F�C�N
	//CameraShakeUpdate(elapsedTime);

	// �J������]�l����]�s��ɕϊ�
	// XMVECTOR�N���X�֕ϊ�
	DirectX::XMFLOAT3 forward = Math::get_posture_forward(orientation);

	// ���C�L���X�g(�^�[�Q�b�g�ƕ�)
	DirectX::XMFLOAT3 ray_target = trakkingTarget + DirectX::XMFLOAT3{ 0,-0.5,0 };//�߂荞�܂Ȃ��悤�������ɉ�����
	DirectX::XMFLOAT3 start = ray_target;
	DirectX::XMFLOAT3 end = ray_target - forward * DirectX::XMFLOAT3(range, range, range);
	HitResult hit;
	//StageManager::Instance().RayCast(start, end, hit);

	//hit.distance = (std::max)(hit.distance, 0.5f);
	//hit.distance = (std::min)(hit.distance, range);

	hit.distance = 20.0f;

	//�����_������x�N�g�������Ɉ�苗�����ꂽ�J�������_�����߂�
	DirectX::XMFLOAT3 pos;
	pos.x = trakkingTarget.x - forward.x * hit.distance;
	pos.y = trakkingTarget.y - forward.y * hit.distance;
	pos.z = trakkingTarget.z - forward.z * hit.distance;
	eye = Math::Lerp(eye, pos, attendRate * elapsedTime);

}


void Camera::UpdateWithLockOn(float elapsedTime)
{
	// XMVECTOR�N���X�֕ϊ�
// �J�����̌��݈ʒu����A�ڕW���W�ւ̕��������߂�
	DirectX::XMFLOAT3 dir = lockOnTarget - eye;
	DirectX::XMVECTOR TargeVecNorm = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&dir));
	DirectX::XMVECTOR Forward = Math::get_posture_forward_vec(orientation);
	DirectX::XMVECTOR Up = { 0.0f,1.0f,0.0f };
	DirectX::XMVECTOR orientationVec = DirectX::XMLoadFloat4(&orientation);

	DirectX::XMVECTOR dot = DirectX::XMVector3Dot(Forward, TargeVecNorm);
	DirectX::XMStoreFloat(&lockOnAngle, dot);
	lockOnAngle = acosf(lockOnAngle);

	DirectX::XMFLOAT3 forward{};//forward�̒l��float3��
	DirectX::XMFLOAT3 up{};//up�̒l��float3��
	DirectX::XMFLOAT3 d_vec{};//d�̒l��float3��
	DirectX::XMStoreFloat3(&forward, Forward);
	DirectX::XMStoreFloat3(&up, Up);
	DirectX::XMStoreFloat3(&d_vec, TargeVecNorm);

	//��]
	{
		//��]��
		DirectX::XMVECTOR axis = Up;
		//��]�p�x�����̒l�𒴂����Ƃ��̂݌v�Z
		const float extrapolated_angle = 5.0f;
		if (fabsf(lockOnAngle) > DirectX::XMConvertToRadians(extrapolated_angle))
		{
			float cross{ forward.x * d_vec.z - forward.z * d_vec.x };
			//�N�I�[�^�j�I���͉�]�̎d��(�ǂ̌�����)
			if (cross < 0.0f)
			{
				//��]���iaxis�j�Ɖ�]�p�iaxis�j�����]�N�I�[�^�j�I���iq�j�����߂�
				DirectX::XMVECTOR q = DirectX::XMQuaternionRotationAxis(axis, lockOnAngle);
				//�������X�ɖڕW���W�Ɍ�����
				DirectX::XMVECTOR  q2 = DirectX::XMQuaternionMultiply(orientationVec, q);
				orientationVec = DirectX::XMQuaternionSlerp(orientationVec, q2, lockOnRate * elapsedTime);
			}
			else
			{
				//��]���iaxis�j�Ɖ�]�p�iaxis�j�����]�N�I�[�^�j�I���iq�j�����߂�
				DirectX::XMVECTOR q = DirectX::XMQuaternionRotationAxis(axis, -lockOnAngle);
				//�������X�ɖڕW���W�Ɍ�����
				DirectX::XMVECTOR  q2 = DirectX::XMQuaternionMultiply(orientationVec, q);
				orientationVec = DirectX::XMQuaternionSlerp(orientationVec, q2, lockOnRate * elapsedTime);
			}
		}


	}
	// orientationVec����orientation���X�V
	XMStoreFloat4(&orientation, orientationVec);

}

void Camera::ControlByGamePadStick(float elapsedTime)
{
	GamePad& game_pad = Device::Instance().GetGamePad();
	float ax = game_pad.GetAxis_RX();
	float ay = game_pad.GetAxis_RY();
	//�J�����c����
	if (ay > 0.1f || ay < 0.1f)
	{
		angle.x = ay * DirectX::XMConvertToRadians(rollSpeed) * elapsedTime;
	}
	//�J����������
	if (ax > 0.1f || ax < 0.1f)
	{
		angle.y = ax * DirectX::XMConvertToRadians(rollSpeed) * elapsedTime;
	}
	// XMVECTOR�N���X�֕ϊ�
	DirectX::XMVECTOR orientationVec = DirectX::XMLoadFloat4(&orientation);

	DirectX::XMVECTOR forward = Math::get_posture_forward_vec(orientation);
	DirectX::XMVECTOR up = { 0,1,0 };//�J������Y���͏�Ɂi0,1,0�j�Ƃ���
	DirectX::XMVECTOR right = Math::get_posture_right_vec(orientation);

	//�c��]
	{
		//��]��
		DirectX::XMVECTOR axis = DirectX::XMVector3Cross(forward, up);

		if (fabs(angle.x) > DirectX::XMConvertToRadians(0.1f))
		{
			//��]���iaxis�j�Ɖ�]�p�iaxis�j�����]�N�I�[�^�j�I���iq�j�����߂�
			DirectX::XMVECTOR q = DirectX::XMQuaternionRotationAxis(axis, angle.x);
			//�������X�ɖڕW���W�Ɍ�����
			DirectX::XMVECTOR  q2 = DirectX::XMQuaternionMultiply(orientationVec, q);
			orientationVec = DirectX::XMQuaternionSlerp(orientationVec, q2, sensitivityRate);
		}

	}

	//����]
	{
		//��]��
		DirectX::XMVECTOR axis = up;

		if (fabs(angle.y) > DirectX::XMConvertToRadians(0.1f))
		{
			//��]���iaxis�j�Ɖ�]�p�iaxis�j�����]�N�I�[�^�j�I���iq�j�����߂�
			DirectX::XMVECTOR q = DirectX::XMQuaternionRotationAxis(axis, angle.y);
			//�������X�ɖڕW���W�Ɍ�����
			DirectX::XMVECTOR  q2 = DirectX::XMQuaternionMultiply(orientationVec, q);
			orientationVec = DirectX::XMQuaternionSlerp(orientationVec, q2, sensitivityRate);
		}

	}
	//------------------------------------
	//�J�������ォ���������������Ƃ��ɕ␳����
	//------------------------------------

	//���������Ɣ��f����l
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

	// orientationVec����orientation���X�V
	XMStoreFloat4(&orientation, orientationVec);
}

void Camera::CameraShakeUpdate(float elapsedTime)
{
	DirectX::XMVECTOR orientationVec = DirectX::XMLoadFloat4(&orientation);
	DirectX::XMVECTOR standard_orientationVec = DirectX::XMLoadFloat4(&standardOrientation);


	//�J�����V�F�C�N���ʒ��ł���v���C���[�⃍�b�N�I���Ȃǂ̂ɂ��J�����ւ̗͂�������Ă��Ȃ��ꍇ�̂ݗh�炷
	if (isCameraShake && fabs(angle.x) <= 0 && fabs(angle.y) <= 0 && fabs(lockOnAngle) <= 0)
	{
		//���ԍX�V
		cameraShakeParam.time -= elapsedTime;

		if (cameraShakeParam.time < 0.0f)
		{
			isCameraShake = false;
			cameraShakeParam.time = 0.0f;
			XMStoreFloat4(&orientation, standard_orientationVec);
			return;
		}

		//�h�炷����
		{
			DirectX::XMVECTOR forward = Math::get_posture_forward_vec(standardOrientation);
			DirectX::XMVECTOR up = { 0,1,0 };
			DirectX::XMVECTOR right = Math::get_posture_right_vec(standardOrientation);

		//	//�c��]
		//	if (cameraShakeParam.max_Y_shake > 0)
		//	{
		//		//�C�ӂ̗h�ꕝ�̍ő�l�ŏ��l�̊Ԃł̃����_������
		//		float shake = Noise::Instance().random_range(-cameraShakeParam.max_Y_shake, cameraShakeParam.max_Y_shake);
		//		shake = DirectX::XMConvertToRadians(shake);
		//		{
		//			//��]��
		//			DirectX::XMVECTOR axis = right;

		//			if (fabs(shake) > DirectX::XMConvertToRadians(0.01f))
		//			{
		//				//��]���iaxis�j�Ɖ�]�p�iaxis�j�����]�N�I�[�^�j�I���iq�j�����߂�
		//				DirectX::XMVECTOR q = DirectX::XMQuaternionRotationAxis(axis, shake);

		//				standard_orientationVec = DirectX::XMQuaternionMultiply(standard_orientationVec, q);
		//			}

		//		}
		//	}

		//	//���h��
		//	if (cameraShakeParam.max_X_shake > 0)
		//	{
		//		//�C�ӂ̗h�ꕝ�̍ő�l�ŏ��l�̊Ԃł̃����_������
		//		float shake = Noise::Instance().random_range(-cameraShakeParam.max_X_shake, cameraShakeParam.max_X_shake);
		//		shake = DirectX::XMConvertToRadians(shake);
		//		{
		//			//��]��
		//			DirectX::XMVECTOR axis = up;

		//			if (fabs(shake) > DirectX::XMConvertToRadians(0.01f))
		//			{
		//				//��]���iaxis�j�Ɖ�]�p�iaxis�j�����]�N�I�[�^�j�I���iq�j�����߂�
		//				DirectX::XMVECTOR q = DirectX::XMQuaternionRotationAxis(axis, shake);

		//				standard_orientationVec = DirectX::XMQuaternionMultiply(standard_orientationVec, q);
		//			}
		//		}
		//	}
		}
		orientationVec = DirectX::XMQuaternionSlerp(orientationVec, standard_orientationVec, cameraShakeParam.shakeSmoothness);
		// orientationVec����orientation���X�V
		XMStoreFloat4(&orientation, orientationVec);

		//XMStoreFloat4(&standard_orientation, orientationVec);

	}
	else
	{
		//�h�ꂪ�Ȃ��Ԃ͊�l
		XMStoreFloat4(&standardOrientation, orientationVec);
	}

}

void Camera::CalcViewProjection(float elapsedTime)
{
	// �r���[�s��/�v���W�F�N�V�����s����쐬
	DirectX::XMMATRIX V = XMLoadFloat4x4(&view);
	DirectX::XMMATRIX P = XMLoadFloat4x4(&projection);
	// �萔�o�b�t�@�Ƀt�F�b�`����
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
	//���ׂẴV�F�[�_�[�Ŏg���\����������̂Ȃ̂őS���ɓ]��
	sceneConstant->Bind(Graphics::Instance().Get_DC().Get(), 1, CB_FLAG::ALL);

}

void Camera::DebugGui()
{
#ifdef USE_IMGUI
	imguiMenuBar("Camera", "main_camera", displayCameraImgui);

	if (displayCameraImgui)
	{
		ImGui::Begin("main_camera");
		//�J�����V�F�C�N
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
		//�J�����V�F�C�N
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
		//�q�b�g�X�g�b�v���̌o�ߎ��ԏ���(0���ƃo�O��̂ŏ����̓X���[)
		resultElapsedTime /= hitStopParam.stoppingStrength;

		//�^�C�}�[����
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
	//�J�����V�F�C�NON
	isCameraShake = true;
	//�p�����[�^�[�ݒ�
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
		if (mouse.GetButton() & (mouse.BTN_A | mouse.BTN_LEFT)) { ax = -1; }    //���ړ�
		if (mouse.GetButton() & (mouse.BTN_D | mouse.BTN_RIGHT)) { ax = 1; }	 //�E�ړ�
		if (mouse.GetButton() & (mouse.BTN_W | mouse.BTN_UP)) { ay = 1; }	 //�O�ړ�
		if (mouse.GetButton() & (mouse.BTN_S | mouse.BTN_DOWN)) { ay = -1; }    //���ړ�
	}

}

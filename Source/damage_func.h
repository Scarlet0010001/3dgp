#pragma once
#include <functional>
#include "primitive.h"
#include <cereal/cereal.hpp>
#include "camera.h"
#include "game_pad.h"
struct AttackParam
{
	bool isAttack;//�U�������ǂ���
	int power;//�U����
	float invinsibleTime;//�U���Ώۂɉۂ����G����
	Camera::CameraShakeParam cameraShake;//�J�����V�F�C�N
	Camera::HitStopParam hitStop;//�q�b�g�X�g�b�v
	GamePad::Viberation hitViberation;
	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(
			cereal::make_nvp("power", power),
			cereal::make_nvp("invinsible_time", invinsibleTime),
			cereal::make_nvp("camera_shake", cameraShake),
			cereal::make_nvp("hit_stop", hitStop),
			cereal::make_nvp("hit_viberation", hitViberation)
		);
	}
};

//�_���[�W���󂯂����̂Ђ�ݕ�
enum class WINCE_TYPE
{
	NONE,//���݂Ȃ�
	SMALL,//�������Ђ��
	BIG,//�傫��������΂����
};
typedef std::function<bool(int, float, WINCE_TYPE)> AddDamageFunc;
#pragma once
#pragma comment (lib, "xinput.lib")

using GamePadButton = unsigned int;
#include <cereal/cereal.hpp>
// �Q�[���p�b�h
class GamePad
{
public:
	static const GamePadButton BTN_UP = (1 << 0);
	static const GamePadButton BTN_RIGHT = (1 << 1);
	static const GamePadButton BTN_DOWN = (1 << 2);
	static const GamePadButton BTN_LEFT = (1 << 3);
	static const GamePadButton BTN_A = (1 << 4);
	static const GamePadButton BTN_B = (1 << 5);
	static const GamePadButton BTN_X = (1 << 6);
	static const GamePadButton BTN_Y = (1 << 7);
	static const GamePadButton BTN_START = (1 << 8);
	static const GamePadButton BTN_BACK = (1 << 9);
	static const GamePadButton BTN_LEFT_THUMB = (1 << 10);
	static const GamePadButton BTN_RIGHT_THUMB = (1 << 11);
	static const GamePadButton BTN_LEFT_SHOULDER = (1 << 12);
	static const GamePadButton BTN_RIGHT_SHOULDER = (1 << 13);
	static const GamePadButton BTN_LEFT_TRIGGER = (1 << 14);
	static const GamePadButton BTN_RIGHT_TRIGGER = (1 << 15);

	struct Viberation
	{
		float R_moter;
		float L_moter;
		float VibeTime;
		// �V���A���C�Y
		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(
				cereal::make_nvp("r_moter", r_moter),
				cereal::make_nvp("l_moter", L_moter),
				cereal::make_nvp("vibe_time", VibeTime)
			);
		}
	};
public:
	GamePad() {}
	~GamePad() {}
	// �X�V
	/*�Ăяo����Framework�̂�*/
	void Update(float elapsedTime);
	// �X���b�g�ݒ�
	void SetSlot(int slot) { this->slot = slot; }

	// �{�^�����͏�Ԃ̎擾
	GamePadButton GetButton() const { return buttonState[0]; }
	// �{�^��������Ԃ̎擾
	GamePadButton GetButtonDown() const { return buttonDown; }
	// �{�^�������Ԃ̎擾
	GamePadButton GetButtonUp() const { return buttonUp; }

	// ���X�e�B�b�NX�����͏�Ԃ̎擾
	float GetAxis_LX() const { return axisLx; }
	// ���X�e�B�b�NY�����͏�Ԃ̎擾
	float GetAxis_LY() const { return axisLy; }
	// �E�X�e�B�b�NX�����͏�Ԃ̎擾
	float GetAxis_RX() const { return axisRx; }
	// �E�X�e�B�b�NY�����͏�Ԃ̎擾
	float GetAxis_RY() const { return axisRy; }
	// ���g���K�[���͏�Ԃ̎擾
	float GetTrigger_L() const { return triggerL; }
	// �E�g���K�[���͏�Ԃ̎擾
	float GetTrigger_R() const { return triggerR; }

	// �o�C�u���[�V����
	bool SetVibration(float R, float L, float StopTime);
	// ����L���A����
	void OperationDisablement() { operable = false; }
	void OperationActivation() { operable = true; }

private:
	GamePadButton		buttonState[2] = { 0 };
	GamePadButton		buttonDown = 0;
	GamePadButton		buttonUp = 0;
	float				axisLx = 0.0f;
	float				axisLy = 0.0f;
	float				axisRx = 0.0f;
	float				axisRy = 0.0f;
	float				triggerL = 0.0f;
	float				triggerR = 0.0f;
	int					slot = 0;

	float vib_stack_sec;
	float vib_time;
	bool isVibration = false;

	bool operable = true;
};
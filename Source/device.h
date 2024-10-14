#pragma once
#include "game_pad.h"
#include "mouse.h"
#include <memory>

class Device
{
private:
	Device();
	~Device() {}
public:
	static Device& Instance()
	{
		static Device instance;
		return instance;
	}
	// �X�V����
	void Update(HWND hwnd, float elapsed_time);

	// �Q�[���p�b�h�擾
	GamePad& GetGamePad() { return gamePad; }

	// �}�E�X�擾
	Mouse& GetMouse() { return mouse; }
private:
	GamePad gamePad;
	Mouse mouse;

};

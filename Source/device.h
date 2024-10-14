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
	// 更新処理
	void Update(HWND hwnd, float elapsed_time);

	// ゲームパッド取得
	GamePad& GetGamePad() { return gamePad; }

	// マウス取得
	Mouse& GetMouse() { return mouse; }
private:
	GamePad gamePad;
	Mouse mouse;

};

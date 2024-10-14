#include "user.h"
#include "mouse.h"

static const int KeyMap[] =
{
	VK_LBUTTON,		// 左ボタン
	VK_MBUTTON,		// 中ボタン
	VK_RBUTTON,		// 右ボタン
	'W',
	'A',
	'S',
	'D',
	VK_SPACE,
	'M',
	VK_ESCAPE,
	'Z',
	'I',
	'K',
	VK_SHIFT,
	'J',
	'L',
	VK_RETURN,
	VK_UP,
	VK_LEFT,
	VK_DOWN,
	VK_RIGHT,
	VK_F1,
	VK_F2,
	VK_F3,

};

void Mouse::Update(HWND hwnd)
{
	UpdateCursor(hwnd);
	//---キーボード----//
	// スイッチ情報
	MouseButton newButtonState = 0;

	for (int i = 0; i < ARRAYSIZE(KeyMap); ++i)
	{
		if (::GetAsyncKeyState(KeyMap[i]) & 0x8000)
		{
			newButtonState |= (1 << i);
		}
	}
	// ボタン情報更新
	if (operable)
	{
		buttonState[1] = buttonState[0];	// スイッチ履歴
		buttonState[0] = newButtonState;

		buttonDown = ~buttonState[1] & newButtonState;	// 押した瞬間
		buttonUp = ~newButtonState & buttonState[1];	// 離した瞬間
	}
	else
	{
		buttonState[1] = 0;	// スイッチ履歴
		buttonState[0] = 0;

		buttonDown = 0;	// 押した瞬間
		buttonUp = 0;	// 離した瞬間
	}
}

void Mouse::UpdateCursor(HWND hwnd)
{
	// マウス
	GetCursorPos(&point);           // スクリーン座標を取得する
	ScreenToClient(hwnd, &point);   // クライアント座標に変換する
	oldCursorPosition = cursorPosition;
	cursorPosition.x = (float)point.x - 15.0f;
	cursorPosition.y = (float)point.y - 15.0f;
	//　カーソルの範囲
	cursorPosition.x = std::clamp(cursorPosition.x, 0.0f, 1280.0f);
	cursorPosition.y = std::clamp(cursorPosition.y, 0.0f, 720.0f);
	// マウスの操作
	switch (state)
	{
	case 0: // 初期化
		SetCursorPos((int)cursorPosition.x, (int)cursorPosition.y);
		++state;
		/*fallthrough*/
	case 1: // 通常時
		ShowCursor(doShow);
		break;
	}
}
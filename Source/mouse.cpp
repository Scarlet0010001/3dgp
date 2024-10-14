#include "user.h"
#include "mouse.h"

static const int KeyMap[] =
{
	VK_LBUTTON,		// ���{�^��
	VK_MBUTTON,		// ���{�^��
	VK_RBUTTON,		// �E�{�^��
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
	//---�L�[�{�[�h----//
	// �X�C�b�`���
	MouseButton newButtonState = 0;

	for (int i = 0; i < ARRAYSIZE(KeyMap); ++i)
	{
		if (::GetAsyncKeyState(KeyMap[i]) & 0x8000)
		{
			newButtonState |= (1 << i);
		}
	}
	// �{�^�����X�V
	if (operable)
	{
		buttonState[1] = buttonState[0];	// �X�C�b�`����
		buttonState[0] = newButtonState;

		buttonDown = ~buttonState[1] & newButtonState;	// �������u��
		buttonUp = ~newButtonState & buttonState[1];	// �������u��
	}
	else
	{
		buttonState[1] = 0;	// �X�C�b�`����
		buttonState[0] = 0;

		buttonDown = 0;	// �������u��
		buttonUp = 0;	// �������u��
	}
}

void Mouse::UpdateCursor(HWND hwnd)
{
	// �}�E�X
	GetCursorPos(&point);           // �X�N���[�����W���擾����
	ScreenToClient(hwnd, &point);   // �N���C�A���g���W�ɕϊ�����
	oldCursorPosition = cursorPosition;
	cursorPosition.x = (float)point.x - 15.0f;
	cursorPosition.y = (float)point.y - 15.0f;
	//�@�J�[�\���͈̔�
	cursorPosition.x = std::clamp(cursorPosition.x, 0.0f, 1280.0f);
	cursorPosition.y = std::clamp(cursorPosition.y, 0.0f, 720.0f);
	// �}�E�X�̑���
	switch (state)
	{
	case 0: // ������
		SetCursorPos((int)cursorPosition.x, (int)cursorPosition.y);
		++state;
		/*fallthrough*/
	case 1: // �ʏ펞
		ShowCursor(doShow);
		break;
	}
}
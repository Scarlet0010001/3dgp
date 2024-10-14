#pragma once

#include <Windows.h>
#include <DirectXMath.h>

using MouseButton = unsigned int;

class Mouse
{
public:
    static const MouseButton BTN_LEFT_CLICK = (1 << 0);
    static const MouseButton BTN_MIDDLE_CLICK = (1 << 1);
    static const MouseButton BTN_RIGHT_CLICK = (1 << 2);
    static const MouseButton BTN_W = (1 << 3);
    static const MouseButton BTN_A = (1 << 4);
    static const MouseButton BTN_S = (1 << 5);
    static const MouseButton BTN_D = (1 << 6);
    static const MouseButton BTN_SPACE = (1 << 7);
    static const MouseButton BTN_M = (1 << 8);
    static const MouseButton BTN_ESC = (1 << 9);
    static const MouseButton BTN_Z = (1 << 10);
    static const MouseButton BTN_I = (1 << 11);
    static const MouseButton BTN_K = (1 << 12);
    static const MouseButton BTN_SHIFT = (1 << 13);
    static const MouseButton BTN_J = (1 << 14);
    static const MouseButton BTN_L = (1 << 15);
    static const MouseButton BTN_ENTER = (1 << 16);
    static const MouseButton BTN_UP = (1 << 17);
    static const MouseButton BTN_LEFT = (1 << 18);
    static const MouseButton BTN_DOWN = (1 << 19);
    static const MouseButton BTN_RIGHT = (1 << 20);
    static const MouseButton BTN_F1 = (1 << 21);
    static const MouseButton BTN_F2 = (1 << 22);
    static const MouseButton BTN_F3 = (1 << 23);
public:
    //--------< �R���X�g���N�^/�֐��� >--------//
    Mouse()
        : cursorPosition()
        , oldCursorPosition()
        , point()
        , state()
        , doShow(true)
        , n_notch()
        , z_delta()
        , isWheel(false) {}
    ~Mouse() {}
public:
    //--------< �֐� >--------//
    // �X�V
    /*�Ăяo����Framework�̂�*/
    void Update(HWND hwnd);
    //�J�[�\�����\���ɂ���
    void Set_do_show(bool show) { doShow = show; }     //�J�[�\�����\���ɂ���
    // �z�C�[���������Z�b�g
    void ResetWheel()
    {
        if (!isWheel) { z_delta = 0; n_notch = 0; }
        isWheel = false;
    }
    // �z�C�[������������(�`���h)
    bool Up_tilt() { return n_notch > 0; }
    bool Down_tilt() { return n_notch < 0; }
    //--------<getter/setter>--------//
    // �J�[�\��
    const DirectX::XMFLOAT2& GetCursorPosition() const { return cursorPosition; }
    const DirectX::XMFLOAT2& GetOldCursorPosition() const { return oldCursorPosition; }
    
    
    // �z�C�[��
    void set_n_notch(int n_not) { n_notch = n_not; }
    const int get_z_delta() const { return z_delta; }
    void set_z_delta(int z_del) { z_delta = z_del; }
    void set_is_wheel(bool is_wh) { isWheel = is_wh; }


    // �{�^�����͏�Ԃ̎擾
    MouseButton GetButton() const { return buttonState[0]; }
    // �{�^��������Ԃ̎擾
    MouseButton GetButtonDown() const { return buttonDown; }
    // �{�^�������Ԃ̎擾
    MouseButton GetButtonUp() const { return buttonUp; }
    // ����L���A����
    void OperationDisablement() { operable = false; }
    void OperationActivation() { operable = true; }

private:
    //--------< �֐� >--------//
    void UpdateCursor(HWND hwnd);
    //--------< �ϐ� >--------//
    // �J�[�\��
    DirectX::XMFLOAT2 cursorPosition;
    DirectX::XMFLOAT2 oldCursorPosition;
    POINT point;
    int state;
    bool doShow;

    // �z�C�[��
    int n_notch;
    int z_delta;
    bool isWheel;

    // �L�[�{�[�h
    MouseButton	buttonState[2] = { 0 };
    MouseButton	buttonDown = 0;
    MouseButton	buttonUp = 0;

    bool operable = true;
};
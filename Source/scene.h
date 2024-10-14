#pragma once
#include "graphics.h"
//�V�[�����N���X
class Scene
{
public:
	Scene() {}
	virtual ~Scene() {}

	//������
	virtual void Initialize() = 0;

	//�I����
	virtual void Finalize() = 0;

	//�X�V����
	virtual void Update(float elapsedTime) = 0;

	//�`�揈��
	virtual void Render(float elapsedTime) = 0;

	//�����������Ă��邩
	bool IsReady() const { return ready; }

	//���������ݒ�
	void SetReady(bool r) { ready = r; }

private:
	bool ready = false;
protected:
	bool isStart;
};

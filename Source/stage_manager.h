#pragma once
#include <vector>
#include "stage.h"

class StageManager
{
private:
	StageManager() {}
	~StageManager() {}

public:
	static StageManager& Instance()
	{
		static StageManager instance;
		return instance;
	}

	//�X�V����
	void Update(float elapsedTime);
	//�`�揈��
	void Render(float elapsed_time);
	//void Render(float elapsed_time, Camera* camera);
	//void shadow_render(float elapsed_time);
	//�X�e�[�W�o�^
	void Register(Stage* stage);
	//�X�e�[�W�S�폜
	void Clear();
	//���C�L���X�g
	bool RayCast(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, HitResult& hit);

private:
	std::vector<Stage*> stages;

};


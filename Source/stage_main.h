#pragma once
#include "stage.h"
class StageMain :
    public Stage
{
public:
	StageMain();
	~StageMain() override;

	//�C���X�^���X�擾
	static StageMain& Instance()
	{
		static StageMain instance;
		return instance;

	}

	//�X�V����
	void Update(float elapsedTime)override;

	//�`�揈��
	void Render(float elapsedTime)override;
	//void  shadow_render(float elapsed_time)override;
	// ���C�L���X�g
	bool RayCast(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, HitResult& hit) override;

private:
	std::unique_ptr<gltf_model>  model = nullptr;
	//std::unique_ptr<gltf_model>  modelCollision = nullptr;
	//std::unique_ptr<gltf_model>  modelShadow = nullptr;
	DirectX::XMFLOAT3 scale;
	DirectX::XMFLOAT3 angle{};
	DirectX::XMFLOAT3 position{};
	DirectX::XMFLOAT4X4 transform{};

	float animeTimer = 0.0f;
	std::vector<gltf_model::node> animated_nodes{};

	bool flustm_flag = false;

};


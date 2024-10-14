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

	//更新処理
	void Update(float elapsedTime);
	//描画処理
	void Render(float elapsed_time);
	//void Render(float elapsed_time, Camera* camera);
	//void shadow_render(float elapsed_time);
	//ステージ登録
	void Register(Stage* stage);
	//ステージ全削除
	void Clear();
	//レイキャスト
	bool RayCast(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, HitResult& hit);

private:
	std::vector<Stage*> stages;

};


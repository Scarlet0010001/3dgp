#pragma once
#include "scene.h"
#include "sprite_batch.h"
//#include <memory>

class SceneLoading :
    public Scene
{
public:
	SceneLoading(Scene* next_Scene) : nextScene(next_Scene) {}
	~SceneLoading() override {}
	void Initialize() override;
	void Finalize() override;
	void Update(float elapsedTime) override;
	void Render(float elapsedTime) override;
private:
	//ローディングスレッド
	static void LoadingThread(SceneLoading* scene);

	std::unique_ptr<SpriteBatch> sprite = nullptr;
	float angle = 0.0f;
	Scene* nextScene = nullptr;
};


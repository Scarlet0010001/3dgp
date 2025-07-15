#pragma once
#include "scene.h"
#include "Sprite/sprite_batch.h"

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

	std::unique_ptr<SpriteBatch> spriteBack = nullptr;
	std::unique_ptr<SpriteBatch> spriteIcon = nullptr;
	std::unique_ptr<SpriteBatch> spriteOperation = nullptr;
	float angle = 0.0f;
	Scene* nextScene = nullptr;
};


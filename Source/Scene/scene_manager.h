#pragma once
#include "scene.h"

class SceneManager
{
public:
	SceneManager();
	~SceneManager();

	static SceneManager& Instance()
	{
		static SceneManager instance;
		return instance;
	}

	void Initialize();
	void Update(float elapsedTime);
	void Render(float elapsedTime);

	//シーンクリア
	void Clear();

	void ChangeScene(Scene* scene);

private:
	//現在のシーン
	Scene* currentScene = nullptr;

};


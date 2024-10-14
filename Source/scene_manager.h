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
	void Update(float elapsed_time);
	void Render(float elapsed_time);

	//�V�[���N���A
	void Clear();

	void ChangeScene(Scene* scene);

private:
	//���݂̃V�[��
	Scene* currentScene = nullptr;

};


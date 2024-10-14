#include "scene_loading.h"
//#include "scene_game.h"
#include "scene_manager.h"

#include<thread>

void SceneLoading::Initialize()
{
	//�X�v���C�g������
	sprite = std::make_unique<SpriteBatch>(Graphics::Instance().GetDevice().Get(), L"Resources/Sprite/Loading/LoadingBack.png", 1);

	//�X���b�h�J�n
	std::thread thread(LoadingThread, this);

	//�X���b�h�̊Ǘ������
	thread.detach();
}

void SceneLoading::Finalize()
{
	sprite.reset();

}

void SceneLoading::Update(float elapsedTime)
{
	constexpr float speed = 180;
	angle += speed * elapsedTime;

	//���̃V�[���̏���������������V�[����؂�ւ���

	if (nextScene->IsReady())
	{
		SceneManager::Instance().ChangeScene(nextScene);
		nextScene = nullptr;
	}
}

void SceneLoading::Render(float elapsedTime)
{
	Graphics& graphics = Graphics::Instance();

	graphics.SetGraphicStatePriset(
		ST_DEPTH::DepthOFF_WriteOFF,
		ST_BLEND::ALPHA,
		ST_RASTERIZER::CULL_NONE
	);
	sprite->begin(graphics.Get_DC().Get());
	sprite->render(graphics.Get_DC().Get(), { 0, 0 }, { 1, 1 });
	sprite->end(graphics.Get_DC().Get());

}

void SceneLoading::LoadingThread(SceneLoading* scene)
{
	//���̃V�[���̏��������s��
	scene->nextScene->Initialize();
	//���̃V�[���̏��������ݒ�
	scene->nextScene->SetReady(true);
}

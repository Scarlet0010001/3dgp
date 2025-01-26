#include "scene_loading.h"
//#include "scene_game.h"
#include "scene_manager.h"

#include<thread>

void SceneLoading::Initialize()
{
	//スプライト初期化
	spriteBack = std::make_unique<SpriteBatch>(
		Graphics::Instance().GetDevice().Get(), 
		L"Resources/Sprite/Loading/LoadingBack.png", 1);
	spriteIcon = std::make_unique<SpriteBatch>(
		Graphics::Instance().GetDevice().Get(),
		L"Resources/Sprite/Loading/LoadingIcon.png", 1);

	//スレッド開始
	std::thread thread(LoadingThread, this);

	//スレッドの管理を放棄
	thread.detach();
}

void SceneLoading::Finalize()
{
	spriteBack.reset();
	spriteIcon.reset();

}

void SceneLoading::Update(float elapsedTime)
{
	constexpr float speed = 180;
	angle += speed * elapsedTime;

	//次のシーンの準備が完了したらシーンを切り替える

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
	spriteBack->begin(graphics.Get_DC().Get());
	spriteBack->render(graphics.Get_DC().Get(), { 0, 0 }, { 1, 1 });
	spriteBack->end(graphics.Get_DC().Get());

	float textureWidth = static_cast<float>(256);
	float textureHeight = static_cast<float>(256);
	float positionX = SCREEN_WIDTH - textureWidth;
	float positionY = SCREEN_HEIGHT - textureHeight;

	spriteIcon->begin(graphics.Get_DC().Get());
	spriteIcon->render(graphics.Get_DC().Get(), { positionX, positionY }, { 1, 1 }, { 1,1,1,1 }, angle);
	spriteIcon->end(graphics.Get_DC().Get());

}

void SceneLoading::LoadingThread(SceneLoading* scene)
{
	//次のシーンの初期化を行う
	scene->nextScene->Initialize();
	//次のシーンの準備完了設定
	scene->nextScene->SetReady(true);
}

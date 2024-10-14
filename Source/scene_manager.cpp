#include "scene_manager.h"

SceneManager::SceneManager()
{
}

SceneManager::~SceneManager()
{
    //シーンの削除
    Clear();
}

void SceneManager::Initialize()
{
    if (currentScene != nullptr)
    {
        currentScene->Initialize();
    }
}

void SceneManager::Update(float elapsed_time)
{
    if (currentScene != nullptr)
    {
        currentScene->Update(elapsed_time);
    }

}

void SceneManager::Render(float elapsed_time)
{
    if (currentScene != nullptr)
    {
        currentScene->Render(elapsed_time);
    }

}

void SceneManager::Clear()
{
    if (currentScene != nullptr)
    {
        currentScene->Finalize();
        delete currentScene;
        currentScene = nullptr;
    }

}

void SceneManager::ChangeScene(Scene* scene)
{
    //古いシーンの終了処理
    Clear();
    currentScene = scene;
    //シーン初期化処理
    if (!currentScene->IsReady()) currentScene->Initialize();
}

#include "scene_manager.h"

SceneManager::SceneManager()
{
}

SceneManager::~SceneManager()
{
    //�V�[���̍폜
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
    //�Â��V�[���̏I������
    Clear();
    currentScene = scene;
    //�V�[������������
    if (!currentScene->IsReady()) currentScene->Initialize();
}

#include "stage_manager.h"

void StageManager::Update(float elapsedTime)
{
    for (Stage* stage : stages)
    {
        stage->Update(elapsedTime);
    }

}

void StageManager::Render(float elapsedTime)
{
    for (Stage* stage : stages)
    {
        stage->Render(elapsedTime);
    }
}

void StageManager::Register(Stage* stage)
{
    stages.emplace_back(stage);

}

void StageManager::Clear()
{
    for (auto& stage : stages)delete stage;
    stages.clear();

}

bool StageManager::RayCast(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, HitResult& hit)
{
	bool result = false;
	hit.distance = FLT_MAX;
	for (auto s : stages)
	{
		HitResult hr;
		if (s->RayCast(start, end, hr))
		{
			if(hit.distance > hr.distance)
				hit = hr;
			result = true;
		}
	}
	return result;
}

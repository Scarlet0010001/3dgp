#pragma once
#include <memory>
#include <string>
#include <map>
#include "model_resource.h"
#include "sprite_batch.h"

class ResourceManager
{
public:
	ResourceManager() {}
	~ResourceManager() {}
	// モデルリソース読み込み
	std::shared_ptr<ModelResource> Load(ID3D11Device* device, const std::string* filename, bool triangulate = false, float sampling_rate = 0);
	static ResourceManager& Instance()
	{
		static ResourceManager instance;
		return instance;
	}
private:
	// model
	using ModelMap = std::map<std::string, std::weak_ptr<ModelResource>>;
	ModelMap  models;

};


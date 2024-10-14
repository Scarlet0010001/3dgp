#include "resource_manager.h"

std::shared_ptr<ModelResource> ResourceManager::Load(ID3D11Device* device, const std::string* filename, bool triangulate, float sampling_rate)
{
	// マップの中からモデルを検索
	ModelMap::iterator iter = models.find(filename->c_str());
	if (iter != models.end())
	{
		//リンクが切れていないか確認
		if (!iter->second.expired())  //secondはmapの[値]の部分のこと  *キーならfirst    expired :リンクが切れていないかの確認
		{
			//モデルリソースを返す
			return iter->second.lock(); //lock: 監視しているshared_ptrオブジェクトを取得する。 // https://cpprefjp.github.io/reference/memory/weak_ptr/lock.html
		}
	}
	// もし検索しても見つからなければ
	// 新規モデルリソース読み込み
	auto model = std::make_shared<ModelResource>(device, filename, triangulate, sampling_rate);
	// マップに登録
	models[filename->c_str()] = model;
	return model;
}

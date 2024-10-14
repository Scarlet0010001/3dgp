#pragma once
#define CAST_SHADOW 1
#include "graphics.h"
#include "light.h"
#include "fullscreen_quad.h"
#include <map>

class LightManager
{
private:
	LightManager() {};
	~LightManager();
public:
	void Initialize();
	//ライトを監視対象に追加
	void  Register(std::string name, std::shared_ptr<Light> light);
	//ライトをG-Bufferに送る
	void Draw(ID3D11ShaderResourceView** rtv, int rtv_num);

	void DebugGUI();

	void DeleteLight(std::string name);

	static LightManager& Instance()
	{
		static LightManager light_manager;
		return light_manager;
	}

#if CAST_SHADOW
	DirectX::XMFLOAT3 GetShadowDirLightDirection() { return shadowDirLight.get()->GetDirection(); }
#endif
private:
	std::map<std::string, std::weak_ptr<Light>>lights;
	std::unique_ptr<fullscreen_quad> lightScreen;
#if CAST_SHADOW
	std::shared_ptr<DirLight> shadowDirLight;//シャドウマップ用ライト
#endif
	Microsoft::WRL::ComPtr<ID3D11PixelShader> shadowMapLight;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> deferredLight;

	bool displayImgui = false;

};


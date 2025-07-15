#pragma once
#include <DirectXMath.h>
#include "Graphics/graphics.h"
#include "Sprite/sprite.h"
#include "Constant/constant.h"
#include "Graphics/fullscreen_quad.h"

class RadialBlur
{
public:
	RadialBlur(ID3D11Device* device);
	~RadialBlur() {}

	void DebugGUI();

	void Blit(ID3D11DeviceContext* immediate_context, ID3D11ShaderResourceView** shader_resource_view);

	bool GetIsDebug() { return isDebug; }

	struct radialBlurConstants
	{
		DirectX::XMFLOAT2 blurCenter = { 0.5, 0.5 };		// 中心点
		float blurStrength = 0.0f;									// ぼかし強度
		float blurRadius = 0.5f;										// ぼかし半径
		float blurDecay = 0.2f;											// 減衰率
		float blurTimer = 0.0f;											// 時間制御用
		float pads[2];															// 調整
	};
	std::unique_ptr<Constants<radialBlurConstants>> radial_blur_constant{};
private:
	//	ラジアルブラー
	Microsoft::WRL::ComPtr<ID3D11Buffer> radial_blur_constant_buffer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> radial_blur_sampler_state;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> radial_blur_pixel_shader;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> radial_blur_shader_resource_view;

	std::unique_ptr<fullscreen_quad> radial_quad;

	bool isDebug = false;

	bool displayRadialBlurImgui = false;
};


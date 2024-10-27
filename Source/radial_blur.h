#pragma once
#include <DirectXMath.h>
#include "graphics.h"
#include "constant.h"

class RadialBlur
{
private:
	struct radial_blur_constants
	{
		float				blur_radius = 50.0f;
		int					blur_sampling_count = 10;
		DirectX::XMFLOAT2	blur_center = { 0.5f, 0.5f };

		float				blur_mask_radius = 0;	//	centerからの指定の範囲はブラーを適応しないようにする
		DirectX::XMFLOAT3	blur_dummy;

	};
	//	ラジアルブラー
	std::unique_ptr<Constants<radial_blur_constants>> radial_blur_constant{};
	Microsoft::WRL::ComPtr<ID3D11Buffer> radial_blur_constant_buffer;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> radial_blur_pixel_shader;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> radial_blur_sampler_state;

	bool displayRadialBlurImgui = false;
public:
	RadialBlur(ID3D11Device* device);
	~RadialBlur() {}

	void DebugGUI();

	void blit();
};


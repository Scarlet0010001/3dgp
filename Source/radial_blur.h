#pragma once
#include <DirectXMath.h>
#include "graphics.h"
#include "sprite.h"
#include "constant.h"
#include "fullscreen_quad.h"

class RadialBlur//fullscreen_quadを親クラスにする
{
public:
	RadialBlur(ID3D11Device* device);
	~RadialBlur() {}

	void DebugGUI();

	void Blit(ID3D11DeviceContext* immediate_context, ID3D11ShaderResourceView** shader_resource_view);

	struct radial_blur_constants
	{
		DirectX::XMFLOAT2 blurCenter = { 0.5, 0.5 }; // center point where the blur is applied
		float blurStrength = 0.0f; // blurring strength
		float blurRadius = 0.5f; // blurred radius
		float blurDecay = 0.2f; // ratio of distance to decay to radius
		float blurTimer = 0.0f;
		float pads[2];

	};
	std::unique_ptr<Constants<radial_blur_constants>> radial_blur_constant{};
private:
	//	ラジアルブラー
	Microsoft::WRL::ComPtr<ID3D11Buffer> radial_blur_constant_buffer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> radial_blur_sampler_state;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> radial_blur_pixel_shader;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> radial_blur_shader_resource_view;

	std::unique_ptr<fullscreen_quad> radial_quad;

	bool displayRadialBlurImgui = false;
};


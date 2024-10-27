#pragma once
#include <DirectXMath.h>
#include "graphics.h"
#include "sprite.h"
#include "constant.h"

class RadialBlur
{
private:
	struct radial_blur_constants
	{
		DirectX::XMFLOAT2 blur_center = { 0.5, 0.5 }; // center point where the blur is applied
		float blur_strength = 1.0f; // blurring strength
		float blur_radius = 0.5f; // blurred radiu
		float blur_decay = 0.2f; // ratio of distance to decay to radius
		float pads[3];

	};
	//	ラジアルブラー
	std::unique_ptr<Constants<radial_blur_constants>> radial_blur_constant{};
	Microsoft::WRL::ComPtr<ID3D11Buffer> radial_blur_constant_buffer;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> radial_blur_pixel_shader;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> radial_blur_sampler_state;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> radial_blur_shader_resource_view;

	bool displayRadialBlurImgui = false;
public:
	RadialBlur(ID3D11Device* device);
	~RadialBlur() {}


	void DebugGUI();

	void blit(ID3D11DeviceContext* immediate_context, ID3D11PixelShader* ps);
};


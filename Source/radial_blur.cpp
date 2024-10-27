#include "radial_blur.h"
#include "user.h"

RadialBlur::RadialBlur(ID3D11Device* device)
{
	HRESULT hr{ S_OK };

	//	ラジアルブラー用定数バッファ
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(radial_blur_constants);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;
	HRESULT hr = device->CreateBuffer(&buffer_desc, nullptr, radial_blur_constant_buffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	//	ラジアルブラー用サンプラー
	D3D11_SAMPLER_DESC sampler_desc{};
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.MipLODBias = 0;
	sampler_desc.MaxAnisotropy = 16;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampler_desc.BorderColor[0] = FLT_MAX;
	sampler_desc.BorderColor[1] = FLT_MAX;
	sampler_desc.BorderColor[2] = FLT_MAX;
	sampler_desc.BorderColor[3] = FLT_MAX;
	sampler_desc.MinLOD = 0;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = device->CreateSamplerState(&sampler_desc, radial_blur_sampler_state.GetAddressOf());

	//	ラジアルブラーシェーダー
	create_ps_from_cso(device, "Shader/radial_blur_ps.cso", radial_blur_pixel_shader.GetAddressOf());

	radial_blur_constant = std::make_unique<Constants<radial_blur_constants>>(Graphics::Instance().GetDevice().Get());
}

void RadialBlur::DebugGUI()
{
#ifdef USE_IMGUI
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
	imguiMenuBar("Blur", "radial_blur", displayRadialBlurImgui);

	if (displayRadialBlurImgui)
	{

		if (ImGui::Begin("radial_blur", nullptr, ImGuiWindowFlags_None))
		{
			ImGui::SliderFloat2("center", &radial_blur_constant->data.blur_center.x, 0, 1);
			ImGui::SliderFloat("radius", &radial_blur_constant->data.blur_radius, 0.001f, +100.0f);
			ImGui::SliderFloat("mask radius", &radial_blur_constant->data.blur_mask_radius, 0, +300.0f);			ImGui::SliderInt("sampling count", &radial_blur_constant->data.blur_sampling_count, 1, 100);
		}

		ImGui::End();
	}
#endif
}

void RadialBlur::blit()
{
	Graphics& graphics = Graphics::Instance();
	//	バックバッファ指定
	graphics.Get_DC()->OMSetRenderTargets(1, graphics.GetRenderTargetView().GetAddressOf(), graphics.GetDepthStencilView().Get());

	//	スプライト描画
	{
		//	ラジアルブラー用定数バッファ
		{
			static constexpr int RadialBlurCBVIndex = 2;
			static constexpr int RadialBlurSamplerIndex = 0;
			graphics.Get_DC()->UpdateSubresource(radial_blur_constant_buffer.Get(), 0, 0, &radial_blur_constant, 0, 0);
			radial_blur_constant->Bind(graphics.Get_DC().Get(), RadialBlurCBVIndex, CB_FLAG::ALL);

			graphics.Get_DC()->VSSetSamplers(RadialBlurSamplerIndex, 1, radial_blur_sampler_state.GetAddressOf());
			graphics.Get_DC()->HSSetSamplers(RadialBlurSamplerIndex, 1, radial_blur_sampler_state.GetAddressOf());
			graphics.Get_DC()->DSSetSamplers(RadialBlurSamplerIndex, 1, radial_blur_sampler_state.GetAddressOf());
			graphics.Get_DC()->GSSetSamplers(RadialBlurSamplerIndex, 1, radial_blur_sampler_state.GetAddressOf());
			graphics.Get_DC()->PSSetSamplers(RadialBlurSamplerIndex, 1, radial_blur_sampler_state.GetAddressOf());
			graphics.Get_DC()->CSSetSamplers(RadialBlurSamplerIndex, 1, radial_blur_sampler_state.GetAddressOf());
		}
		graphics.Get_DC()->PSSetShader(radial_blur_pixel_shader.Get(), nullptr, 0);

		//write_scene_render_sprite->render(get_renderdata()->immediate_context.Get(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	}

}

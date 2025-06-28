#include "radial_blur.h"
#include "user.h"

RadialBlur::RadialBlur(ID3D11Device* device)
{
	//	ラジアルブラー用定数バッファ
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(radialBlurConstants);
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

	radial_quad = std::make_unique<fullscreen_quad>(Graphics::Instance().GetDevice().Get());

	radial_blur_constant = std::make_unique<Constants<radialBlurConstants>>(Graphics::Instance().GetDevice().Get());
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
			ImGui::Checkbox("isDebug", &isDebug);

			ImGui::DragFloat2("blurCenter", &radial_blur_constant->data.blurCenter.x, 0.01f);
			ImGui::SliderFloat("blurStrength", &radial_blur_constant->data.blurStrength, +0.0f, +1.0f);
			ImGui::SliderFloat("blurRadius", &radial_blur_constant->data.blurRadius, +0.0f, +1.0f);
			ImGui::SliderFloat("blurDecay", &radial_blur_constant->data.blurDecay, +0.0f, +1.0f);
			ImGui::DragFloat("blurTimer", &radial_blur_constant->data.blurTimer);
		}

		ImGui::End();
	}
#endif
}

void RadialBlur::Blit(ID3D11DeviceContext* immediate_context, ID3D11ShaderResourceView** shader_resource_view)
{
	Graphics& graphics = Graphics::Instance();
	//	バックバッファ指定
	//immediate_context->OMSetRenderTargets(1, graphics.GetRenderTargetView().GetAddressOf(), graphics.GetDepthStencilView().Get());

	immediate_context->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
	immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	immediate_context->IASetInputLayout(NULL);

	immediate_context->PSSetShader(radial_blur_pixel_shader.Get(), 0, 0);

	//	ラジアルブラー用定数バッファ
	{
		static constexpr int RadialBlurCBVIndex = 2;
		radial_blur_constant->Bind(graphics.Get_DC().Get(), RadialBlurCBVIndex, CB_FLAG::PS);
	}
	radial_quad->Blit(immediate_context, shader_resource_view, 0, 2, radial_blur_pixel_shader.Get());
}

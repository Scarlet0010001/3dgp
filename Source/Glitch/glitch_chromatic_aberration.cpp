#include "glitch_chromatic_aberration.h"
#include "User/user.h"
#include "Sprite/texture.h"

Glitch_CA::Glitch_CA(ID3D11Device* device)
{
	//	ラジアルブラー用定数バッファ
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(Glitch_CA_constants);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;
	HRESULT hr = device->CreateBuffer(&buffer_desc, nullptr, glitch_CA_ConstantBuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	//	ラジアルブラー用サンプラー
	D3D11_SAMPLER_DESC sampler_desc{};
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.BorderColor[0] = 0;
	sampler_desc.BorderColor[1] = 0;
	sampler_desc.BorderColor[2] = 0;
	sampler_desc.BorderColor[3] = 0;
	hr = device->CreateSamplerState(&sampler_desc, glitch_CA_SamplerState.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	//画像セット
	{
		D3D11_TEXTURE2D_DESC texture2d_desc{};
		load_texture_from_file(device, L"Resources/Sprite/glitch/Noise.png", glitch_CA_ShaderResourceView[0].GetAddressOf(), &texture2d_desc);
		load_texture_from_file(device, L"Resources/Sprite/glitch/noise1.png", glitch_CA_ShaderResourceView[1].GetAddressOf(), &texture2d_desc);
	}

	//	ラジアルブラーシェーダー
	create_ps_from_cso(device, "Shader/glitch_chromatic_aberration.cso", glitch_CA_PixelShader.GetAddressOf());

	glitch_CA_Quad = std::make_unique<fullscreen_quad>(Graphics::Instance().GetDevice().Get());

	glitch_CA_constant = std::make_unique<Constants<Glitch_CA_constants>>(Graphics::Instance().GetDevice().Get());

}

void Glitch_CA::DebugGUI()
{
#ifdef USE_IMGUI
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
	ImguiMenuBar("Glitch", "Chromatic Aberration", displayGlitch_CA_Imgui);

	if (displayGlitch_CA_Imgui)
	{

		if (ImGui::Begin("Chromatic Aberration", nullptr, ImGuiWindowFlags_None))
		{
			ImGui::Checkbox("isDebug", &isDebug);

			if (ImGui::CollapsingHeader("Paramator", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::InputFloat("time", &glitch_CA_constant->data.time);
				ImGui::SliderFloat("density", &glitch_CA_constant->data.density, 0.0f, +1.0f);
			}
			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::SliderFloat("shift", &glitch_CA_constant->data.shift, 0.0f, +1.0f);
				ImGui::SliderFloat2("XShift", &glitch_CA_constant->data.XShift.x, -1.0f, 1.0f);
				ImGui::SliderFloat2("YShift", &glitch_CA_constant->data.YShift.x, -1.0f, 1.0f);
				ImGui::SliderFloat("random", &glitch_CA_constant->data.randFloat, 0.0f, 1.0f);
				ImGui::SliderFloat("XShifting", &glitch_CA_constant->data.XShifting, 0.0f, 1.0f);
				ImGui::SliderFloat("YShifting", &glitch_CA_constant->data.YShifting, 0.0f, 1.0f);
				ImGui::SliderFloat("extension", &glitch_CA_constant->data.extension, 0.0f, 1.0f);
				ImGui::SliderFloat("uv slider", &glitch_CA_constant->data.uvSlider, 0.0f, 1.0f);
				ImGui::SliderFloat("brightness", &glitch_CA_constant->data.brightness, 0.0f, 1.0f);
			}
			if (ImGui::CollapsingHeader("Mask", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::SliderFloat("glitch mask radius", &glitch_CA_constant->data.glitchMaskRadius, 0, +1.0f);
				ImGui::SliderInt("glitch sampling count", &glitch_CA_constant->data.glitchSamplingCount, 1, 100);
				ImGui::SliderFloat2("center", &glitch_CA_constant->data.center.x, 0.0f, 1.0f);
			}
		}

		ImGui::End();
	}
#endif

}

void Glitch_CA::Blit(ID3D11DeviceContext* immediate_context, ID3D11ShaderResourceView** shader_resource_view)
{
	Graphics& graphics = Graphics::Instance();
	//	バックバッファ指定

	immediate_context->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
	immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	immediate_context->IASetInputLayout(NULL);

	ID3D11ShaderResourceView* shader_resource_views[] = {
	*shader_resource_view,
	glitch_CA_ShaderResourceView[0].Get(),
	glitch_CA_ShaderResourceView[1].Get(),
	};

	immediate_context->PSSetShader(glitch_CA_PixelShader.Get(), 0, 0);

	//	ラジアルブラー用定数バッファ
	{
		glitch_CA_constant->Bind(graphics.Get_DC().Get(), 1, CB_FLAG::PS);
	}
	glitch_CA_Quad->Blit(immediate_context, shader_resource_views, 0, _countof(shader_resource_views), glitch_CA_PixelShader.Get());

}

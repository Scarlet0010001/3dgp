#include "sky_map.h"
#include "texture.h"
#include "shader.h"

#include "misc.h"

SkyMap::SkyMap(ID3D11Device* device, const wchar_t* filename, bool generate_mips)
{
	D3D11_TEXTURE2D_DESC texture2d_desc;
	load_texture_from_file(device, filename, shader_resource_view.GetAddressOf(), &texture2d_desc);

	if (texture2d_desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE)
	{
		is_texturecube = true;
	}

	create_vs_from_cso(device, "Shader/sky_map_vs.cso", skymap_vs.GetAddressOf(), NULL, NULL, 0);
	create_ps_from_cso(device, "Shader/sky_map_ps.cso", skymap_ps.GetAddressOf());
	create_ps_from_cso(device, "Shader/sky_box_ps.cso", skybox_ps.GetAddressOf());

	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(constants);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;
	HRESULT hr = device->CreateBuffer(&buffer_desc, nullptr, constant_buffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void SkyMap::blit(ID3D11DeviceContext* immediate_context, const DirectX::XMFLOAT4X4& view_projection)
{
	immediate_context->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
	immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	immediate_context->IASetInputLayout(NULL);

	immediate_context->VSSetShader(skymap_vs.Get(), 0, 0);
	immediate_context->PSSetShader(is_texturecube ? skybox_ps.Get() : skymap_ps.Get(), 0, 0);

	immediate_context->PSSetShaderResources(0, 1, shader_resource_view.GetAddressOf());

	constants data;
	DirectX::XMStoreFloat4x4(&data.inverse_view_projection, DirectX::XMMatrixInverse(NULL, DirectX::XMLoadFloat4x4(&view_projection)));

	immediate_context->UpdateSubresource(constant_buffer.Get(), 0, 0, &data, 0, 0);
	immediate_context->PSSetConstantBuffers(0, 1, constant_buffer.GetAddressOf());

	immediate_context->Draw(4, 0);

	immediate_context->VSSetShader(NULL, 0, 0);
	immediate_context->PSSetShader(NULL, 0, 0);
}

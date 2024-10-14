#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <directxmath.h>

class SkyMap
{
public:
	SkyMap(ID3D11Device* device, const wchar_t* filename, bool generate_mips = false);
	virtual ~SkyMap() = default;
	SkyMap(const SkyMap&) = delete;
	SkyMap& operator =(const SkyMap&) = delete;
	SkyMap(SkyMap&&) noexcept = delete;
	SkyMap& operator =(SkyMap&&) noexcept = delete;

	void blit(ID3D11DeviceContext* immediate_context, const DirectX::XMFLOAT4X4& view_projection);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view;

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> skymap_vs;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> skymap_ps;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> skybox_ps;

	struct constants
	{
		DirectX::XMFLOAT4X4 inverse_view_projection;
	};
	Microsoft::WRL::ComPtr<ID3D11Buffer> constant_buffer;

	bool is_texturecube = false;

};
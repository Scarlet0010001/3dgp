#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <directxmath.h>

#include <vector>
#include <functional>

#include "constant.h"

// https://learnopengl.com/Guest-Articles/2021/CSM
// https://learn.microsoft.com/en-us/windows/win32/dxtecharts/cascaded-shadow-maps
// https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-10-parallel-split-shadow-maps-programmable-gpus
class cascaded_shadow_map
{
public:
	cascaded_shadow_map(ID3D11Device* device, uint32_t width, uint32_t height);
	virtual ~cascaded_shadow_map() = default;
	cascaded_shadow_map(const cascaded_shadow_map&) = delete;
	cascaded_shadow_map& operator =(const cascaded_shadow_map&) = delete;
	cascaded_shadow_map(cascaded_shadow_map&&) noexcept = delete;
	cascaded_shadow_map& operator =(cascaded_shadow_map&&) noexcept = delete;

	void make(ID3D11DeviceContext* immediate_context,
		const DirectX::XMFLOAT4X4& camera_view,
		const DirectX::XMFLOAT4X4& camera_projection,
		const DirectX::XMFLOAT4& light_direction,
		float critical_depth_value, // If this value is 0, the camera's far panel distance is used.
		std::function<void()> drawcallback);

	Microsoft::WRL::ComPtr<ID3D11Texture2D> depth_stencil_buffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _depth_stencil_view;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _shader_resource_view;
	D3D11_VIEWPORT _viewport;

	std::vector<DirectX::XMFLOAT4X4> _view_projection;
	std::vector<float> _distances;


	struct constants
	{
		DirectX::XMFLOAT4X4 view_projection_matrices[4];
		float cascade_plane_distances[4];
	};
	std::unique_ptr<Constants<constants>> _constants;



public:
	const size_t _cascade_count;
	float _split_scheme_weight = 0.82f; // logarithmic_split_scheme * _split_scheme_weight + uniform_split_scheme * (1 - _split_scheme_weight)
};

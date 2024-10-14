#pragma once
#include "graphics.h"
#include "g_buffer.h"
#include "fullscreen_quad.h"
#include "framebuffer.h"
#include "light_manager.h"

class DeferredRenderer
{
public:
		DeferredRenderer();
		~DeferredRenderer() {}


		void Active();
		void Deactive();
		void Render();
	#if CAST_SHADOW
		void ShadowActive(DirectX::XMFLOAT3 target_pos);
		void ShadowDeactive();
	#endif
		void Lighting() const;

		ID3D11DepthStencilView* get_dsv() { return depth_stencil_view.Get(); }
	private:
		void DepthStencilCreate(ID3D11Device* device, DXGI_FORMAT format);

	std::unique_ptr<GBuffer> g_color;
	std::unique_ptr<GBuffer> g_depth;
	std::unique_ptr<GBuffer> g_normal;
	std::unique_ptr<GBuffer> g_position;
	std::unique_ptr<GBuffer> g_metal_smooth;
	std::unique_ptr<GBuffer> g_emissive;
	std::unique_ptr<fullscreen_quad> deferred_screen;


	std::unique_ptr<GBuffer> l_light;
	std::unique_ptr<GBuffer> l_composite;

	Microsoft::WRL::ComPtr<ID3D11PixelShader> final_sprite_ps;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> deferred_env_light;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> deferred_composite_light;

	//シャドウマップ
#if CAST_SHADOW
	float scale = -90;
	float vie = 140;
	float FarZ = 200;

	struct SHADOW_CONSTANTS
	{
		DirectX::XMFLOAT4X4	shadowVP;
	};
	std::unique_ptr<Constants<SHADOW_CONSTANTS>> shadow_constants{};
	std::unique_ptr<framebuffer> shadow_frame_buffer;
#endif
	//深度ステンシルビュー
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depth_stencil_view;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> cached_render_target_view;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> cached_depth_stencil_view;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> env_texture;

	bool displayImgui = false;

};


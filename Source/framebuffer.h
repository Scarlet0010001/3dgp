#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <cstdint>

#include <DirectXMath.h>

//FrameBufferのどの情報を使うか（カラーマップ、デプスマップ、ステンシルマップ）
enum class FB_FLAG : uint8_t
{
    COLOR = 0x01,
    DEPTH = 0x02,
    STENCIL = 0x04,
    COLOR_DEPTH_STENCIL = COLOR | DEPTH | STENCIL,
    COLOR_DEPTH = COLOR | DEPTH,
    DEPTH_STENCIL = DEPTH | STENCIL,
};

class framebuffer
{
public:
    framebuffer(ID3D11Device* device, uint32_t width, uint32_t height);
    virtual ~framebuffer() = default;

    //Microsoft::WRL::ComPtr<ID3D11RenderTargetView> render_target_view;
    //Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depth_stencil_view;
    //Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_views[2];
    //D3D11_VIEWPORT viewport;

    void clear(ID3D11DeviceContext* immediate_context,
        FB_FLAG flags = FB_FLAG::COLOR_DEPTH_STENCIL, DirectX::XMFLOAT4 color = { 0, 0, 0, 1 }, float depth = 1, uint8_t stencil = 0);
    void activate(ID3D11DeviceContext* immediate_context, FB_FLAG flags = FB_FLAG::COLOR_DEPTH_STENCIL);
    void deactivate(ID3D11DeviceContext* immediate_context);

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& get_color_map() { return shader_resource_views[0]; }
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& depth_map() { return shader_resource_views[1]; }
    const UINT get_tex_width() { return tex_size[0]; }
    const UINT get_tex_height() { return tex_size[1]; }

private:
    UINT tex_size[2] = {};

    UINT viewport_count{ D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE };
    D3D11_VIEWPORT cached_viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> cached_render_target_view;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> cached_depth_stencil_view;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture_2d;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> render_target_view;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depth_stencil_view;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_views[2];
    D3D11_VIEWPORT viewport;

};
#pragma once
#include <d3d11.h>
#include <directxmath.h>
#include <string>
#include "inline.h"

class Sprite
{
public:
    Sprite(ID3D11Device* device, const wchar_t* filename);
    ~Sprite();
    void render(ID3D11DeviceContext*,
        float dx, float dy, // 矩形の左上の座標（スクリーン座標系）
        float dw, float dh, // 矩形のサイズ（スクリーン座標系）
        float r, float g, float b, float a, //色合い
        float angle //角度
    );
    void render(ID3D11DeviceContext* immediate_context,
        float dx, float dy, // 矩形の左上の座標（スクリーン座標系）
        float dw, float dh, // 矩形のサイズ（スクリーン座標系）
        float r, float g, float b, float a, //色合い
        float angle, // 角度
        float sx, float sy, float sw, float sh
    );
    void render(ID3D11DeviceContext* immediate_context,
        float dx, float dy, // 矩形の左上の座標（スクリーン座標系）
        float dw, float dh  // 矩形のサイズ（スクリーン座標系）
    );

    void textout(ID3D11DeviceContext* immediate_context,
        std::string s,
        float x, float y, float w, float h, 
        float r, float g, float b, float a);

        
private:
    ID3D11VertexShader* vertex_shader;
    ID3D11PixelShader* pixel_shader;
    ID3D11InputLayout* input_layout;
    ID3D11Buffer* vertex_buffer;

    ID3D11ShaderResourceView* shader_resource_view;
    D3D11_TEXTURE2D_DESC texture2d_desc;

    struct vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT4 color;
        DirectX::XMFLOAT2 texcoord;
    };
};

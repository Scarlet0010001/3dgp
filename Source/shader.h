#pragma once
#include <d3d11.h>
#include <wrl.h>
//#include <string>
//#include<map>
#include "misc.h"
#include <memory>

class Shader
{
public:
    Shader() {}
    Shader(ID3D11Device* device) {}
    virtual ~Shader() {}

    // 描画開始
    virtual void Active(ID3D11DeviceContext* immediate_context);

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> f_pixel_shader;//デフォルトはフォワードレンダリング
};


HRESULT create_vs_from_cso(Microsoft::WRL::ComPtr<ID3D11Device> device,
    const char* cso_name,
    ID3D11VertexShader** vertex_shader,
    ID3D11InputLayout** input_layout,
    D3D11_INPUT_ELEMENT_DESC* input_element_desc,
    UINT num_elements);

HRESULT create_ps_from_cso(Microsoft::WRL::ComPtr<ID3D11Device> device,
    const char* cso_name,
    ID3D11PixelShader** pixel_shader);

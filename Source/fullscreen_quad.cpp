#include "fullscreen_quad.h"
#include "shader.h"
#include "misc.h"

fullscreen_quad::fullscreen_quad(ID3D11Device* device)
{
    // 頂点シェーダーのコンパイルおよび作成
    // create_vs_from_cso関数を使用して、
    //埋め込まれた頂点シェーダーのコンパイルおよび作成を行う
    create_vs_from_cso(device, "Shader/fullscreen_quad_vs.cso", embedded_vertex_shader.ReleaseAndGetAddressOf(),
        nullptr, nullptr, 0);

    // ピクセルシェーダーのコンパイルおよび作成
    // create_ps_from_cso関数を使用して、
    //埋め込まれたピクセルシェーダーのコンパイルおよび作成を行う
    create_ps_from_cso(device, "Shader/fullscreen_quad_ps.cso", embedded_pixel_shader.ReleaseAndGetAddressOf());

}

void fullscreen_quad::blit(ID3D11DeviceContext* immediate_context, ID3D11ShaderResourceView** shader_resource_view, uint32_t start_slot, uint32_t num_views, ID3D11PixelShader* replaced_pixel_shader)
{
    // 頂点バッファの設定
    immediate_context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
    // プリミティブのトポロジーを三角ストリップに設定
    immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    // 入力レイアウトの設定をクリア
    immediate_context->IASetInputLayout(nullptr);

    // 頂点シェーダーをセット
    immediate_context->VSSetShader(embedded_vertex_shader.Get(), 0, 0);
    // ピクセルシェーダーをセット
    // replaced_pixel_shader が指定されていればそれを、
    // そうでなければ埋め込まれたピクセルシェーダーを使用
    replaced_pixel_shader ? immediate_context->PSSetShader(replaced_pixel_shader, 0, 0) :
        immediate_context->PSSetShader(embedded_pixel_shader.Get(), 0, 0);

    // シェーダーリソースビューをピクセルシェーダーに設定
    immediate_context->PSSetShaderResources(start_slot, num_views, shader_resource_view);

    // フルスクリーンクワッドを描画
    immediate_context->Draw(4, 0);

}

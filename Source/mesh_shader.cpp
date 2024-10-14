#include "mesh_shader.h"

MeshShader::MeshShader(ID3D11Device* device)
{
}

void MeshShader::active(ID3D11DeviceContext* immediate_context, RenderType rt)
{
}

void MeshShader::active(ID3D11DeviceContext* immediate_context, ID3D11VertexShader* vertex_shader, ID3D11PixelShader* pixcel_shader)
{
}

void MeshShader::render(ID3D11DeviceContext* immediate_context, gltf_model* model, const DirectX::XMFLOAT4X4& world)
{
    for (gltf_model::mesh& mesh : model->meshes)
    {
        //uint32_t stride{ sizeof(gltf_model::) };
    }
}

void MeshShader::render(ID3D11DeviceContext* immediate_context, gltf_model* model, DirectX::XMFLOAT4X4 camera_view, DirectX::XMFLOAT4X4 camara_proj, const DirectX::XMFLOAT4X4& world)
{
}

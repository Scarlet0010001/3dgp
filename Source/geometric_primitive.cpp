#include "geometric_primitive.h"
#include "shader.h"
#include <vector>
#include "misc.h"

geometric_primitive::geometric_primitive(ID3D11Device* device)
{
	vertex vertices[24]{};
	// �T�C�Y�� 1.0 �̐������̃f�[�^���쐬����i�d�S�����_�ɂ���j�B�������̂̃R���g���[���|�C���g���� 8 �A
	// 1 �̃R���g���[���|�C���g�̈ʒu�ɂ͖@���̌������Ⴄ���_�� 3 ����̂Œ��_���̑����� 8x3=24 �A
	// ���_���z��ivertices�j�ɂ��ׂĒ��_�̈ʒu�E�@�������i�[����B

	uint32_t indices[36]{};
	// �������̂� 6 �ʎ����A1 �̖ʂ� 2 �� 3 �p�`�|���S���ō\�������̂� 3 �p�`�|���S���̑����� 6x2=12 �A
	// �������̂�`�悷�邽�߂� 12 ��� 3 �p�`�|���S���`�悪�K�v�A����ĎQ�Ƃ���钸�_���� 12x3=36 ��A
	// 3 �p�`�|���S�����Q�Ƃ��钸�_���̃C���f�b�N�X�i���_�ԍ��j��`�揇�ɔz��iindices�j�Ɋi�[����B
	// ���v��肪�\�ʂɂȂ�悤�Ɋi�[���邱�ƁB

	uint32_t face{ 0 };
	// top-side
	// 0---------1
	// |         |
	// |   -Y    |
	// |         |
	// 2---------3
	face = 0;
	vertices[face * 4 + 0].position = { -0.5f, +0.5f, +0.5f };
	vertices[face * 4 + 1].position = { +0.5f, +0.5f, +0.5f };
	vertices[face * 4 + 2].position = { -0.5f, +0.5f, -0.5f };
	vertices[face * 4 + 3].position = { +0.5f, +0.5f, -0.5f };
	vertices[face * 4 + 0].normal = { +0.0f, +1.0f, +0.0f };
	vertices[face * 4 + 1].normal = { +0.0f, +1.0f, +0.0f };
	vertices[face * 4 + 2].normal = { +0.0f, +1.0f, +0.0f };
	vertices[face * 4 + 3].normal = { +0.0f, +1.0f, +0.0f };
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 1;
	indices[face * 6 + 2] = face * 4 + 2;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 3;
	indices[face * 6 + 5] = face * 4 + 2;

	// bottom-side
	// 0---------1
	// |         |
	// |   -Y    |
	// |         |
	// 2---------3
	face += 1;
	vertices[face * 4 + 0].position = { -0.5f, -0.5f, +0.5f };
	vertices[face * 4 + 1].position = { +0.5f, -0.5f, +0.5f };
	vertices[face * 4 + 2].position = { -0.5f, -0.5f, -0.5f };
	vertices[face * 4 + 3].position = { +0.5f, -0.5f, -0.5f };
	vertices[face * 4 + 0].normal = { +0.0f, -1.0f, +0.0f };
	vertices[face * 4 + 1].normal = { +0.0f, -1.0f, +0.0f };
	vertices[face * 4 + 2].normal = { +0.0f, -1.0f, +0.0f };
	vertices[face * 4 + 3].normal = { +0.0f, -1.0f, +0.0f };
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 2;
	indices[face * 6 + 2] = face * 4 + 1;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 2;
	indices[face * 6 + 5] = face * 4 + 3;

	// front-side
	// 0---------1
	// |         |
	// |   +Z    |
	// |         |
	// 2---------3
	face += 1;
	vertices[face * 4 + 0].position = { -0.5f, +0.5f, -0.5f };
	vertices[face * 4 + 1].position = { +0.5f, +0.5f, -0.5f };
	vertices[face * 4 + 2].position = { -0.5f, -0.5f, -0.5f };
	vertices[face * 4 + 3].position = { +0.5f, -0.5f, -0.5f };
	vertices[face * 4 + 0].normal = { +0.0f, +0.0f, -1.0f };
	vertices[face * 4 + 1].normal = { +0.0f, +0.0f, -1.0f };
	vertices[face * 4 + 2].normal = { +0.0f, +0.0f, -1.0f };
	vertices[face * 4 + 3].normal = { +0.0f, +0.0f, -1.0f };
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 1;
	indices[face * 6 + 2] = face * 4 + 2;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 3;
	indices[face * 6 + 5] = face * 4 + 2;

	// back-side
	// 0---------1
	// |         |
	// |   +Z    |
	// |         |
	// 2---------3
	face += 1;
	vertices[face * 4 + 0].position = { -0.5f, +0.5f, +0.5f };
	vertices[face * 4 + 1].position = { +0.5f, +0.5f, +0.5f };
	vertices[face * 4 + 2].position = { -0.5f, -0.5f, +0.5f };
	vertices[face * 4 + 3].position = { +0.5f, -0.5f, +0.5f };
	vertices[face * 4 + 0].normal = { +0.0f, +0.0f, +1.0f };
	vertices[face * 4 + 1].normal = { +0.0f, +0.0f, +1.0f };
	vertices[face * 4 + 2].normal = { +0.0f, +0.0f, +1.0f };
	vertices[face * 4 + 3].normal = { +0.0f, +0.0f, +1.0f };
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 2;
	indices[face * 6 + 2] = face * 4 + 1;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 2;
	indices[face * 6 + 5] = face * 4 + 3;

	// right-side
	// 0---------1
	// |         |      
	// |   -X    |
	// |         |
	// 2---------3
	face += 1;
	vertices[face * 4 + 0].position = { +0.5f, +0.5f, -0.5f };
	vertices[face * 4 + 1].position = { +0.5f, +0.5f, +0.5f };
	vertices[face * 4 + 2].position = { +0.5f, -0.5f, -0.5f };
	vertices[face * 4 + 3].position = { +0.5f, -0.5f, +0.5f };
	vertices[face * 4 + 0].normal = { +1.0f, +0.0f, +0.0f };
	vertices[face * 4 + 1].normal = { +1.0f, +0.0f, +0.0f };
	vertices[face * 4 + 2].normal = { +1.0f, +0.0f, +0.0f };
	vertices[face * 4 + 3].normal = { +1.0f, +0.0f, +0.0f };
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 1;
	indices[face * 6 + 2] = face * 4 + 2;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 3;
	indices[face * 6 + 5] = face * 4 + 2;

	// left-side
	// 0---------1
	// |         |      
	// |   -X    |
	// |         |
	// 2---------3
	face += 1;
	vertices[face * 4 + 0].position = { -0.5f, +0.5f, -0.5f };
	vertices[face * 4 + 1].position = { -0.5f, +0.5f, +0.5f };
	vertices[face * 4 + 2].position = { -0.5f, -0.5f, -0.5f };
	vertices[face * 4 + 3].position = { -0.5f, -0.5f, +0.5f };
	vertices[face * 4 + 0].normal = { -1.0f, +0.0f, +0.0f };
	vertices[face * 4 + 1].normal = { -1.0f, +0.0f, +0.0f };
	vertices[face * 4 + 2].normal = { -1.0f, +0.0f, +0.0f };
	vertices[face * 4 + 3].normal = { -1.0f, +0.0f, +0.0f };
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 2;
	indices[face * 6 + 2] = face * 4 + 1;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 2;
	indices[face * 6 + 5] = face * 4 + 3;

	create_com_buffers(device, vertices, 24, indices, 36);


    HRESULT hr{ S_OK };
    D3D11_INPUT_ELEMENT_DESC input_element_desc[]
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
        D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
        D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },

    };

    create_vs_from_cso(device,
		"Shader/geometric_primitive_vs.cso", 
		vertex_shader.GetAddressOf(),
        input_layout.GetAddressOf(),
		input_element_desc,
		ARRAYSIZE(input_element_desc));
    create_ps_from_cso(device,
		"Shader/geometric_primitive_ps.cso", 
		pixel_shader.GetAddressOf());

    D3D11_BUFFER_DESC buffer_desc{};
    buffer_desc.ByteWidth = sizeof(constants);
    buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    hr = device->CreateBuffer(&buffer_desc, nullptr, constant_buffer.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void geometric_primitive::create_com_buffers(ID3D11Device* device, vertex* vertices, size_t vertex_count,
    uint32_t* indices, size_t index_count)
{
    HRESULT hr{ S_OK };
    
    D3D11_BUFFER_DESC buffer_desc{};
    D3D11_SUBRESOURCE_DATA subresource_data{};
    buffer_desc.ByteWidth = static_cast<UINT>(sizeof(vertex) * vertex_count);
    buffer_desc.Usage = D3D11_USAGE_DEFAULT;

    buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    buffer_desc.CPUAccessFlags = 0;
    buffer_desc.MiscFlags = 0;
    buffer_desc.StructureByteStride = 0;
    subresource_data.pSysMem = vertices;
    subresource_data.SysMemPitch = 0;
    subresource_data.SysMemSlicePitch = 0;
    hr = device->CreateBuffer(&buffer_desc, &subresource_data, vertex_buffer.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    buffer_desc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * index_count);
    buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    subresource_data.pSysMem = indices;
    hr = device->CreateBuffer(&buffer_desc, &subresource_data, index_buffer.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

}

void geometric_primitive::render(ID3D11DeviceContext* immediate_context,
    const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& material_color)
{
    uint32_t stride{ sizeof(vertex) };
    uint32_t offset{ 0 };
    immediate_context->IASetVertexBuffers(0, 1, vertex_buffer.GetAddressOf(), &stride, &offset);
    immediate_context->IASetIndexBuffer(index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    immediate_context->IASetInputLayout(input_layout.Get());

    immediate_context->VSSetShader(vertex_shader.Get(), nullptr, 0);
    immediate_context->PSSetShader(pixel_shader.Get(), nullptr, 0);

    constants data{ world, material_color };
    immediate_context->UpdateSubresource(constant_buffer.Get(), 0, 0, &data, 0, 0);
    immediate_context->VSSetConstantBuffers(0, 1, constant_buffer.GetAddressOf());

    D3D11_BUFFER_DESC buffer_desc{};
    index_buffer->GetDesc(&buffer_desc);
    immediate_context->DrawIndexed(buffer_desc.ByteWidth / sizeof(uint32_t), 0, 0);

    
}

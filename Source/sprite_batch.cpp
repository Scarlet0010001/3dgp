#include "sprite_batch.h"
#include "misc.h"
#include <sstream>
#include <wrl.h>
#include <WICTextureLoader.h>

#if _DEBUG
CONST LONG SCREEN_WIDTH{ 1280 };
CONST LONG SCREEN_HEIGHT{ 720 };
#else
CONST LONG SCREEN_WIDTH{ 1920 };
CONST LONG SCREEN_HEIGHT{ 1080 };
#endif

//�𑜓x�ɂ�鍷���̔{��
CONST FLOAT MAGNI_RESOLUTION_WIDTH{ SCREEN_WIDTH / 1280.0f };
//�𑜓x�ɂ�鍷���̔{��
CONST FLOAT MAGNI_RESOLUTION_HEIGHT{ SCREEN_HEIGHT / 720.0f };


SpriteBatch::SpriteBatch(ID3D11Device* device,
    const wchar_t* filename, size_t max_sprites)
    : max_vertices(max_sprites * 6)
{
    HRESULT hr{ S_OK };

    //���_���̃Z�b�g
    //vertex vertices[]
    //{       //  ���_�@�@�@�@�@�@�@�F��
    //    { { -1.0, +1.0, 0 }, { 1, 1, 1, 1 }, { 0, 0 } },
    //    { { +1.0, +1.0, 0 }, { 1, 1, 1, 1 }, { 1, 0 } },
    //    { { -1.0, -1.0, 0 }, { 1, 1, 1, 1 }, { 0, 1 } },
    //    { { +1.0, -1.0, 0 }, { 1, 1, 1, 1 }, { 1, 1 } },
    //};

    std::unique_ptr<vertex[]> vertices{ std::make_unique<vertex[]>(max_vertices) };


    //���_�o�b�t�@�I�u�W�F�N�g�̐���
    D3D11_BUFFER_DESC buffer_desc{};
    buffer_desc.ByteWidth = sizeof(vertex) * max_vertices;
    buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    buffer_desc.MiscFlags = 0;
    buffer_desc.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA subresource_data{};
    subresource_data.pSysMem = vertices.get();
    subresource_data.SysMemPitch = 0;
    subresource_data.SysMemSlicePitch = 0;
    hr = device->CreateBuffer(&buffer_desc, &subresource_data, &vertex_buffer);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    //���_�V�F�[�_�[�I�u�W�F�N�g�̐���
    const char* cso_name{ "Shader/sprite_vs.cso" };

    FILE* fp{};
    fopen_s(&fp, cso_name, "rb");
    _ASSERT_EXPR_A(fp, "CSO File not found");

    fseek(fp, 0, SEEK_END);
    long cso_sz{ ftell(fp) };
    fseek(fp, 0, SEEK_SET);

    std::unique_ptr<unsigned char[]> cso_data{ std::make_unique<unsigned char[]>(cso_sz) };
    fread(cso_data.get(), cso_sz, 1, fp);
    fclose(fp);

    hr = device->CreateVertexShader(cso_data.get(), cso_sz, nullptr, &vertex_shader);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    //���̓��C�A�E�g�I�u�W�F�N�g�̐���
    D3D11_INPUT_ELEMENT_DESC input_element_desc[]
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
            D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
            D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
            D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    hr = device->CreateInputLayout(input_element_desc, _countof(input_element_desc),
        cso_data.get(), cso_sz, &input_layout);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    //�s�N�Z���V�F�[�_�[�I�u�W�F�N�g�̐���
    const char* p_cso_name{ "Shader/sprite_ps.cso" };

    FILE* p_fp{};
    fopen_s(&p_fp, p_cso_name, "rb");
    _ASSERT_EXPR_A(p_fp, "CSO File not found");

    fseek(p_fp, 0, SEEK_END);
    long p_cso_sz{ ftell(p_fp) };
    fseek(p_fp, 0, SEEK_SET);

    std::unique_ptr<unsigned char[]> p_cso_data{ std::make_unique<unsigned char[]>(p_cso_sz) };
    fread(p_cso_data.get(), p_cso_sz, 1, p_fp);
    fclose(p_fp);

    hr = device->CreatePixelShader(p_cso_data.get(), p_cso_sz, nullptr, &pixel_shader);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    //�摜�t�@�C���̃��[�h�ƃV�F�[�_�[���\�[�X�r���[�I�u�W�F�N�g(ID3D11ShaderResourceView)�̐���
    Microsoft::WRL::ComPtr<ID3D11Resource> resource{};
    hr = DirectX::CreateWICTextureFromFile(
        device, filename, resource.GetAddressOf(),
        &shader_resource_view);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    //resource->Release();

    //�e�N�X�`�����(D3D11_TEXTURE2D_DESC)�̎擾
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d{};
    hr = resource->QueryInterface<ID3D11Texture2D>(
        texture2d.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    texture2d->GetDesc(&texture2d_desc);
    //texture2d->Release();
}

SpriteBatch::~SpriteBatch()
{
    vertex_shader->Release();
    pixel_shader->Release();
    input_layout->Release();
    shader_resource_view->Release();
    vertex_buffer->Release();
    //texture2d_desc
}

void SpriteBatch::render(ID3D11DeviceContext* immediate_context,
    float dx, float dy, // ��`�̍���̍��W�i�X�N���[�����W�n�j
    float dw, float dh, // ��`�̃T�C�Y�i�X�N���[�����W�n�j
    float r, float g, float b, float a, //�F����
    float angle // �p�x
)
{
    //�X�N���[���i�r���[�|�[�g�j�̃T�C�Y���擾����
    D3D11_VIEWPORT viewport{};
    UINT num_viewports{ 1 };
    immediate_context->RSGetViewports(&num_viewports, &viewport);

    //render �����o�֐��̈����idx, dy, dw, dh�j�����`�̊e���_�̈ʒu�i�X�N���[�����W�n�j���v�Z����
    // (x0, y0) *----* (x1, y1) 
    //          |   /|
    //          |  / |
    //          | /  |
    //          |/   |
    // (x2, y2) *----* (x3, y3) 
    // 
    // left-top
    float x0{ dx };
    float y0{ dy };
    // right-top
    float x1{ dx + dw };
    float y1{ dy };
    // left-bottom
    float x2{ dx };
    float y2{ dy + dh };
    // right-bottom
    float x3{ dx + dw };
    float y3{ dy + dh };

    //��]�̒��S����`�̒��S�_�ɂ����ꍇ
    float cx = dx + dw * 0.5f;
    float cy = dy + dh * 0.5f;
    rotate(x0, y0, cx, cy, angle);
    rotate(x1, y1, cx, cy, angle);
    rotate(x2, y2, cx, cy, angle);
    rotate(x3, y3, cx, cy, angle);

    //�X�N���[�����W�n���� NDC �ւ̍��W�ϊ��������Ȃ�
    x0 = 2.0f * x0 / viewport.Width - 1.0f;
    y0 = 1.0f - 2.0f * y0 / viewport.Height;
    x1 = 2.0f * x1 / viewport.Width - 1.0f;
    y1 = 1.0f - 2.0f * y1 / viewport.Height;
    x2 = 2.0f * x2 / viewport.Width - 1.0f;
    y2 = 1.0f - 2.0f * y2 / viewport.Height;
    x3 = 2.0f * x3 / viewport.Width - 1.0f;
    y3 = 1.0f - 2.0f * y3 / viewport.Height;

    //�e�N�X�`�����W�𒸓_�o�b�t�@�ɃZ�b�g����
    HRESULT hr{ S_OK };
    D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
    hr = immediate_context->Map(vertex_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    vertex* vertices{ reinterpret_cast<vertex*>(mapped_subresource.pData) };
    if (vertices != nullptr)
    {
        vertices[0].position = { x0, y0 , 0 };
        vertices[1].position = { x1, y1 , 0 };
        vertices[2].position = { x2, y2 , 0 };
        vertices[3].position = { x3, y3 , 0 };
        vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = { r, g, b, a };

        vertices[0].texcoord = { 0,0 };//�������g��
        vertices[1].texcoord = { 1,0 };
        vertices[2].texcoord = { 0,1 };
        vertices[3].texcoord = { 1,1 };

    }

    immediate_context->Unmap(vertex_buffer, 0);

    //���_�o�b�t�@�[�̃o�C���h
    UINT stride{ sizeof(vertex) };
    UINT offset{ 0 };
    immediate_context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);

    //�v���~�e�B�u�^�C�v����уf�[�^�̏����Ɋւ�����̃o�C���h
    immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    //�V�F�[�_�[ ���\�[�X�̃o�C���h
    immediate_context->PSSetShaderResources(0, 1, &shader_resource_view);

    //���̓��C�A�E�g�I�u�W�F�N�g�̃o�C���h
    immediate_context->IASetInputLayout(input_layout);

    //�V�F�[�_�[�̃o�C���h
    immediate_context->VSSetShader(vertex_shader, nullptr, 0);
    immediate_context->PSSetShader(pixel_shader, nullptr, 0);
    
    //�v���~�e�B�u�̕`��
    immediate_context->Draw(4, 0);

}

void SpriteBatch::render(ID3D11DeviceContext* immediate_context,
    float dx, float dy, float dw, float dh,
    float r, float g, float b, float a,
    float angle,
    float sx, float sy, float sw, float sh)
{
    //�ǂ������sx,sy,sw,sh���g���̂�
    //�X�N���[���i�r���[�|�[�g�j�̃T�C�Y���擾����
    D3D11_VIEWPORT viewport{};
    UINT num_viewports{ 1 };
    immediate_context->RSGetViewports(&num_viewports, &viewport);

    //render �����o�֐��̈����idx, dy, dw, dh�j�����`�̊e���_�̈ʒu�i�X�N���[�����W�n�j���v�Z����
    // (x0, y0) *----* (x1, y1) 
    //          |   /|
    //          |  / |
    //          | /  |
    //          |/   |
    // (x2, y2) *----* (x3, y3) 
    // 
    // left-top
    float x0{ dx };
    float y0{ dy };
    // right-top
    float x1{ dx + dw };
    float y1{ dy };
    // left-bottom
    float x2{ dx };
    float y2{ dy + dh };
    // right-bottom
    float x3{ dx + dw };
    float y3{ dy + dh };

    //��]�̒��S����`�̒��S�_�ɂ����ꍇ
    float cx = dx + dw * 0.5f;
    float cy = dy + dh * 0.5f;
    rotate(x0, y0, cx, cy, angle);
    rotate(x1, y1, cx, cy, angle);
    rotate(x2, y2, cx, cy, angle);
    rotate(x3, y3, cx, cy, angle);

    //�����Ƀe�N�Z���v�Z
    x0 = 2.0f * x0 / viewport.Width - 1.0f;
    y0 = 1.0f - 2.0f * y0 / viewport.Height;
    x1 = 2.0f * x1 / viewport.Width - 1.0f;
    y1 = 1.0f - 2.0f * y1 / viewport.Height;
    x2 = 2.0f * x2 / viewport.Width - 1.0f;
    y2 = 1.0f - 2.0f * y2 / viewport.Height;
    x3 = 2.0f * x3 / viewport.Width - 1.0f;
    y3 = 1.0f - 2.0f * y3 / viewport.Height;

    float u0{ sx / texture2d_desc.Width };
    float v0{ sy / texture2d_desc.Height };
    float u1{ (sx + sw) / texture2d_desc.Width };
    float v1{ (sy + sh) / texture2d_desc.Height };

    vertices.push_back({ { x0, y0 , 0 }, { r, g, b, a }, { u0, v0 } });
    vertices.push_back({ { x1, y1 , 0 }, { r, g, b, a }, { u1, v0 } });
    vertices.push_back({ { x2, y2 , 0 }, { r, g, b, a }, { u0, v1 } });
    vertices.push_back({ { x2, y2 , 0 }, { r, g, b, a }, { u0, v1 } });
    vertices.push_back({ { x1, y1 , 0 }, { r, g, b, a }, { u1, v0 } });
    vertices.push_back({ { x3, y3 , 0 }, { r, g, b, a }, { u1, v1 } });


}

void SpriteBatch::render(ID3D11DeviceContext* immediate_context, float dx, float dy, float dw, float dh)
{
    render(immediate_context, dx, dy, dw, dh,
        1, 1, 1, 1, 0);
}

void SpriteBatch::render(ID3D11DeviceContext* dc,
    DirectX::XMFLOAT2 position,
    //��`�̍���̍��W(�X�N���[�����W�n)
    DirectX::XMFLOAT2 scale,
    //��`�̃T�C�Y(�X�N���[�����W�n)
    DirectX::XMFLOAT4 color,
    float angle/*degree*/
)
{
    render(dc, position, scale, color, angle, { 0.0f, 0.0f }, { static_cast<float>(texture2d_desc.Width), static_cast<float>(texture2d_desc.Height) });
}

void SpriteBatch::render(ID3D11DeviceContext* dc,
    DirectX::XMFLOAT2 position, DirectX::XMFLOAT2 scale)
{
    render(dc, { position }, { scale }, { 1.0f, 1.0f, 1.0f, 1.0f }, 0.0f,
        { 0.0f, 0.0f }, { static_cast<float>(texture2d_desc.Width), static_cast<float>(texture2d_desc.Height) });

}

void SpriteBatch::render(ID3D11DeviceContext* dc,
    DirectX::XMFLOAT2 position, DirectX::XMFLOAT2 scale,
    DirectX::XMFLOAT4 color, float angle,
    DirectX::XMFLOAT2 tex_pos, DirectX::XMFLOAT2 tex_size)
{
    //�X�N���[��(�r���[�|�[�g)�̃T�C�Y���擾����
    D3D11_VIEWPORT viewport{}; //�r���[�|�[�g�̐��@�̒�`�@https://docs.microsoft.com/ja-jp/windows/win32/api/d3d11/ns-d3d11-d3d11_viewport
    UINT numViewports{ 1 };
    dc->RSGetViewports(&numViewports, &viewport); //���X�^���C�U�X�e�[�W�Ƀo�C���h���ꂽ�r���[�|�[�g�̔z����擾

    //�𑜓x�̈Ⴂ�ɂ�鍷����␳
    position = { position.x * MAGNI_RESOLUTION_WIDTH, position.y * MAGNI_RESOLUTION_HEIGHT };
    scale = { scale.x * MAGNI_RESOLUTION_WIDTH, scale.y * MAGNI_RESOLUTION_HEIGHT };

    //render�����o�֐��̈��������`�̊e���_�̈ʒu���v�Z����
    // (x0, y0) *----* (x1, y1)
    //			|   /|
    //			|  / |
    //			| /	 |
    //			|/	 |
    // (x2, y2),*----* (x3, y3)


    // left-top
    float x0{ position.x - scale.x };
    float y0{ position.y - scale.y };
    // right-top
    float x1{ position.x + (fabsf(tex_size.x) * scale.x) };
    float y1{ position.y - scale.y };
    // left-bottom
    float x2{ position.x - scale.x };
    float y2{ position.y + (fabsf(tex_size.y) * scale.y) };
    // right-bottom
    float x3{ position.x + (fabsf(tex_size.x) * scale.x) };
    float y3{ position.y + (fabsf(tex_size.y) * scale.y) };


    //��]�̒��S����`�̒��S�_�ɂ����ꍇ
    float cx = position.x + static_cast<float>(texture2d_desc.Width) * scale.x * 0.5f;
    float cy = position.y + static_cast<float>(texture2d_desc.Height) * scale.y * 0.5f;

    rotate(x0, y0, cx, cy, angle);
    rotate(x1, y1, cx, cy, angle);
    rotate(x2, y2, cx, cy, angle);
    rotate(x3, y3, cx, cy, angle);


    ////�X�N���[�����W�n����NDC�ւ̍��W�Ԋ҂��s��
    x0 = 2.0f * x0 / viewport.Width - 1.0f;
    y0 = 1.0f - 2.0f * y0 / viewport.Height;
    x1 = 2.0f * x1 / viewport.Width - 1.0f;
    y1 = 1.0f - 2.0f * y1 / viewport.Height;
    x2 = 2.0f * x2 / viewport.Width - 1.0f;
    y2 = 1.0f - 2.0f * y2 / viewport.Height;
    x3 = 2.0f * x3 / viewport.Width - 1.0f;
    y3 = 1.0f - 2.0f * y3 / viewport.Height;

    //�e�N�Z�����W�n����e�N�X�`�����W�n�ւ̕ϊ�
    float u0{ tex_pos.x / texture2d_desc.Width };
    float v0{ tex_pos.y / texture2d_desc.Height };

    float u1{ (tex_pos.x + tex_size.x) / texture2d_desc.Width };
    float v1{ (tex_pos.y + tex_size.y) / texture2d_desc.Height };

    vertices.push_back({ {x0, y0, 0}, color, {u0, v0} });
    vertices.push_back({ {x1, y1, 0}, color, {u1, v0} });
    vertices.push_back({ {x2, y2, 0}, color, {u0, v1} });
    vertices.push_back({ {x2, y2, 0}, color, {u0, v1} });
    vertices.push_back({ {x1, y1, 0}, color, {u1, v0} });
    vertices.push_back({ {x3, y3, 0}, color, {u1, v1} });

}

void SpriteBatch::render(ID3D11DeviceContext* dc, DirectX::XMFLOAT2 position, DirectX::XMFLOAT2 scale, DirectX::XMFLOAT2 pivot, DirectX::XMFLOAT4 color, float angle, DirectX::XMFLOAT2 texpos, DirectX::XMFLOAT2 texsize)
{
    D3D11_VIEWPORT viewport{};
    UINT num_viewports{ 1 };
    dc->RSGetViewports(&num_viewports, &viewport);

    position = { position.x * MAGNI_RESOLUTION_WIDTH, position.y * MAGNI_RESOLUTION_HEIGHT };
    scale = { scale.x * MAGNI_RESOLUTION_WIDTH, scale.y * MAGNI_RESOLUTION_HEIGHT };

    // render�����o�֐��̈����idx, dy, dw, dh�j�����`�̊e���_�̈ʒu�i�X�N���[�����W�n�j���v�Z����
    //  (x0, y0) *----* (x1, y1)
    //           |   /|
    //           |  / |
    //           | /  |
    //           |/   |
    //  (x2, y2) *----* (x3, y3)

    // left-top
    float x0{ position.x - (pivot.x * scale.x) };
    float y0{ position.y - (pivot.y * scale.y) };
    // right-top
    float x1{ position.x + ((fabsf(texsize.x) - pivot.x) * scale.x) };
    float y1{ position.y - (pivot.y * scale.y) };
    // left-bottom
    float x2{ position.x - (pivot.x * scale.x) };
    float y2{ position.y + ((fabsf(texsize.y) - pivot.y) * scale.y) };
    // right-bottom
    float x3{ position.x + ((fabsf(texsize.x) - pivot.x) * scale.x) };
    float y3{ position.y + ((fabsf(texsize.y) - pivot.y) * scale.y) };

    //��]�̒��S����`�̒��S�_�ɂ����ꍇ
    float cx = position.x;
    float cy = position.y;
    rotate(x0, y0, cx, cy, angle);
    rotate(x1, y1, cx, cy, angle);
    rotate(x2, y2, cx, cy, angle);
    rotate(x3, y3, cx, cy, angle);

    // Convert to NDC space
    x0 = 2.0f * x0 / viewport.Width - 1.0f;
    y0 = 1.0f - 2.0f * y0 / viewport.Height;
    x1 = 2.0f * x1 / viewport.Width - 1.0f;
    y1 = 1.0f - 2.0f * y1 / viewport.Height;
    x2 = 2.0f * x2 / viewport.Width - 1.0f;
    y2 = 1.0f - 2.0f * y2 / viewport.Height;
    x3 = 2.0f * x3 / viewport.Width - 1.0f;
    y3 = 1.0f - 2.0f * y3 / viewport.Height;

    float u0{ texpos.x / texture2d_desc.Width };
    float v0{ texpos.y / texture2d_desc.Height };
    float u1{ (texpos.x + texsize.x) / texture2d_desc.Width };
    float v1{ (texpos.y + texsize.y) / texture2d_desc.Height };

    if (scale.x >= 0)
    {
        vertices.push_back({ { x0, y0 , 0 }, { color.x, color.y, color.z, color.w }, { u0, v0 } });
        vertices.push_back({ { x1, y1 , 0 }, { color.x, color.y, color.z, color.w }, { u1, v0 } });
        vertices.push_back({ { x2, y2 , 0 }, { color.x, color.y, color.z, color.w }, { u0, v1 } });
        vertices.push_back({ { x2, y2 , 0 }, { color.x, color.y, color.z, color.w }, { u0, v1 } });
        vertices.push_back({ { x1, y1 , 0 }, { color.x, color.y, color.z, color.w }, { u1, v0 } });
        vertices.push_back({ { x3, y3 , 0 }, { color.x, color.y, color.z, color.w }, { u1, v1 } });
    }
    else
    {
        vertices.push_back({ { x1, y1 , 0 }, { color.x, color.y, color.z, color.w }, { u1, v0 } });
        vertices.push_back({ { x0, y0 , 0 }, { color.x, color.y, color.z, color.w }, { u0, v0 } });
        vertices.push_back({ { x3, y3 , 0 }, { color.x, color.y, color.z, color.w }, { u1, v1 } });
        vertices.push_back({ { x3, y3 , 0 }, { color.x, color.y, color.z, color.w }, { u1, v1 } });
        vertices.push_back({ { x0, y0 , 0 }, { color.x, color.y, color.z, color.w }, { u0, v0 } });
        vertices.push_back({ { x2, y2 , 0 }, { color.x, color.y, color.z, color.w }, { u0, v1 } });
    }

}

void SpriteBatch::begin(ID3D11DeviceContext* immediate_context)
{
    vertices.clear();
    immediate_context->VSSetShader(vertex_shader, nullptr, 0);
    immediate_context->PSSetShader(pixel_shader, nullptr, 0);
    immediate_context->PSSetShaderResources(0, 1, &shader_resource_view);
}

void SpriteBatch::end(ID3D11DeviceContext* immediate_context)
{

    //�e�N�X�`�����W�𒸓_�o�b�t�@�ɃZ�b�g����
    HRESULT hr{ S_OK };
    D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
    hr = immediate_context->Map(vertex_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));


    size_t vertex_count = vertices.size();
    _ASSERT_EXPR(max_vertices >= vertex_count, "Buffer overflow");
    vertex * data{ reinterpret_cast<vertex*>(mapped_subresource.pData) };
    if (data != nullptr)
    {
        const vertex * p = vertices.data();
        memcpy_s(data, max_vertices * sizeof(vertex), p, vertex_count * sizeof(vertex));
    }

    immediate_context->Unmap(vertex_buffer, 0);

    //���_�o�b�t�@�[�̃o�C���h
    UINT stride{ sizeof(vertex) };
    UINT offset{ 0 };
    immediate_context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);

    //�v���~�e�B�u�^�C�v����уf�[�^�̏����Ɋւ�����̃o�C���h
    immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //�V�F�[�_�[ ���\�[�X�̃o�C���h
    immediate_context->PSSetShaderResources(0, 1, &shader_resource_view);

    //���̓��C�A�E�g�I�u�W�F�N�g�̃o�C���h
    immediate_context->IASetInputLayout(input_layout);

    //�V�F�[�_�[�̃o�C���h
    immediate_context->VSSetShader(vertex_shader, nullptr, 0);
    immediate_context->PSSetShader(pixel_shader, nullptr, 0);

    //�v���~�e�B�u�̕`��
    immediate_context->Draw(static_cast<UINT>(vertex_count), 0);


}

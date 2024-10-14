#include "sprite.h"
#include "texture.h"
#include "misc.h"
#include <sstream>
#include <WICTextureLoader.h>
#include <wrl.h>


Sprite::Sprite(ID3D11Device* device,
    const wchar_t* filename)
{
    //���_���̃Z�b�g
    vertex vertices[]
    {       //  ���_�@�@�@�@�@�@�@�F��
        { { -1.0, +1.0, 0 }, { 1, 1, 1, 1 }, { 0, 0 } },
        { { +1.0, +1.0, 0 }, { 1, 1, 1, 1 }, { 1, 0 } },
        { { -1.0, -1.0, 0 }, { 1, 1, 1, 1 }, { 0, 1 } },
        { { +1.0, -1.0, 0 }, { 1, 1, 1, 1 }, { 1, 1 } }, };

    HRESULT hr{ S_OK };

    //���_�o�b�t�@�I�u�W�F�N�g�̐���
    D3D11_BUFFER_DESC buffer_desc{};
    buffer_desc.ByteWidth = sizeof(vertices);
    buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    //buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    //buffer_desc.CPUAccessFlags = 0;
    buffer_desc.MiscFlags = 0;
    buffer_desc.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA subresource_data{};
    subresource_data.pSysMem = vertices;
    subresource_data.SysMemPitch = 0;
    subresource_data.SysMemSlicePitch = 0;
    hr = device->CreateBuffer(
        &buffer_desc, &subresource_data, &vertex_buffer);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    
    //���_�V�F�[�_�[�I�u�W�F�N�g�̐���
    const char* cso_name{ "sprite_vs.cso" };
    
    FILE * fp{};
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
   const char* p_cso_name{ "sprite_ps.cso" };

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

   /*
   UNIT10
       HRESULT hr{ S_OK };
    ComPtr<ID3D11Resource> resource;
    
    auto it = resources.find(filename);
    if (it != resources.end())
    {
        *shader_resource_view = it->second.Get();
        (*shader_resource_view)->AddRef();
        (*shader_resource_view)->GetResource(resource.GetAddressOf());
    }
    else
    {
        hr = CreateWICTextureFromFile(device, filename, resource.GetAddressOf(), shader_resource_view);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        resources.insert(make_pair(filename, *shader_resource_view));
    }
        
    ComPtr<ID3D11Texture2D> texture2d;
    hr = resource.Get()->QueryInterface<ID3D11Texture2D>(texture2d.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    texture2d->GetDesc(texture2d_desc);

   */

   //Microsoft::WRL::ComPtr<>�������Ŏg����
   //�����s���̃G���[���o��

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

Sprite::~Sprite()
{
}

//auto rotate = [](float& x, float& y,
//    float& cx, float& cy,
//    float angle
//    )
//{
//    x -= cx;
//    y -= cy;
//
//    float cos{ cosf(DirectX::XMConvertToRadians(angle)) };
//    float sin{ sinf(DirectX::XMConvertToRadians(angle)) };
//    float tx{ x }, ty{ y };
//
//    x = cos * tx + -sin * ty;
//    y = sin * tx + cos * ty;
//
//    x += cx;
//    y += cy;
//};

void Sprite::render(ID3D11DeviceContext* immediate_context,
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
    
    vertex * vertices{ reinterpret_cast<vertex*>(mapped_subresource.pData) };
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

void Sprite::render(ID3D11DeviceContext* immediate_context, 
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

    float tx0 = 0.0f;
    float ty0 = 0.0f;

    float tx1 = 1.0f;
    float ty1 = 0.0f;

    float tx2 = 0.0f;
    float ty2 = 1.0f;

    float tx3 = 1.0f;
    float ty3 = 1.0f;

    tx0 = sx / texture2d_desc.Width;
    ty0 = sy / texture2d_desc.Height;
    tx1 = (sx + sw) / texture2d_desc.Width;
    ty1 = sy / texture2d_desc.Height;
    tx2 = sx / texture2d_desc.Width;
    ty2 = (sy + sh) / texture2d_desc.Height;
    tx3 = (sx + sw) / texture2d_desc.Width;
    ty3 = (sy + sh) / texture2d_desc.Height;


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

        vertices[0].texcoord = { tx0,ty0 };//�������g��
        vertices[1].texcoord = { tx1,ty1 };
        vertices[2].texcoord = { tx2,ty2 };
        vertices[3].texcoord = { tx3,ty3 };

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

void Sprite::render(ID3D11DeviceContext* immediate_context, float dx, float dy, float dw, float dh)
{
    render(immediate_context, dx, dy, dw, dh,
        1, 1, 1, 1, 0);

}

//�����[�X���G���[�f��
void Sprite::textout(ID3D11DeviceContext* immediate_context,
    std::string s,
    float x, float y, float w, float h,
    float r, float g, float b, float a)
{
    float sw = static_cast<float>(texture2d_desc.Width / 16);
    float sh = static_cast<float>(texture2d_desc.Height / 16);
    float carriage = 0;
    for (const char c : s)
    {
        Sprite::render(immediate_context, x + carriage, y, w, h,
            r, g, b, a, 0,
            sw * (c & 0x0F), sh * (c >> 4), sw, sh);
        carriage += w;
    }
}
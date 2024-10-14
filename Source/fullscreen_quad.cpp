#include "fullscreen_quad.h"
#include "shader.h"
#include "misc.h"

fullscreen_quad::fullscreen_quad(ID3D11Device* device)
{
    // ���_�V�F�[�_�[�̃R���p�C������э쐬
    // create_vs_from_cso�֐����g�p���āA
    //���ߍ��܂ꂽ���_�V�F�[�_�[�̃R���p�C������э쐬���s��
    create_vs_from_cso(device, "Shader/fullscreen_quad_vs.cso", embedded_vertex_shader.ReleaseAndGetAddressOf(),
        nullptr, nullptr, 0);

    // �s�N�Z���V�F�[�_�[�̃R���p�C������э쐬
    // create_ps_from_cso�֐����g�p���āA
    //���ߍ��܂ꂽ�s�N�Z���V�F�[�_�[�̃R���p�C������э쐬���s��
    create_ps_from_cso(device, "Shader/fullscreen_quad_ps.cso", embedded_pixel_shader.ReleaseAndGetAddressOf());

}

void fullscreen_quad::blit(ID3D11DeviceContext* immediate_context, ID3D11ShaderResourceView** shader_resource_view, uint32_t start_slot, uint32_t num_views, ID3D11PixelShader* replaced_pixel_shader)
{
    // ���_�o�b�t�@�̐ݒ�
    immediate_context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
    // �v���~�e�B�u�̃g�|���W�[���O�p�X�g���b�v�ɐݒ�
    immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    // ���̓��C�A�E�g�̐ݒ���N���A
    immediate_context->IASetInputLayout(nullptr);

    // ���_�V�F�[�_�[���Z�b�g
    immediate_context->VSSetShader(embedded_vertex_shader.Get(), 0, 0);
    // �s�N�Z���V�F�[�_�[���Z�b�g
    // replaced_pixel_shader ���w�肳��Ă���΂�����A
    // �����łȂ���Ζ��ߍ��܂ꂽ�s�N�Z���V�F�[�_�[���g�p
    replaced_pixel_shader ? immediate_context->PSSetShader(replaced_pixel_shader, 0, 0) :
        immediate_context->PSSetShader(embedded_pixel_shader.Get(), 0, 0);

    // �V�F�[�_�[���\�[�X�r���[���s�N�Z���V�F�[�_�[�ɐݒ�
    immediate_context->PSSetShaderResources(start_slot, num_views, shader_resource_view);

    // �t���X�N���[���N���b�h��`��
    immediate_context->Draw(4, 0);

}

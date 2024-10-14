#pragma once
#include "graphics.h"

class GBuffer
{
public:
	GBuffer() {}
	~GBuffer() {}

	//G-Buffer����
	void Create(ID3D11Device* device, DXGI_FORMAT format);


	//�����_�[�^�[�Q�b�g�r���[
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> render_target_view;
	//�V�F�[�_�[���\�[�X�r���[
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view;

	//�V�F�[�_�[���\�[�X�r���[�̃Q�b�^�[
	ID3D11ShaderResourceView* Get_srv() const { return shader_resource_view.Get(); }
	//�����_�[�^�[�Q�b�g�̃Q�b�^�[�i����\�j
	ID3D11RenderTargetView* Get_rtv() { return render_target_view.Get(); }

};


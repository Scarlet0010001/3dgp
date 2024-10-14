#include "Graphics.h"
#include "framework.h"

Graphics::~Graphics()
{
}

void Graphics::Initialize(HWND hwnd)
{
	HRESULT hr{ S_OK };

	UINT create_device_flags{ 0 };

#ifdef _DEBUG
	create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG

	D3D_FEATURE_LEVEL featureLevels{ D3D_FEATURE_LEVEL_11_0 };
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = SCREEN_WIDTH;
	swapChainDesc.BufferDesc.Height = SCREEN_HEIGHT;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = !FULLSCREEN;
	//�f�o�C�X�A�f�o�C�X�R���e�L�X�g�A�X���b�v�`�F�[���𓯎��ɐ���
	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, create_device_flags,
		&featureLevels, 1, D3D11_SDK_VERSION, &swapChainDesc,
		swapChain.GetAddressOf(), device.GetAddressOf(), NULL, immediateContext.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	//�A�����_�[�^�[�Q�b�g�r���[�̍쐬
	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer{};		//2D �e�N�X�`���[ �C���^�[�t�F�C�X:�A�\�������ꂽ�������[�ł���e�N�Z�� �f�[�^���Ǘ� https://docs.microsoft.com/ja-jp/previous-versions/direct-x/ee420038(v=vs.85)
	hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(backBuffer.GetAddressOf()));
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	hr = device->CreateRenderTargetView(backBuffer.Get(), NULL, renderTargetView.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));



	//�A�[�w�X�e���V���r���[�̍쐬

	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer{};	//
	D3D11_TEXTURE2D_DESC texture2dDesc{};	//
	texture2dDesc.Width = SCREEN_WIDTH;		//�e�N�X�`����
	texture2dDesc.Height = SCREEN_HEIGHT;	//�e�N�X�`���̍���
	texture2dDesc.MipLevels = 1;			//�e�N�X�`�����̃~�b�v�}�b�v���x���̍ő吔
	texture2dDesc.ArraySize = 1;			//�e�N�X�`���z����̃e�N�X�`���̐�
	texture2dDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;		/*�e�N�X�`���t�H�[�}�b�g
																�[�x�p��24�r�b�g�A�X�e���V���p��8�r�b�g���T�|�[�g����32�r�b�g��z�o�b�t�@�`���B*/
	texture2dDesc.SampleDesc.Count = 1;		//�e�N�X�`���̃}���`�T���v�����O�p�����[�^���w�肷��\���B
	texture2dDesc.SampleDesc.Quality = 0;	//
	texture2dDesc.Usage = D3D11_USAGE_DEFAULT;		//�e�N�X�`���̓ǂݎ�肨��я������ݕ��@�����ʂ���l
	texture2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;		//�p�C�v���C���X�e�[�W�Ƀo�C���h���邽�߂̃t���O
	texture2dDesc.CPUAccessFlags = 0;		//�������CPU�A�N�Z�X�̃^�C�v���w�肷��t���O
	texture2dDesc.MiscFlags = 0;		//���̂��܂��ʓI�ł͂Ȃ����\�[�X�I�v�V���������ʂ���t���O
	hr = device->CreateTexture2D(&texture2dDesc, NULL, depthStencilBuffer.GetAddressOf());	//
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));		//

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilVewDesc{};
	depthStencilVewDesc.Format = texture2dDesc.Format;						/*���\�[�X�f�[�^�`���iDXGI_FORMAT���Q�Ɓj
																				https://docs.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format */
	depthStencilVewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;		/*���\�[�X�̃^�C�v	https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_dsv_dimension)
																				�f�v�X�X�e���V�����\�[�X�ւ̃A�N�Z�X���@���w�肵�܂��B�l�́A���̍\���̂̋��p�̂Ɋi�[����܂��B*/
	depthStencilVewDesc.Texture2D.MipSlice = 0;								//1D�e�N�X�`���T�u���\�[�X���w�肵�܂��iD3D11_TEX1D_DSV���Q�Ɓj�B
	hr = device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilVewDesc, depthStencilView.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	//depthStencilBuffer->Release();	


	//�r���[�|�[�g�̍쐬
	D3D11_VIEWPORT viewport{};		//https://docs.microsoft.com/ja-jp/windows/win32/api/d3d11/ns-d3d11-d3d11_viewport)
	viewport.TopLeftX = 0;		//�r���[�|�[�g�̍�����X�ʒu
	viewport.TopLeftY = 0;		//�r���[�|�[�g�̏㕔��Y�ʒu
	viewport.Width = static_cast<float>(SCREEN_WIDTH);		//�r���[�|�[�g�̕�
	viewport.Height = static_cast<float>(SCREEN_HEIGHT);		//�r���[�|�[�g�̍���
	viewport.MinDepth = 0.0f;		//�r���[�|�[�g�̍ŏ��[�x
	viewport.MaxDepth = 1.0f;		//�r���[�|�[�g�̍ő�[�x
	immediateContext->RSSetViewports(1, &viewport);		//NumViewports:�o�C���h����r���[�|�[�g�̐� pViewports:�f�o�C�X�Ƀo�C���h����D3D11_VIEWPORT�\���̂̔z��


	D3D11_SAMPLER_DESC sampler_desc;	//https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_sampler_desc)
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;	//�e�N�X�`�����T���v�����O����Ƃ��Ɏg�p����t�B���^�����O���@
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;		//0����1�͈̔͊O��au�e�N�X�`�����W���������邽�߂Ɏg�p���郁�\�b�h
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;		//0����1�͈̔͊O��av�e�N�X�`�����W���������邽�߂Ɏg�p������@�B
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;		//0����1�͈̔͊O��aw�e�N�X�`�����W���������邽�߂Ɏg�p������@
	sampler_desc.MipLODBias = 0;			//�v�Z���ꂽ�~�b�v�}�b�v���x������̃I�t�Z�b�g�B
	sampler_desc.MaxAnisotropy = 16;		//�t�B���^��D3D11_FILTER_ANISOTROPIC�܂���
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;	//�T���v�����O���ꂽ�f�[�^�������̃T���v�����O���ꂽ�f�[�^�Ɣ�r����֐�
	sampler_desc.BorderColor[0] = 0.0f;		//AddressU�AAddressV�A�܂���AddressW��D3D11_TEXTURE_ADDRESS_BORDER���w�肳��Ă���ꍇ�Ɏg�p���鋫�E���̐F
	sampler_desc.BorderColor[1] = 0.0f;
	sampler_desc.BorderColor[2] = 0.0f;
	sampler_desc.BorderColor[3] = 0.0f;
	sampler_desc.MinLOD = 0;	//�A�N�Z�X���N�����v����~�b�v�}�b�v�͈͂̉����B0�͍ő傩�ł��ڍׂȃ~�b�v�}�b�v���x���ł���A�����荂�����x���͏ڍדx���Ⴍ�Ȃ�܂�
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;	//�A�N�Z�X���N�����v����~�b�v�}�b�v�͈͂̏���B0�͍ő傩�ł��ڍׂȃ~�b�v�}�b�v���x���ł���A�����荂�����x���͏ڍדx���Ⴍ�Ȃ�܂�
	hr = device->CreateSamplerState(&sampler_desc, samplerStates[static_cast<size_t>(SAMPLER_STATE::POINT_SAMPLE)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	hr = device->CreateSamplerState(&sampler_desc, samplerStates[static_cast<size_t>(SAMPLER_STATE::LINEAR)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	hr = device->CreateSamplerState(&sampler_desc, samplerStates[static_cast<size_t>(SAMPLER_STATE::ANISOTROPIC)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.BorderColor[0] = 0.0f;
	sampler_desc.BorderColor[1] = 0.0f;
	sampler_desc.BorderColor[2] = 0.0f;
	sampler_desc.BorderColor[3] = 0.0f;
	hr = device->CreateSamplerState(&sampler_desc, samplerStates[static_cast<size_t>(SAMPLER_STATE::LINEAR_BORDER_BLACK)].GetAddressOf());

	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.BorderColor[0] = 1;
	sampler_desc.BorderColor[1] = 1;
	sampler_desc.BorderColor[2] = 1;
	sampler_desc.BorderColor[3] = 1;
	hr = device->CreateSamplerState(&sampler_desc, samplerStates[static_cast<size_t>(SAMPLER_STATE::LINEAR_BORDER_WHITE)].GetAddressOf());

	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.BorderColor[0] = 0;
	sampler_desc.BorderColor[1] = 0;
	sampler_desc.BorderColor[2] = 0;
	sampler_desc.BorderColor[3] = 0;
	hr = device->CreateSamplerState(&sampler_desc, samplerStates[static_cast<size_t>(SAMPLER_STATE::CLAMP)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	//	�V���h�E�}�b�v�p
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.BorderColor[0] = 1;
	sampler_desc.BorderColor[1] = 1;
	sampler_desc.BorderColor[2] = 1;
	sampler_desc.BorderColor[3] = 1;
	hr = device->CreateSamplerState(&sampler_desc, samplerStates[static_cast<size_t>(SAMPLER_STATE::SHADOW_MAP)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));


	//-----------------------------------------------//
	//					�[�x�e�X�g					//
	//-----------------------------------------------//
	D3D11_DEPTH_STENCIL_DESC depth_stencil_desc{};

	//�[�x�e�X�g��ON�A�[�x�ւ̏�������ON
	depth_stencil_desc.DepthEnable = TRUE;	//�[�x�e�X�g��L��dd
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;//�[�x�f�[�^�ɂ���ĕύX�ł���[�x�X�e���V�� �o�b�t�@�[�̕��������
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;//�[�x�f�[�^�ɂ���ĕύX�ł���[�x�X�e���V�� �o�b�t�@�[�̕��������
	hr = device->CreateDepthStencilState(&depth_stencil_desc, depthStencilStates[static_cast<size_t>(DEPTH_STATE::DepthON_WriteON)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	//�[�x�e�X�g��ON�A�[�x�ւ̏�������OFF
	depth_stencil_desc.DepthEnable = TRUE;
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = device->CreateDepthStencilState(&depth_stencil_desc, depthStencilStates[static_cast<size_t>(DEPTH_STATE::DepthON_WriteOFF)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// �[�x�e�X�g�� OFF�A�[�x�ւ̏������� ON
	depth_stencil_desc.DepthEnable = FALSE;
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = device->CreateDepthStencilState(&depth_stencil_desc, depthStencilStates[static_cast<size_t>(DEPTH_STATE::DepthOFF_WriteON)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// �[�x�e�X�g�� OFF�A�[�x�ւ̏������� OFF
	depth_stencil_desc.DepthEnable = FALSE;
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = device->CreateDepthStencilState(&depth_stencil_desc, depthStencilStates[static_cast<size_t>(DEPTH_STATE::DepthOFF_WriteOFF)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	// �u�����f�B���O�X�e�[�g�I�u�W�F�N�g
	{
		auto r_set_blend_mode = [&](int blend_enable, const D3D11_BLEND& src_blend, const D3D11_BLEND& dest_blend,
			const D3D11_BLEND_OP& blend_op, const D3D11_BLEND& src_blend_alpha, const D3D11_BLEND& dest_blend_alpha, const BLEND_STATE& index)
		{
			D3D11_BLEND_DESC blend_desc{};
			blend_desc.AlphaToCoverageEnable = FALSE;
			blend_desc.IndependentBlendEnable = FALSE;
			blend_desc.RenderTarget[0].BlendEnable = blend_enable;
			blend_desc.RenderTarget[0].SrcBlend = src_blend;
			blend_desc.RenderTarget[0].DestBlend = dest_blend;
			blend_desc.RenderTarget[0].BlendOp = blend_op;
			blend_desc.RenderTarget[0].SrcBlendAlpha = src_blend_alpha;
			blend_desc.RenderTarget[0].DestBlendAlpha = dest_blend_alpha;
			blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			hr = device->CreateBlendState(&blend_desc, blendStates[static_cast<int>(index)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		};
		// �Ȃ�
		r_set_blend_mode(FALSE, D3D11_BLEND_ONE, D3D11_BLEND_ZERO,
			D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, BLEND_STATE::NORMAL);
		// �ʏ�i�A���t�@�u�����h)
		r_set_blend_mode(TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA,
			D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA, BLEND_STATE::ALPHA);
		// ���Z(���߂���)
		r_set_blend_mode(TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE,
			D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ONE, BLEND_STATE::ADD);
		// ���Z
		r_set_blend_mode(TRUE, D3D11_BLEND_ZERO, D3D11_BLEND_INV_SRC_COLOR,
			D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, BLEND_STATE::SUBTRACTIVE);
		// ��Z
		r_set_blend_mode(TRUE, D3D11_BLEND_ZERO, D3D11_BLEND_SRC_COLOR,
			D3D11_BLEND_OP_ADD, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_ZERO, BLEND_STATE::MULTIPLY);
	}

	// ���X�^���C�U�X�e�[�g�I�u�W�F�N�g
	{
		D3D11_RASTERIZER_DESC rasterizer_desc{};
		rasterizer_desc.DepthBias = 0;
		rasterizer_desc.DepthBiasClamp = 0;
		rasterizer_desc.SlopeScaledDepthBias = 0;
		rasterizer_desc.DepthClipEnable = TRUE;
		rasterizer_desc.ScissorEnable = FALSE;
		rasterizer_desc.MultisampleEnable = FALSE;

		auto r_set_RasterizerState = [&](const D3D11_FILL_MODE& fill_mode, const D3D11_CULL_MODE& cull_mode,
			int antialiased_line_enable, int counterclockwise, const RASTERIZER_STATE& index)
		{
			rasterizer_desc.FillMode = fill_mode;
			rasterizer_desc.CullMode = cull_mode;
			rasterizer_desc.AntialiasedLineEnable = antialiased_line_enable;
			rasterizer_desc.FrontCounterClockwise = counterclockwise;
			hr = device->CreateRasterizerState(&rasterizer_desc, rasterizerStates[static_cast<int>(index)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		};
		// SOLID
		r_set_RasterizerState(D3D11_FILL_SOLID, D3D11_CULL_BACK, FALSE, FALSE, RASTERIZER_STATE::SOLID_ONESIDE);
		// CULL_NONE
		r_set_RasterizerState(D3D11_FILL_SOLID, D3D11_CULL_NONE, FALSE, FALSE, RASTERIZER_STATE::CULL_NONE);
		// SOLID(�����v���)
		r_set_RasterizerState(D3D11_FILL_SOLID, D3D11_CULL_BACK, FALSE, TRUE, RASTERIZER_STATE::SOLID_COUNTERCLOCKWISE);
		// WIREFRAME_CULL_BACK
		r_set_RasterizerState(D3D11_FILL_WIREFRAME, D3D11_CULL_BACK, TRUE, FALSE, RASTERIZER_STATE::WIREFRAME_CULL_BACK);
		// WIREFRAME_CULL_NONE
		r_set_RasterizerState(D3D11_FILL_WIREFRAME, D3D11_CULL_NONE, TRUE, FALSE, RASTERIZER_STATE::WIREFRAME_CULL_NONE);
	}

	//�����_���[
	{
		debugRenderer = std::make_unique<DebugRenderer>(device.Get());

	}
}

void Graphics::DebugGui()
{
}

void Graphics::SetDepthStencilState(DEPTH_STATE z_stencil)
{
	// �[�x�X�e���V���X�e�[�g�I�u�W�F�N�g
	immediateContext->OMSetDepthStencilState(depthStencilStates[static_cast<int>(z_stencil)].Get(), 1);
}

void Graphics::SetBlendState(BLEND_STATE blend)
{
	// �u�����f�B���O�X�e�[�g�I�u�W�F�N�g
	immediateContext->OMSetBlendState(blendStates[static_cast<int>(blend)].Get(), nullptr, 0xFFFFFFFF);
}

void Graphics::SetRasterizerState(RASTERIZER_STATE rasterizer)
{
	// ���X�^���C�U�X�e�[�g
	immediateContext->RSSetState(rasterizerStates[static_cast<int>(rasterizer)].Get());

}

void Graphics::SetGraphicStatePriset(DEPTH_STATE z_stencil, BLEND_STATE blend, RASTERIZER_STATE rasterizer)
{
	// �[�x�X�e���V���X�e�[�g�I�u�W�F�N�g
	immediateContext->OMSetDepthStencilState(depthStencilStates[static_cast<int>(z_stencil)].Get(), 1);
	// �u�����f�B���O�X�e�[�g�I�u�W�F�N�g
	immediateContext->OMSetBlendState(blendStates[static_cast<int>(blend)].Get(), nullptr, 0xFFFFFFFF);
	// ���X�^���C�U�X�e�[�g
	immediateContext->RSSetState(rasterizerStates[static_cast<int>(rasterizer)].Get());

}

void Graphics::ShaderActivate(SHADER_TYPES sh, RENDER_TYPE rt)
{
	////�w�肵���V�F�[�_�[�ɐ؂�ւ���
	//shader = shaders.at(sh);
	////�V�F�[�_�[���A�N�e�B�u��Ԃ�
	////shader->active(immediate_context.Get());
	//if (shaders.at(sh))
	//	shaders.at(sh)->active(immediate_context.Get(), rt);

}

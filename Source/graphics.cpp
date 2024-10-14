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
	//デバイス、デバイスコンテキスト、スワップチェーンを同時に生成
	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, create_device_flags,
		&featureLevels, 1, D3D11_SDK_VERSION, &swapChainDesc,
		swapChain.GetAddressOf(), device.GetAddressOf(), NULL, immediateContext.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	//②レンダーターゲットビューの作成
	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer{};		//2D テクスチャー インターフェイス:、構造化されたメモリーであるテクセル データを管理 https://docs.microsoft.com/ja-jp/previous-versions/direct-x/ee420038(v=vs.85)
	hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(backBuffer.GetAddressOf()));
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	hr = device->CreateRenderTargetView(backBuffer.Get(), NULL, renderTargetView.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));



	//②深層ステンシルビューの作成

	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer{};	//
	D3D11_TEXTURE2D_DESC texture2dDesc{};	//
	texture2dDesc.Width = SCREEN_WIDTH;		//テクスチャ幅
	texture2dDesc.Height = SCREEN_HEIGHT;	//テクスチャの高さ
	texture2dDesc.MipLevels = 1;			//テクスチャ内のミップマップレベルの最大数
	texture2dDesc.ArraySize = 1;			//テクスチャ配列内のテクスチャの数
	texture2dDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;		/*テクスチャフォーマット
																深度用に24ビット、ステンシル用に8ビットをサポートする32ビットのzバッファ形式。*/
	texture2dDesc.SampleDesc.Count = 1;		//テクスチャのマルチサンプリングパラメータを指定する構造。
	texture2dDesc.SampleDesc.Quality = 0;	//
	texture2dDesc.Usage = D3D11_USAGE_DEFAULT;		//テクスチャの読み取りおよび書き込み方法を識別する値
	texture2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;		//パイプラインステージにバインドするためのフラグ
	texture2dDesc.CPUAccessFlags = 0;		//許可されるCPUアクセスのタイプを指定するフラグ
	texture2dDesc.MiscFlags = 0;		//他のあまり一般的ではないリソースオプションを識別するフラグ
	hr = device->CreateTexture2D(&texture2dDesc, NULL, depthStencilBuffer.GetAddressOf());	//
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));		//

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilVewDesc{};
	depthStencilVewDesc.Format = texture2dDesc.Format;						/*リソースデータ形式（DXGI_FORMATを参照）
																				https://docs.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format */
	depthStencilVewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;		/*リソースのタイプ	https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_dsv_dimension)
																				デプスステンシルリソースへのアクセス方法を指定します。値は、この構造体の共用体に格納されます。*/
	depthStencilVewDesc.Texture2D.MipSlice = 0;								//1Dテクスチャサブリソースを指定します（D3D11_TEX1D_DSVを参照）。
	hr = device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilVewDesc, depthStencilView.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	//depthStencilBuffer->Release();	


	//ビューポートの作成
	D3D11_VIEWPORT viewport{};		//https://docs.microsoft.com/ja-jp/windows/win32/api/d3d11/ns-d3d11-d3d11_viewport)
	viewport.TopLeftX = 0;		//ビューポートの左側のX位置
	viewport.TopLeftY = 0;		//ビューポートの上部のY位置
	viewport.Width = static_cast<float>(SCREEN_WIDTH);		//ビューポートの幅
	viewport.Height = static_cast<float>(SCREEN_HEIGHT);		//ビューポートの高さ
	viewport.MinDepth = 0.0f;		//ビューポートの最小深度
	viewport.MaxDepth = 1.0f;		//ビューポートの最大深度
	immediateContext->RSSetViewports(1, &viewport);		//NumViewports:バインドするビューポートの数 pViewports:デバイスにバインドするD3D11_VIEWPORT構造体の配列


	D3D11_SAMPLER_DESC sampler_desc;	//https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_sampler_desc)
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;	//テクスチャをサンプリングするときに使用するフィルタリング方法
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;		//0から1の範囲外のauテクスチャ座標を解決するために使用するメソッド
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;		//0から1の範囲外のavテクスチャ座標を解決するために使用する方法。
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;		//0から1の範囲外のawテクスチャ座標を解決するために使用する方法
	sampler_desc.MipLODBias = 0;			//計算されたミップマップレベルからのオフセット。
	sampler_desc.MaxAnisotropy = 16;		//フィルタでD3D11_FILTER_ANISOTROPICまたは
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;	//サンプリングされたデータを既存のサンプリングされたデータと比較する関数
	sampler_desc.BorderColor[0] = 0.0f;		//AddressU、AddressV、またはAddressWにD3D11_TEXTURE_ADDRESS_BORDERが指定されている場合に使用する境界線の色
	sampler_desc.BorderColor[1] = 0.0f;
	sampler_desc.BorderColor[2] = 0.0f;
	sampler_desc.BorderColor[3] = 0.0f;
	sampler_desc.MinLOD = 0;	//アクセスをクランプするミップマップ範囲の下限。0は最大かつ最も詳細なミップマップレベルであり、それより高いレベルは詳細度が低くなります
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;	//アクセスをクランプするミップマップ範囲の上限。0は最大かつ最も詳細なミップマップレベルであり、それより高いレベルは詳細度が低くなります
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
	//	シャドウマップ用
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
	//					深度テスト					//
	//-----------------------------------------------//
	D3D11_DEPTH_STENCIL_DESC depth_stencil_desc{};

	//深度テストをON、深度への書き込みON
	depth_stencil_desc.DepthEnable = TRUE;	//深度テストを有効dd
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;//深度データによって変更できる深度ステンシル バッファーの部分を特定
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;//深度データによって変更できる深度ステンシル バッファーの部分を特定
	hr = device->CreateDepthStencilState(&depth_stencil_desc, depthStencilStates[static_cast<size_t>(DEPTH_STATE::DepthON_WriteON)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	//深度テストをON、深度への書き込みOFF
	depth_stencil_desc.DepthEnable = TRUE;
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = device->CreateDepthStencilState(&depth_stencil_desc, depthStencilStates[static_cast<size_t>(DEPTH_STATE::DepthON_WriteOFF)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// 深度テストを OFF、深度への書き込み ON
	depth_stencil_desc.DepthEnable = FALSE;
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = device->CreateDepthStencilState(&depth_stencil_desc, depthStencilStates[static_cast<size_t>(DEPTH_STATE::DepthOFF_WriteON)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// 深度テストを OFF、深度への書き込み OFF
	depth_stencil_desc.DepthEnable = FALSE;
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = device->CreateDepthStencilState(&depth_stencil_desc, depthStencilStates[static_cast<size_t>(DEPTH_STATE::DepthOFF_WriteOFF)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	// ブレンディングステートオブジェクト
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
		// なし
		r_set_blend_mode(FALSE, D3D11_BLEND_ONE, D3D11_BLEND_ZERO,
			D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, BLEND_STATE::NORMAL);
		// 通常（アルファブレンド)
		r_set_blend_mode(TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA,
			D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA, BLEND_STATE::ALPHA);
		// 加算(透過あり)
		r_set_blend_mode(TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE,
			D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ONE, BLEND_STATE::ADD);
		// 減算
		r_set_blend_mode(TRUE, D3D11_BLEND_ZERO, D3D11_BLEND_INV_SRC_COLOR,
			D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, BLEND_STATE::SUBTRACTIVE);
		// 乗算
		r_set_blend_mode(TRUE, D3D11_BLEND_ZERO, D3D11_BLEND_SRC_COLOR,
			D3D11_BLEND_OP_ADD, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_ZERO, BLEND_STATE::MULTIPLY);
	}

	// ラスタライザステートオブジェクト
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
		// SOLID(反時計回り)
		r_set_RasterizerState(D3D11_FILL_SOLID, D3D11_CULL_BACK, FALSE, TRUE, RASTERIZER_STATE::SOLID_COUNTERCLOCKWISE);
		// WIREFRAME_CULL_BACK
		r_set_RasterizerState(D3D11_FILL_WIREFRAME, D3D11_CULL_BACK, TRUE, FALSE, RASTERIZER_STATE::WIREFRAME_CULL_BACK);
		// WIREFRAME_CULL_NONE
		r_set_RasterizerState(D3D11_FILL_WIREFRAME, D3D11_CULL_NONE, TRUE, FALSE, RASTERIZER_STATE::WIREFRAME_CULL_NONE);
	}

	//レンダラー
	{
		debugRenderer = std::make_unique<DebugRenderer>(device.Get());

	}
}

void Graphics::DebugGui()
{
}

void Graphics::SetDepthStencilState(DEPTH_STATE z_stencil)
{
	// 深度ステンシルステートオブジェクト
	immediateContext->OMSetDepthStencilState(depthStencilStates[static_cast<int>(z_stencil)].Get(), 1);
}

void Graphics::SetBlendState(BLEND_STATE blend)
{
	// ブレンディングステートオブジェクト
	immediateContext->OMSetBlendState(blendStates[static_cast<int>(blend)].Get(), nullptr, 0xFFFFFFFF);
}

void Graphics::SetRasterizerState(RASTERIZER_STATE rasterizer)
{
	// ラスタライザステート
	immediateContext->RSSetState(rasterizerStates[static_cast<int>(rasterizer)].Get());

}

void Graphics::SetGraphicStatePriset(DEPTH_STATE z_stencil, BLEND_STATE blend, RASTERIZER_STATE rasterizer)
{
	// 深度ステンシルステートオブジェクト
	immediateContext->OMSetDepthStencilState(depthStencilStates[static_cast<int>(z_stencil)].Get(), 1);
	// ブレンディングステートオブジェクト
	immediateContext->OMSetBlendState(blendStates[static_cast<int>(blend)].Get(), nullptr, 0xFFFFFFFF);
	// ラスタライザステート
	immediateContext->RSSetState(rasterizerStates[static_cast<int>(rasterizer)].Get());

}

void Graphics::ShaderActivate(SHADER_TYPES sh, RENDER_TYPE rt)
{
	////指定したシェーダーに切り替える
	//shader = shaders.at(sh);
	////シェーダーをアクティブ状態に
	////shader->active(immediate_context.Get());
	//if (shaders.at(sh))
	//	shaders.at(sh)->active(immediate_context.Get(), rt);

}

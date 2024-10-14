#include "deferred_renderer.h"

#include "shader.h"
#include "texture.h"
#include "user.h"

DeferredRenderer::DeferredRenderer()
{
	Graphics& graphics = Graphics::Instance();
	//G-Buffer
	g_color = std::make_unique<GBuffer>();
	g_depth = std::make_unique<GBuffer>();
	g_normal = std::make_unique<GBuffer>();
	g_position = std::make_unique<GBuffer>();
	g_metal_smooth = std::make_unique<GBuffer>();
	g_emissive = std::make_unique<GBuffer>();
	l_light = std::make_unique<GBuffer>();
	l_composite = std::make_unique<GBuffer>();


	g_color->Create(graphics.GetDevice().Get(), DXGI_FORMAT_R16G16B16A16_FLOAT);
	g_depth->Create(graphics.GetDevice().Get(), DXGI_FORMAT_R32_FLOAT);
	g_normal->Create(graphics.GetDevice().Get(), DXGI_FORMAT_R8G8B8A8_UNORM);
	g_position->Create(graphics.GetDevice().Get(), DXGI_FORMAT_R32G32B32A32_FLOAT);
	g_metal_smooth->Create(graphics.GetDevice().Get(), DXGI_FORMAT_R16G16B16A16_FLOAT);
	g_emissive->Create(graphics.GetDevice().Get(), DXGI_FORMAT_R16G16B16A16_FLOAT);
	l_light->Create(graphics.GetDevice().Get(), DXGI_FORMAT_R16G16B16A16_FLOAT);
	l_composite->Create(graphics.GetDevice().Get(), DXGI_FORMAT_R16G16B16A16_FLOAT);
#if CAST_SHADOW
	//シャドウマップ初期化
	int ShadowMapSize = 1024;
	shadow_frame_buffer = std::make_unique<framebuffer>(graphics.GetDevice().Get(), ShadowMapSize, ShadowMapSize);
	shadow_constants = std::make_unique<Constants<SHADOW_CONSTANTS>>(graphics.GetDevice().Get());
#endif
	//深度ステンシル
	DepthStencilCreate(graphics.GetDevice().Get(), DXGI_FORMAT_D24_UNORM_S8_UINT);
	HRESULT hr{};

	deferred_screen = std::make_unique<fullscreen_quad>(graphics.GetDevice().Get());
	D3D11_TEXTURE2D_DESC texture2d_desc{};
	//hr = load_texture_from_file(graphics.GetDevice().Get(), L"./resources/Sprite/emv_map.DDS", env_texture.ReleaseAndGetAddressOf(), &texture2d_desc);
	//
	//hr = create_ps_from_cso(graphics.GetDevice().Get(), "shaders/deferred_env_light.cso", deferred_env_light.GetAddressOf());
	//hr = create_ps_from_cso(graphics.GetDevice().Get(), "shaders/deferred_composite_light.cso", deferred_composite_light.GetAddressOf());
	//hr = create_ps_from_cso(graphics.GetDevice().Get(), "shaders/final_sprite_ps.cso", final_sprite_ps.GetAddressOf());
	if (FAILED(hr)) return;

}

//書き込み開始
void DeferredRenderer::Active()
{
	Graphics& graphics = Graphics::Instance();
	//RTVを変更する前に使用中のRTVを保存
	graphics.Get_DC()->OMGetRenderTargets(1, cached_render_target_view.ReleaseAndGetAddressOf(), cached_depth_stencil_view.ReleaseAndGetAddressOf());

	ID3D11RenderTargetView* targets[] = {
		g_color->Get_rtv(),   //Target0
		g_depth->Get_rtv(),   //Target1
		g_normal->Get_rtv(),   //Target2
		g_position->Get_rtv(),   //Target3
		g_metal_smooth->Get_rtv(),  //Target4
		g_emissive->Get_rtv(),  //Target5

	};
	// レンダーターゲットビュー設定
	graphics.Get_DC()->OMSetRenderTargets(
		6, targets, depth_stencil_view.Get());
	//レンダーターゲットビューのクリア
	float clearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	graphics.Get_DC()->ClearRenderTargetView(g_color->Get_rtv(), clearColor);

	float clear_metallic_smooth[4] = { 0.8f, 0.1f, 0, 0 };
	graphics.Get_DC()->ClearRenderTargetView(g_metal_smooth->Get_rtv(), clear_metallic_smooth);

	float clear_pos_normal_light[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	graphics.Get_DC()->ClearRenderTargetView(g_normal->Get_rtv(), clear_pos_normal_light);
	graphics.Get_DC()->ClearRenderTargetView(g_position->Get_rtv(), clear_pos_normal_light);
	graphics.Get_DC()->ClearRenderTargetView(g_emissive->Get_rtv(), clear_pos_normal_light);

	float cleardepth[4] = { 5000, 1.0f, 1.0f, 1.0f };
	graphics.Get_DC()->ClearRenderTargetView(g_depth->Get_rtv(), cleardepth);

	//深度ステンシルビューのクリア
	graphics.Get_DC()->ClearDepthStencilView(depth_stencil_view.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// ビューポート設定
	D3D11_VIEWPORT vp = {
		0,0,
		static_cast<float>(SCREEN_WIDTH),
		static_cast<float>(SCREEN_HEIGHT),
		0,1 };
	graphics.Get_DC()->RSSetViewports(1, &vp);

}

//書き込み終了
void DeferredRenderer::Deactive()
{
	Graphics& graphics = Graphics::Instance();
	//ライティング実行
	Lighting();
	//描画先を戻す
	graphics.Get_DC()->OMSetRenderTargets(1, cached_render_target_view.GetAddressOf(),
		cached_depth_stencil_view.Get());

}

void DeferredRenderer::Lighting() const
{
	Graphics& graphics = Graphics::Instance();
	ID3D11RenderTargetView* rtv = l_light->Get_rtv();
	// レンダーターゲットビュー設定
	graphics.Get_DC()->OMSetRenderTargets(
		1, &rtv, depth_stencil_view.Get());
	//レンダーターゲットビューのクリア
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	graphics.Get_DC()->ClearRenderTargetView(l_light->Get_rtv(), clearColor);

	//G-Buffer：カラー、ノーマル、ポジション、メタリック・スムース、ライト
	ID3D11ShaderResourceView* g_buffers[]
	{
		g_color->Get_srv(),
		g_normal->Get_srv(),
		g_position->Get_srv(),
		g_metal_smooth->Get_srv(),
		g_emissive->Get_srv(),
		l_light->Get_srv(),
	};
	//ブレンドステートを加算に
	graphics.SetGraphicStatePriset(ST_DEPTH::DepthON_WriteON, ST_BLEND::ADD, ST_RASTERIZER::CULL_NONE);

	UINT G_BUFFERS_NUM = ARRAYSIZE(g_buffers);
	//環境ライト
	graphics.Get_DC().Get()->PSSetShaderResources(15, 1, env_texture.GetAddressOf());
	deferred_screen->blit(graphics.Get_DC().Get(), g_buffers, 0, G_BUFFERS_NUM, deferred_env_light.Get());

	//平行光、点光源ライトを描き込む
#if CAST_SHADOW
	shadow_constants->Bind(graphics.Get_DC().Get(), 10, CB_FLAG::PS_VS);
	graphics.Get_DC().Get()->PSSetShaderResources(16, 1, shadow_frame_buffer->get_color_map().GetAddressOf());
#endif
	LightManager::Instance().Draw(g_buffers, G_BUFFERS_NUM);

	//ライトの合成 ブレンドステートをアルファに
	graphics.SetBlendState(ST_BLEND::ALPHA);
	rtv = l_composite->Get_rtv();
	graphics.Get_DC()->OMSetRenderTargets(
		1, &rtv, depth_stencil_view.Get());
	graphics.Get_DC()->ClearRenderTargetView(l_composite->Get_rtv(), clearColor);

	deferred_screen->blit(graphics.Get_DC().Get(), g_buffers, 0, G_BUFFERS_NUM, deferred_composite_light.Get());

	LightManager::Instance().DebugGUI();

}

void DeferredRenderer::Render()
{
	Graphics& graphics = Graphics::Instance();
	ID3D11ShaderResourceView* g_buffers[]
	{
		l_composite->Get_srv(),
	};
	UINT G_BUFFERS_NUM = ARRAYSIZE(g_buffers);
	deferred_screen->blit(graphics.Get_DC().Get(), g_buffers, 0, G_BUFFERS_NUM, final_sprite_ps.Get());

#if USE_IMGUI
	imguiMenuBar("Window", "G-Buffer", displayImgui);
	if (displayImgui)
	{
		ImGui::Begin("G -Buffer");
		ImGui::Text("color");
		ImGui::Image(g_color->Get_srv(), { 1280 * (ImGui::GetWindowSize().x / 1280),  720 * (ImGui::GetWindowSize().y / 720) });
		ImGui::Text("depth");
		ImGui::Image(g_depth->Get_srv(), { 1280 * (ImGui::GetWindowSize().x / 1280),  720 * (ImGui::GetWindowSize().y / 720) });
		ImGui::Text("normal");
		ImGui::Image(g_normal->Get_srv(), { 1280 * (ImGui::GetWindowSize().x / 1280),  720 * (ImGui::GetWindowSize().y / 720) });
		ImGui::Text("position");
		ImGui::Image(g_position->Get_srv(), { 1280 * (ImGui::GetWindowSize().x / 1280),  720 * (ImGui::GetWindowSize().y / 720) });
		ImGui::Text("metal_smooth");
		ImGui::Image(g_metal_smooth->Get_srv(), { 1280 * (ImGui::GetWindowSize().x / 1280),  720 * (ImGui::GetWindowSize().y / 720) });
		ImGui::Text("light");
		ImGui::Image(l_light->Get_srv(), { 1280 * (ImGui::GetWindowSize().x / 1280),  720 * (ImGui::GetWindowSize().y / 720) });
		ImGui::DragFloat("scale", &scale, 0.1f);
		ImGui::DragFloat("vie", &vie, 0.1f);
		ImGui::DragFloat("FarZ", &FarZ, 0.1f);
#if CAST_SHADOW
		ImGui::Text("shadow");
		ImGui::Image(shadow_frame_buffer->get_color_map().Get(), { 1080 * (ImGui::GetWindowSize().x / 1080),  1080 * (ImGui::GetWindowSize().y / 1080) });
#endif

		ImGui::End();

	}
#endif // USE_IMGUI

}

//==============================================================
// 
// 影用の描画書き込み開始
// 
//==============================================================
#if CAST_SHADOW
void DeferredRenderer::ShadowActive(DirectX::XMFLOAT3 target_pos)
{
	Graphics& graphics = Graphics::Instance();
	DirectX::XMFLOAT4 clearColor = { FLT_MAX, FLT_MAX, FLT_MAX, 1.0f };
	shadow_frame_buffer->clear(graphics.Get_DC().Get(), FB_FLAG::COLOR_DEPTH_STENCIL, clearColor);
	shadow_frame_buffer->activate(graphics.Get_DC().Get());

	// ビューポートの設定
	D3D11_VIEWPORT	vp = {};
	vp.Width = static_cast<float>(shadow_frame_buffer->get_tex_width());
	vp.Height = static_cast<float>(shadow_frame_buffer->get_tex_height());
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	graphics.Get_DC().Get()->RSSetViewports(1, &vp);


	// 平行光源からカメラ位置を作成し、そこから原点の位置を見るように視線行列を生成
	DirectX::XMVECTOR TargetPosition = DirectX::XMVectorSet(target_pos.x, target_pos.y, target_pos.z, 0.0f);
	DirectX::XMFLOAT3 shadow_dir_light_dir = LightManager::Instance().GetShadowDirLightDirection();
	shadow_dir_light_dir = Math::Normalize(shadow_dir_light_dir);
	DirectX::XMVECTOR LightPosition = DirectX::XMLoadFloat3(&shadow_dir_light_dir);
	LightPosition = DirectX::XMVectorScale(LightPosition, scale);
	LightPosition = DirectX::XMVectorAdd(LightPosition, TargetPosition);
	DirectX::XMMATRIX V = DirectX::XMMatrixLookAtLH(LightPosition,
		TargetPosition,
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	DirectX::XMMATRIX P = DirectX::XMMatrixOrthographicLH(vie, vie, 10, FarZ);
	DirectX::XMMATRIX VP = V * P;
	DirectX::XMFLOAT4X4	shadowVP;
	DirectX::XMStoreFloat4x4(&shadow_constants->data.shadowVP, VP);
	shadow_constants->Bind(graphics.Get_DC().Get(), 10, CB_FLAG::PS_VS);

}
//==============================================================
// 
// 影用の描画書き込み終了
// 
//==============================================================
void DeferredRenderer::ShadowDeactive()
{
	Graphics& graphics = Graphics::Instance();
	shadow_frame_buffer->deactivate(graphics.Get_DC().Get());

}
#endif

//==============================================================
// 
// 深度バッファ生成
// 
//==============================================================
void DeferredRenderer::DepthStencilCreate(ID3D11Device* device, DXGI_FORMAT format)
{
	HRESULT hr;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depth_stencil_buffer;
	// 深度ステンシル設定
	D3D11_TEXTURE2D_DESC td;
	ZeroMemory(&td, sizeof(D3D11_TEXTURE2D_DESC));
	td.Width = SCREEN_WIDTH;
	td.Height = SCREEN_HEIGHT;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = format;
	td.SampleDesc.Count = 1;
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	td.CPUAccessFlags = 0;
	td.MiscFlags = 0;

	// 深度ステンシルテクスチャ生成
	hr = device->CreateTexture2D(&td, NULL, depth_stencil_buffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// 深度ステンシルビュー設定
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
	ZeroMemory(&dsvd, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	dsvd.Format = td.Format;
	dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvd.Texture2D.MipSlice = 0;

	// 深度ステンシルビュー生成
	hr = device->CreateDepthStencilView(depth_stencil_buffer.Get(), &dsvd, depth_stencil_view.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

}

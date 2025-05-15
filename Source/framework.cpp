#include "framework.h"
#include "interval.h"
#include "shader.h"
#include "texture.h"
#include "user.h"
#include "Graphics.h"
#include "scene_manager.h"
#include "scene_title.h"
#include "device.h"
#include "effect_manager.h"

framework::framework(HWND hwnd) : hwnd(hwnd)
{
}

bool framework::Initialize()
{
	Graphics::Instance().Initialize(hwnd);
	Device::Instance().GetMouse().Set_do_show(false);
	SceneManager::Instance().ChangeScene(new SceneTitle());

	// エフェクトマネージャー初期化
	EffectManager::Instance().Initialize();

	//IBLテクスチャをロード
	D3D11_TEXTURE2D_DESC texture2d_desc;
	load_texture_from_file(Graphics::Instance().GetDevice().Get(), L"Resources/SkyMap/captured at (0, 0, 0)/lut_ggx.dds",
		shader_resource_views[0].GetAddressOf(), &texture2d_desc);

	return true;
}

void framework::Update(float elapsedTime/*Elapsed seconds from last frame*/)
{
#ifdef USE_IMGUI
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif
	//デバイス
	Device& device = Device::Instance();
	device.Update(hwnd, elapsedTime);
	device.GetMouse().OperationActivation();
	device.GetGamePad().OperationActivation();

	Graphics::Instance().SetHwnd(hwnd);

	//シーン更新
	SceneManager::Instance().Update(elapsedTime);
	Graphics::Instance().DebugGui();

	//デバッグモード切り替え
	if (device.GetMouse().GetButton() & device.GetMouse().BTN_F2)
	{
		isDebug = !isDebug;
		device.GetMouse().Set_do_show(isDebug);
		//Graphics::Instance().SetisDisplayDebug(isDebug);
	}

#ifdef USE_IMGUI
#endif
}

void framework::Render(float elapsedTime/*Elapsed seconds from last frame*/)
{
	//別スレッド中にデバイスコンテキストが使われていた場合に
	//同時アクセスしないように排他制御する
	Graphics& graphics = Graphics::Instance();
	std::lock_guard<std::mutex> lock(graphics.GetMutex());

	//HRESULT hr{ S_OK };

	ID3D11RenderTargetView* null_render_target_views[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT]{};
	graphics.Get_DC()->OMSetRenderTargets(_countof(null_render_target_views), null_render_target_views, 0);
	ID3D11ShaderResourceView* null_shader_resource_views[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
	graphics.Get_DC()->VSSetShaderResources(0, _countof(null_shader_resource_views), null_shader_resource_views);
	graphics.Get_DC()->PSSetShaderResources(0, _countof(null_shader_resource_views), null_shader_resource_views);

	// IBLテクスチャをバインド
	graphics.Get_DC()->PSSetShaderResources(35, 1, shader_resource_views[0].GetAddressOf());

	FLOAT color[]{ 0.2f, 0.2f, 0.2f, 1.0f };
	graphics.Get_DC()->ClearRenderTargetView(graphics.GetRenderTargetView().Get(), color);
#if 1
	graphics.Get_DC()->ClearDepthStencilView(graphics.GetDepthStencilView().Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
#endif
	graphics.Get_DC()->OMSetRenderTargets(1, graphics.GetRenderTargetView().GetAddressOf(), graphics.GetDepthStencilView().Get());

	//サンプラーステートオブジェクトをすべてバインド
	graphics.Get_DC()->PSSetSamplers(0, 1, graphics.GetSamplerState(ST_SAMPLER::POINT_SAMPLE).GetAddressOf());
	graphics.Get_DC()->PSSetSamplers(1, 1, graphics.GetSamplerState(ST_SAMPLER::LINEAR).GetAddressOf());
	graphics.Get_DC()->PSSetSamplers(2, 1, graphics.GetSamplerState(ST_SAMPLER::ANISOTROPIC).GetAddressOf());
	graphics.Get_DC()->PSSetSamplers(3, 1, graphics.GetSamplerState(ST_SAMPLER::LINEAR_BORDER_BLACK).GetAddressOf());
	graphics.Get_DC()->PSSetSamplers(4, 1, graphics.GetSamplerState(ST_SAMPLER::LINEAR_BORDER_WHITE).GetAddressOf());
	graphics.Get_DC()->PSSetSamplers(5, 1, graphics.GetSamplerState(ST_SAMPLER::CLAMP).GetAddressOf());
	graphics.Get_DC()->PSSetSamplers(6, 1, graphics.GetSamplerState(ST_SAMPLER::SHADOW_MAP).GetAddressOf());

	graphics.SetGraphicStatePriset(ST_DEPTH::DepthON_WriteON, ST_BLEND::ALPHA, ST_RASTERIZER::SOLID_ONESIDE);
	
	SceneManager::Instance().Render(elapsedTime);

#ifdef USE_IMGUI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif

	UINT sync_interval{ 0 };
	graphics.GetSwapChain()->Present(sync_interval, 0);

}

bool framework::Uninitialize()
{
	//Sprite オブジェクトを解放する
	SceneManager::Instance().Clear();

	// エフェクトマネージャー終了化
	EffectManager::Instance().Finalize();

	return true;
}

framework::~framework()
{
#ifdef _DEBUG

#endif
}

int framework::run()
{
	MSG msg{};

	if (!Initialize())
	{
		return 0;
	}
	Graphics& graphics = Graphics::Instance();
#ifdef USE_IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 14.0f, nullptr, glyphRangesJapanese);
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(graphics.GetDevice().Get(), graphics.Get_DC().Get());
	ImGui::StyleColorsDark();
#endif

	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			tictoc.tick();
			calculate_frame_stats();
			Update(tictoc.time_interval());
			Render(tictoc.time_interval());
		}
	}

#ifdef USE_IMGUI
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif

#if 1
	BOOL fullscreen{};
	graphics.GetSwapChain()->GetFullscreenState(&fullscreen, 0);
	if (fullscreen)
	{
		graphics.GetSwapChain()->SetFullscreenState(FALSE, 0);
	}
#endif

	return Uninitialize() ? static_cast<int>(msg.wParam) : 0;
}
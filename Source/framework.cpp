#include "framework.h"
#include "interval.h"
#include "shader.h"
#include "texture.h"
#include "user.h"
#include "Graphics.h"
#include "scene_manager.h"
#include "scene_title.h"

framework::framework(HWND hwnd) : hwnd(hwnd)
{
}

bool framework::initialize()
{
	Graphics::Instance().Initialize(hwnd);

	SceneManager::Instance().ChangeScene(new SceneTitle());

	{
		//シーン定数バッファオブジェクトを生成
		{
			//D3D11_BUFFER_DESC buffer_desc{};
			//buffer_desc.ByteWidth = sizeof(scene_constants);
			//
			//buffer_desc.Usage = D3D11_USAGE_DEFAULT;
			//buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			//buffer_desc.CPUAccessFlags = 0;
			//buffer_desc.MiscFlags = 0;
			//buffer_desc.StructureByteStride = 0;
			//hr = Graphics::Instance().GetDevice().Get()->CreateBuffer(&buffer_desc, nullptr, constant_buffers[0].GetAddressOf());
			//_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		}

		//sprite オブジェクトを生成する
		//sprites[0] = new Sprite(device.Get(), L"./resources/cyberpunk.jpg");
		//sprites[1] = new Sprite(device.Get(), L"./resources/player-sprites.png");

		//sprite_batches[0] = new Sprite_Batch(device.Get(), L"./resources/player-sprites.png", 2048);

		//geometric_primitive オブジェクトを生成
		//geometric_primitives[0] = std::make_unique<geometric_primitive>(device.Get());
		//geometric_primitives[1] = std::make_unique<geometric_primitive>(device.Get());

		//static_mesh オブジェクトを生成
		//static_meshes[0] = std::make_unique<static_mesh>(device.Get(), L"./resources/F-14A_Tomcat/F-14A_Tomcat.obj", true);
		//static_meshes[1] = std::make_unique<static_mesh>(device.Get(), L"./resources/cube.obj", true);

		//create_ps_from_cso(device.Get(), "luminance_extraction_ps.cso", pixel_shaders[0].GetAddressOf());
		//create_ps_from_cso(device.Get(), "blur_ps.cso", pixel_shaders[1].GetAddressOf());

		//framebuffers[0] = std::make_unique<framebuffer>(device.Get(), 1280, 720);
		//framebuffers[1] = std::make_unique<framebuffer>(device.Get(), 1280 / 2, 720 / 2);
		//bit_block_transfer = std::make_unique<fullscreen_quad>(device.Get());

		//skinned_meshes[0] = std::make_unique<skinned_mesh>(device.Get(), "./Resources/nico.fbx");
		//sprite_batches[0] = std::make_unique<Sprite_Batch>(device.Get(), L"./Resources/screenshot.jpg", 1);

		//sprites[0] = std::make_unique<Sprite>(device.Get(), L"./Resources/クロスヘア.png");

		//gltf_models[0] = std::make_unique<gltf_model>(Graphics::Instance().GetDevice().Get(),
		//	"Resources/glTF-Sample-Models-master/2.0/DamagedHelmet/glTF/DamagedHelmet.gltf");

		//IBLテクスチャをロード
		//D3D11_TEXTURE2D_DESC texture2d_desc;
		//load_texture_from_file(device.Get(), L"reflection_capture_tool/captured at (0, 0, 0)/skybox.dds",
		//	shader_resource_views[0].GetAddressOf(), &texture2d_desc);
		//load_texture_from_file(device.Get(), L"reflection_capture_tool/captured at (0, 0, 0)/diffuse_iem.dds",
		//	shader_resource_views[1].GetAddressOf(), &texture2d_desc);
		//load_texture_from_file(device.Get(), L"reflection_capture_tool/captured at (0, 0, 0)/specular_pmrem.dds",
		//	shader_resource_views[2].GetAddressOf(), &texture2d_desc);
		//load_texture_from_file(device.Get(), L"reflection_capture_tool/captured at (0, 0, 0)/lut_ggx.dds",
		//	shader_resource_views[3].GetAddressOf(), &texture2d_desc);
	}

	return true;
}

void framework::update(float elapsed_time/*Elapsed seconds from last frame*/)
{
#ifdef USE_IMGUI
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif

	Device::Instance().Update(hwnd, elapsed_time);
	Device::Instance().GetMouse().OperationActivation();
	Device::Instance().GetGamePad().OperationActivation();

	Graphics::Instance().SetHwnd(hwnd);
	SceneManager::Instance().Update(elapsed_time);
	Graphics::Instance().DebugGui();

#ifdef USE_IMGUI
//	if (ImGui::Begin("ImGUI"))
//	{
//		ImGui::SliderFloat("camera_position.x", &camera_position.x, -100.0f, +100.0f);
//		ImGui::SliderFloat("camera_position.y", &camera_position.y, -100.0f, +100.0f);
//		ImGui::SliderFloat("camera_position.z", &camera_position.z, -100.0f, +100.0f);
//
//		ImGui::SliderFloat("light_direction.x", &light_direction.x, -1.0f, +1.0f);
//		ImGui::SliderFloat("light_direction.y", &light_direction.y, -1.0f, +1.0f);
//		ImGui::SliderFloat("light_direction.z", &light_direction.z, -1.0f, +1.0f);
//
//		ImGui::SliderFloat("translation.x", &translation.x, -10.0f, +10.0f);
//		ImGui::SliderFloat("translation.y", &translation.y, -10.0f, +10.0f);
//		ImGui::SliderFloat("translation.z", &translation.z, -10.0f, +10.0f);
//
//		ImGui::SliderFloat("scaling.x", &scaling.x, -10.0f, +10.0f);
//		ImGui::SliderFloat("scaling.y", &scaling.y, -10.0f, +10.0f);
//		ImGui::SliderFloat("scaling.z", &scaling.z, -10.0f, +10.0f);
//
//		ImGui::SliderFloat("rotation.x", &rotation.x, -10.0f, +10.0f);
//		ImGui::SliderFloat("rotation.y", &rotation.y, -10.0f, +10.0f);
//		ImGui::SliderFloat("rotation.z", &rotation.z, -10.0f, +10.0f);
//
//		ImGui::SliderFloat("factor", &factor, 0.0f, +1.0f);
//
//		ImGui::SliderInt("animation_clip", &clip_count, 0, clip_Max);
//#if 0
//		ImGui::SliderFloat("axis.x", &axis.x, -5.0f, +5.0f);
//		ImGui::SliderFloat("axis.y", &axis.y, -5.0f, +5.0f);
//		ImGui::SliderFloat("axis.z", &axis.z, -5.0f, +5.0f);
//
//		ImGui::SliderFloat("kf_Translation.x", &kf_Translation.x, -300.0f, +300.0f);
//		ImGui::SliderFloat("kf_Translation.y", &kf_Translation.y, -300.0f, +300.0f);
//		ImGui::SliderFloat("kf_Translation.z", &kf_Translation.z, -300.0f, +300.0f);
//#endif
//		ImGui::ColorEdit4("material_color", reinterpret_cast<float*>(&material_color));
//
//		//if (ImGui::TreeNode("luminance_extraction_ps"))
//		//{
//			ImGui::SliderFloat("smoothstep_minEdge", &smoothstep_minEdge, 0.0f, +1.0f);
//			ImGui::SliderFloat("smoothstep_maxEdge", &smoothstep_maxEdge, 0.0f, +1.0f);
//		//}
//			ImGui::SliderFloat("gaussian_sigma", &gaussian_sigma, 0.0f, +10.0f);
//			ImGui::SliderFloat("bloom_intensity", &bloom_intensity, 0.0f, +1.0f);
//			
//			ImGui::SliderFloat("exposure", &exposure, 0.0f, +10.0f);
//	}
//	ImGui::End();
#endif
}

void framework::render(float elapsed_time/*Elapsed seconds from last frame*/)
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
	//immediate_context->PSSetShaderResources(32, 1, shader_resource_views[0].GetAddressOf());
	//immediate_context->PSSetShaderResources(33, 1, shader_resource_views[1].GetAddressOf());
	//immediate_context->PSSetShaderResources(34, 1, shader_resource_views[2].GetAddressOf());
	//immediate_context->PSSetShaderResources(35, 1, shader_resource_views[3].GetAddressOf());


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
	
	SceneManager::Instance().Render(elapsed_time);

	////////////////////////////////////////////////////////////////////////
	// ここから移動
	////////////////////////////////////////////////////////////////////////

	using namespace DirectX;
	//ビュー・プロジェクション変換行列を計算し、
	//それを定数バッファにセット

	D3D11_VIEWPORT viewport;
	UINT num_viewports{ 1 };
	graphics.Get_DC().Get()->RSGetViewports(&num_viewports, &viewport);
	float aspect_ratio{ viewport.Width / viewport.Height };
	XMMATRIX P{ XMMatrixPerspectiveFovLH(XMConvertToRadians(30), aspect_ratio, 0.1f, 1000.0f) };
	XMVECTOR eye{ XMLoadFloat4(&camera_position) };
	XMVECTOR focus{ XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f) };

	XMVECTOR up{ XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) };
	XMMATRIX V{ XMMatrixLookAtLH(eye, focus, up) };

	//// 定数バッファのデータを更新
	//scene_constants data{};
	//DirectX::XMStoreFloat4x4(&data.view_projection, V * P);
	//data.light_direction = light_direction;
	//data.camera_position = camera_position;
	//
	//data.smoothstep_minEdge = smoothstep_minEdge;
	//data.smoothstep_maxEdge = smoothstep_maxEdge;
	//
	//data.gaussian_sigma = gaussian_sigma;
	//data.bloom_intensity = bloom_intensity;
	//
	//data.exposure = exposure;
	//
	//// バッファのデータをGPUに送信
	//graphics.Get_DC()->UpdateSubresource(constant_buffers[0].Get(), 0, 0, &data, 0, 0);

	//// シェーダーに定数バッファを設定
	//graphics.Get_DC().Get()->VSSetConstantBuffers(1, 1, constant_buffers[0].GetAddressOf());
	//graphics.Get_DC().Get()->PSSetConstantBuffers(1, 1, constant_buffers[0].GetAddressOf());

	//userに置く
	

	{
		//アニメーション
		// クリップの最大インデックスを設定（アニメーションクリップが存在する場合）
		//clip_Max = skinned_meshes[0]->animation_clips.size() - 1;
		// アニメーションクリップのインデックス、フレームインデックス、アニメーションの進行度を管理
		//int clip_index{ clip_count };
		//int frame_index{ 0 };
		//static float animation_tick{ 0 };

		// 最初のスキンメッシュのアニメーションを取得
		//skinned_mesh::animation& animation{
		//	skinned_meshes[0]->animation_clips.at(clip_index) };

		// アニメーションの進行度から現在のフレームインデックスを計算
		//frame_index = static_cast<int>(
		//	animation_tick * animation.sampling_rate);

		// フレームがシーケンスの範囲を超えた場合、最初のフレームから再生を始める
		//if (frame_index > animation.sequence.size() - 1)
		//{
		//	frame_index = 0;
		//	animation_tick = 0;
		//}
		//else
		//{
		//	// 経過時間に基づいてアニメーションの進行度を更新
		//	animation_tick += elapsed_time;
		//}

		// 現在のフレームに対応するキーフレームを取得
		//skinned_mesh::animation::keyframe& keyframe{
		//	animation.sequence.at(frame_index) };


		//framebuffers[0]->clear(immediate_context.Get(), 0.05f, 0.05f, 0.05f);
		//framebuffers[0]->activate(immediate_context.Get());

		//sprite_batchesの描画
		//immediate_context->OMSetDepthStencilState(depth_stencil_states[DEPTH_STATE::DepthDisabled_WriteNone].Get(), 1);
		//immediate_context->OMSetBlendState(blend_states[BLEND_STATE::Alpha].Get(), nullptr, 0xFFFFFFFF);
		//immediate_context->RSSetState(rasterizer_states[RASTERIZER_STATE::CullingOff].Get());
		//sprite_batches[0].get()->render(immediate_context.Get(),
		//	0.0f, 0.0f,
		//	SCREEN_WIDTH, SCREEN_HEIGHT);

		//skinned_meshの描画
		//immediate_context->OMSetDepthStencilState(depth_stencil_states[DEPTH_STATE::DepthEnabled_WriteAll].Get(), 1);
		//immediate_context->OMSetBlendState(blend_states[BLEND_STATE::Alpha].Get(), nullptr, 0xFFFFFFFF);
		//immediate_context->RSSetState(rasterizer_states[RASTERIZER_STATE::Solid].Get());
		//skinned_meshes[0]->render(immediate_context.Get(), world, material_color, &keyframe);


		//immediate_context->OMSetDepthStencilState(depth_stencil_states[DEPTH_STATE::DepthEnabled_WriteAll].Get(), 1);
		//immediate_context->OMSetBlendState(blend_states[BLEND_STATE::Alpha].Get(), nullptr, 0xFFFFFFFF);
		//immediate_context->RSSetState(rasterizer_states[RASTERIZER_STATE::Solid].Get());

		//static std::vector<gltf_model::node> animated_nodes{ gltf_models[0]->nodes };
		//static float time{ 0 };
		//gltf_models[0]->animate(0, time += elapsed_time, animated_nodes);
		//gltf_models[0]->render(graphics.Get_DC().Get(), world, animated_nodes);
		//framebuffers[0]->deactivate(immediate_context.Get());

		//framebuffers[0]から高輝度成分を抽出し
		//framebuffers[1]に転送する
		//immediate_context->OMSetDepthStencilState(depth_stencil_states[DEPTH_STATE::DepthDisabled_WriteNone].Get(), 1);
		//immediate_context->OMSetBlendState(blend_states[BLEND_STATE::Alpha].Get(), nullptr, 0xFFFFFFFF);
		//immediate_context->RSSetState(rasterizer_states[RASTERIZER_STATE::CullingOff].Get());

		//framebuffers[1]->clear(immediate_context.Get());
		//framebuffers[1]->activate(immediate_context.Get());

		//bit_block_transfer->blit(immediate_context.Get(),
		//	framebuffers[0]->shader_resource_views[0].GetAddressOf(), 0, 1, pixel_shaders[0].Get());
		//
		//framebuffers[1]->deactivate(immediate_context.Get());

		//シーン画像とブラーをかけた高輝度成分画像を合成し、
		//画面に出力する
		//ID3D11ShaderResourceView* shader_resource_views[2]
		//{ framebuffers[0]->shader_resource_views[0].Get(), framebuffers[1]->shader_resource_views[0].Get() };
		//bit_block_transfer->blit(immediate_context.Get(), shader_resource_views, 0, 2, pixel_shaders[1].Get());
	}

#ifdef USE_IMGUI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif

	UINT sync_interval{ 0 };
	graphics.GetSwapChain()->Present(sync_interval, 0);

}

bool framework::uninitialize()
{
	//Sprite オブジェクトを解放する
	SceneManager::Instance().Clear();

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

	if (!initialize())
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
			update(tictoc.time_interval());
			render(tictoc.time_interval());
		}
	}

#ifdef USE_IMGUI
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif

#if 0
	BOOL fullscreen = 0;
	swap_chain->GetFullscreenState(&fullscreen, 0);
	if (fullscreen)
	{
		swap_chain->SetFullscreenState(FALSE, 0);
	}

#endif
#if 1
	BOOL fullscreen{};
	graphics.GetSwapChain()->GetFullscreenState(&fullscreen, 0);
	if (fullscreen)
	{
		graphics.GetSwapChain()->SetFullscreenState(FALSE, 0);
	}
#endif

	return uninitialize() ? static_cast<int>(msg.wParam) : 0;
}
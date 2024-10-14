#include "scene_game.h"

#include "device.h"

#include "scene_manager.h"
#include "scene_title.h"
#include "scene_loading.h"

#include "stage_manager.h"
#include "stage_main.h"


SceneGame::SceneGame()
{
}

void SceneGame::Initialize()
{
    Graphics& graphics = Graphics::Instance();

    //scene_constants = std::make_unique<Constants<SCENE_CONSTANTS>>(graphics.GetDevice().Get());
    //scene_constantsバッファをHLSLで探したりして弄る
    
    framebuffers[0] = std::make_unique<framebuffer>(graphics.GetDevice().Get(), SCREEN_WIDTH, SCREEN_HEIGHT);
    bit_block_transfer = std::make_unique<fullscreen_quad>(graphics.GetDevice().Get());

    //camera = std::make_unique<Camera>();
    camera = &Camera::Instance();
    player = std::make_unique<Player>();

    StageManager& stageManager = StageManager::Instance();
    StageMain* stageMain = new StageMain();
    stageManager.Register(stageMain);

    deferred = std::make_unique<DeferredRenderer>();

    LightManager::Instance().Initialize();
    dirLight = std::make_shared<DirLight>(
        DirectX::XMFLOAT3(0.6f, -0.6f, 1.6f),
        DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f));
    LightManager::Instance().Register("scene_dir", dirLight);

    // SKY_MAP
    skymap = std::make_unique<SkyMap>(graphics.GetDevice().Get(), L"Resources/SkyMap/captured at (0, 0, 0)/skybox.dds");

}

void SceneGame::Finalize()
{
    StageManager::Instance().Clear();

}

void SceneGame::Update(float elapsedTime)
{
    Graphics& graphics = Graphics::Instance();
    //ゲームパッド
    GamePad& gamepad = Device::Instance().GetGamePad();

    //**********カメラの更新**********//
    camera->Update(elapsedTime);
    camera->CalcViewProjection(elapsedTime);
    camera->SetTrakkingTarget(player.get()->GetGazingPoint());
    //camera->set_lock_on_target(boss.get()->get_position());

        //カメラの経過時間
    float cameraElapsedTime = camera->HitStopUpdate(elapsedTime);

    player->Update(cameraElapsedTime);

    
    
    //**********ステージの更新**********//
    StageManager::Instance().Update(elapsedTime);

}

void SceneGame::Render(float elapsedTime)
{
    Graphics& graphics = Graphics::Instance();
    StageManager& stageManager = StageManager::Instance();

    framebuffers[0]->clear(graphics.Get_DC().Get(), 
        FB_FLAG::COLOR_DEPTH_STENCIL);
    framebuffers[0]->activate(graphics.Get_DC().Get(),
        FB_FLAG::COLOR_DEPTH_STENCIL);
    //***************************************************************//
    ///		    	            			スカイマップ		                             	  ///
    //***************************************************************//

    D3D11_VIEWPORT viewport;
    UINT num_viewports{ 1 };
    graphics.Get_DC().Get()->RSGetViewports(&num_viewports, &viewport);
    float aspect_ratio{ viewport.Width / viewport.Height };
    DirectX::XMMATRIX P{ DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(30), aspect_ratio, 0.1f, 1000.0f) };
    DirectX::XMVECTOR eye{ DirectX::XMVectorSet(0,0,0,0) };
    DirectX::XMVECTOR focus{ DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 1.0f) };
    
    DirectX::XMVECTOR up{ DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) };
    DirectX::XMMATRIX V{ DirectX::XMLoadFloat4x4(&camera->GetView()) };
    //DirectX::XMMATRIX V{ DirectX::XMMatrixLookAtLH(eye, focus, up) };

    DirectX::XMFLOAT4X4 view_pro{};
    DirectX::XMStoreFloat4x4(&view_pro, V * P);

    graphics.SetGraphicStatePriset(ST_DEPTH::DepthOFF_WriteOFF, ST_BLEND::ALPHA, ST_RASTERIZER::CULL_NONE);
    skymap->blit(graphics.Get_DC().Get(), view_pro);

    //***************************************************************//
    ///						ディファ―ドレンダリング				  ///
    //***************************************************************//

    //deferred->Active();
    graphics.SetGraphicStatePriset(ST_DEPTH::DepthON_WriteON, ST_BLEND::ALPHA, ST_RASTERIZER::SOLID_COUNTERCLOCKWISE);
    //graphics.ShaderActivate(SHADER_TYPE::PBR, RENDER_TYPE::Deferred);
    
    stageManager.Render(elapsedTime);
    //プレイヤー描画
    player->Render_d(elapsedTime);

    //ここで各種ライティング（環境光、平行光、点光源）
    //deferred->Deactive();

    //レンダーターゲットを戻す
    graphics.SetGraphicStatePriset(ST_DEPTH::DepthON_WriteON, ST_BLEND::ADD, ST_RASTERIZER::CULL_NONE);

    //deferred->Render();

    //***************************************************************//
    ///						フォワードレンダリング					///
    //***************************************************************//
    //graphics.ShaderActivate(Graphics::SHADER_TYPES::LAMBERT, RENDER_TYPE::Forward);
    graphics.SetGraphicStatePriset(
        ST_DEPTH::DepthON_WriteON,
        ST_BLEND::ALPHA,
        ST_RASTERIZER::CULL_NONE);

    player->Render_f(elapsedTime);

    //-------------------DebugPrimitive----------------------//
    graphics.SetGraphicStatePriset(ST_DEPTH::DepthON_WriteON, ST_BLEND::ALPHA, ST_RASTERIZER::WIREFRAME_CULL_BACK);
    graphics.GetDebugRenderer()->RenderAlFigures(graphics.Get_DC().Get());

    graphics.SetGraphicStatePriset(ST_DEPTH::DepthON_WriteON, ST_BLEND::ALPHA, ST_RASTERIZER::CULL_NONE);
    framebuffers[0]->deactivate(graphics.Get_DC().Get());
    ID3D11ShaderResourceView* shader_resource_views[2]
    { framebuffers[0]->get_color_map().Get(), framebuffers[0]->depth_map().Get() };
    bit_block_transfer->blit(graphics.Get_DC().Get(),framebuffers[0]->get_color_map().GetAddressOf(), 0, 1);

#if USE_IMGUI
    camera->DebugGui();
    imguiMenuBar("Game", "game_menu", displayImgui);
    if (displayImgui)
    {
        if (ImGui::Button("back_title"))
        {
            //シーンリセット
            SceneManager::Instance().ChangeScene(new SceneLoading(new SceneTitle()));
            return;
        };

    }
#endif

}

void SceneGame::DebugGui()
{
}

void SceneGame::SceneReset()
{
}

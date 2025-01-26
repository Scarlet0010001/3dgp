#include "scene_game.h"

#include "device.h"

#include "scene_manager.h"
#include "scene_title.h"
#include "scene_loading.h"

#include "bullet_manager.h"

#include "stage_manager.h"
#include "stage_main.h"


SceneGame::SceneGame()
{
}

void SceneGame::Initialize()
{
    Graphics& graphics = Graphics::Instance();
    
    framebuffers[0] = std::make_unique<framebuffer>(graphics.GetDevice().Get(), SCREEN_WIDTH, SCREEN_HEIGHT);
    framebuffers[1] = std::make_unique<framebuffer>(graphics.GetDevice().Get(), SCREEN_WIDTH, SCREEN_HEIGHT);
    bit_block_transfer = std::make_unique<fullscreen_quad>(graphics.GetDevice().Get());

    //camera = std::make_unique<Camera>();
    camera = &Camera::Instance();
    player = std::make_unique<Player>();
    boss = std::make_unique<Boss>();

    StageManager& stageManager = StageManager::Instance();
    StageMain* stageMain = new StageMain();
    stageManager.Register(stageMain);

    BulletManager::Instance().Initialize();

    deferred = std::make_unique<DeferredRenderer>();

    LightManager::Instance().Initialize();
    dirLight = std::make_shared<DirLight>(
        DirectX::XMFLOAT3(0.6f, -0.6f, 1.6f),
        DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f));
    LightManager::Instance().Register("scene_dir", dirLight);

    // SKY_MAP
    skymap = std::make_unique<SkyMap>(graphics.GetDevice().Get(), L"Resources/SkyMap/captured at (0, 0, 0)/skybox.dds");

    radialBlur = std::make_unique<RadialBlur>(graphics.GetDevice().Get());
    IBL_constant = std::make_unique<Constants<IBL_constants>>(Graphics::Instance().GetDevice().Get());
    create_ps_from_cso(graphics.GetDevice().Get(), "Shader/tone_map_ps.cso", toneMapPixelShader.GetAddressOf());

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
    BulletManager& bulletManager = BulletManager::Instance();

    //**********カメラの更新**********//
    camera->SetTrakkingTarget(player->GetGazingPoint());
    camera->SetPlayerOrientation(player->GetOrientation());
    camera->SetLockOnTarget(boss->GetPosition());
    camera->Update(elapsedTime);
    camera->CalcViewProjection(elapsedTime);

    //カメラの経過時間
    float cameraElapsedTime = camera->HitStopUpdate(elapsedTime);
    //**********プレイヤーの更新**********//
    player->Update(cameraElapsedTime);

    //**********ボスの更新**********//
    boss->SetLocationOfAttackTarget(player->GetPosition());
    boss->SetAttackTarget_height(player->GetHeight());
    boss->Update(cameraElapsedTime);

    camera->SetShakeDistance(
        DirectX::XMVectorGetX(DirectX::XMVector3Length(
            DirectX::XMVectorSubtract(
                DirectX::XMLoadFloat3(&player->GetPosition()),
                DirectX::XMLoadFloat3(&boss->GetPosition())))));

    //**********弾の更新**********//
    bulletManager.Update(cameraElapsedTime);

    //**********ステージの更新**********//
    StageManager::Instance().Update(cameraElapsedTime);
    
    //player->CalcAttack_vs_Enemy(boss->GetBodyCollision().capsule,
    //    boss->GetBodyCollision().height, boss->damagedFunction);

    JudgeCollision();
    if (player->GetIsDead() || boss->GetIsDead())
    {
        SceneManager::Instance().ChangeScene(new SceneTitle);
    }

    cameraElapsedTime_ = cameraElapsedTime;
}

void SceneGame::Render(float elapsedTime)
{
    Graphics& graphics = Graphics::Instance();
    StageManager& stageManager = StageManager::Instance();
    BulletManager& bulletManager = BulletManager::Instance();

    framebuffers[0]->clear(graphics.Get_DC().Get(),
        FB_FLAG::COLOR_DEPTH_STENCIL);
    framebuffers[1]->clear(graphics.Get_DC().Get(),
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
    IBL_constant->Bind(Graphics::Instance().Get_DC().Get(), 11, CB_FLAG::ALL);
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

    player->Render_f(cameraElapsedTime_);

    boss->Render_f(cameraElapsedTime_);

    bulletManager.Render(cameraElapsedTime_);

    //---------------------------UI----------------------------//
    //player->RenderUI(cameraElapsedTime_);

    //-------------------DebugPrimitive----------------------//
    graphics.SetGraphicStatePriset(ST_DEPTH::DepthON_WriteON, ST_BLEND::ALPHA, ST_RASTERIZER::WIREFRAME_CULL_BACK);
    //graphics.GetDebugRenderer()->RenderAlFigures(graphics.Get_DC().Get());

    graphics.SetGraphicStatePriset(ST_DEPTH::DepthON_WriteON, ST_BLEND::ALPHA, ST_RASTERIZER::CULL_NONE);
    framebuffers[0]->deactivate(graphics.Get_DC().Get());
    ID3D11ShaderResourceView* shader_resource_views[2]
    { framebuffers[0]->get_color_map().Get()/*, framebuffers[0]->depth_map().Get() */};
    
    framebuffers[1]->activate(graphics.Get_DC().Get());
    LightManager::Instance().Draw(shader_resource_views, 1);
    bit_block_transfer->blit(graphics.Get_DC().Get(), shader_resource_views, 0, 1);
    framebuffers[1]->deactivate(graphics.Get_DC().Get());
    
    //graphics.SetGraphicStatePriset(ST_DEPTH::DepthOFF_WriteOFF, ST_BLEND::NORMAL, ST_RASTERIZER::CULL_NONE);
    //bit_block_transfer->blit(graphics.Get_DC().Get(), shader_resource_views, 0, 1, toneMapPixelShader.Get());

    radialBlur->blit(graphics.Get_DC().Get(), framebuffers[1]->get_color_map().GetAddressOf());

//#if USE_IMGUI
#if 0
    stageManager.DebugGUI();
    camera->DebugGui();
    player->DebugGUI();
    boss->DebugDUI();

    bulletManager.DebugGUI();
    radialBlur->DebugGUI();
    LightManager::Instance().DebugGUI();
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

    imguiMenuBar("Game", "IBL", IBLImgui);
    if (IBLImgui)
    {
        if (ImGui::Begin("IBL", nullptr, ImGuiWindowFlags_None))
        {
            ImGui::DragFloat4("lightRoti", &IBL_constant->data.lightRoti.x);
            ImGui::DragFloat4("iblIntencity", &IBL_constant->data.iblIntencity.x);
        }
        ImGui::End();
    }
#endif

}

void SceneGame::JudgeCollision()
{
    boss->CalcAttack_vs_Player(player->collider, player->GetHeight(), 
        player->damagedFunction);

    player->CalcCollision_vs_Enemy(boss->GetBodyCollision().capsule,
        boss->GetBodyCollision().height);
    player->CalcAttack_vs_Enemy(boss->GetBodyCollision().capsule,
        boss->GetBodyCollision().height, boss->damagedFunction);

    BulletManager::Instance().CollisionBullet(player.get(), boss.get());
}

void SceneGame::DebugGui()
{
}

void SceneGame::SceneReset()
{
}

#include "scene_game.h"

#include "device.h"

#include "scene_manager.h"
#include "scene_title.h"
#include "scene_loading.h"

#include "bullet_manager.h"
#include "effect_manager.h"

#include "stage_manager.h"
#include "stage_main.h"


SceneGame::SceneGame()
{
}

void SceneGame::Initialize()
{
    Graphics& graphics = Graphics::Instance();
    
    //フレームバッファの生成
    framebuffers[0] = std::make_unique<framebuffer>(graphics.GetDevice().Get(), SCREEN_WIDTH, SCREEN_HEIGHT);
    framebuffers[1] = std::make_unique<framebuffer>(graphics.GetDevice().Get(), SCREEN_WIDTH, SCREEN_HEIGHT);
    framebuffers[2] = std::make_unique<framebuffer>(graphics.GetDevice().Get(), SCREEN_WIDTH, SCREEN_HEIGHT);
    bit_block_transfer = std::make_unique<fullscreen_quad>(graphics.GetDevice().Get());

    //各クラスの生成と初期化
    {
        camera = &Camera::Instance();
        camera->Initialize();
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
    }

    //ゲーム終了時の画像
    spriteVictory = std::make_unique<SpriteBatch>(
        graphics.GetDevice().Get(),
        L"Resources/Sprite/Game/victory.png", 1);
    spriteLose = std::make_unique<SpriteBatch>(
        graphics.GetDevice().Get(),
        L"Resources/Sprite/Game/lose.png", 1);

    //ゲーム終了フラグの初期化
    isEnd = false;

    //ゲーム中のBGM
    audios[0] = audio::_emplace(L"Resources/Sound/BGM/Electric_Highway.wav");
    audios[0]->play(30);
    audios[0]->volume(0.3f);

    // SKY_MAP
    skymap = std::make_unique<SkyMap>(graphics.GetDevice().Get(), L"Resources/SkyMap/captured at (0, 0, 0)/skybox.dds");

    //シェーダー生成
    radialBlur = std::make_unique<RadialBlur>(graphics.GetDevice().Get());
    glitch_CA = std::make_unique<Glitch_CA>(graphics.GetDevice().Get());
   
    IBL_constant = std::make_unique<Constants<IBL_constants>>(Graphics::Instance().GetDevice().Get());
    create_ps_from_cso(graphics.GetDevice().Get(), "Shader/tone_map_ps.cso", toneMapPixelShader.GetAddressOf());

}

void SceneGame::Finalize()
{
    //ステージ初期化
    StageManager::Instance().Clear();

}

void SceneGame::Update(float elapsedTime)
{
    //インスタンス設定
    Graphics& graphics = Graphics::Instance();
    GamePad& gamepad = Device::Instance().GetGamePad();
    Mouse& mouse = Device::Instance().GetMouse();
    BulletManager& bulletManager = BulletManager::Instance();

    //ゲーム終了フラグが立ってたら
    if (isEnd)
    {
        const GamePadButton anyButton =
            GamePad::BTN_A
            | GamePad::BTN_B
            | GamePad::BTN_X
            | GamePad::BTN_Y;
        //何かしらボタン押したらタイトルに戻る
        if (gamepad.GetButton() & anyButton)
        {
            audios[0]->stop();
            SceneManager::Instance().ChangeScene(new SceneTitle);
        }
        return;
    }

    //**********カメラの更新**********//
    camera->SetTrakkingTarget(player->GetGazingPoint());
    camera->SetPlayerOrientation(player->GetOrientation());
    camera->SetLockOnTarget(boss->GetPosition());
    camera->Update(elapsedTime);
    camera->CalcViewProjection(elapsedTime);
    camera->SetShakeDistance(
        DirectX::XMVectorGetX(DirectX::XMVector3Length(
            DirectX::XMVectorSubtract(
                DirectX::XMLoadFloat3(&player->GetPosition()),
                DirectX::XMLoadFloat3(&boss->GetPosition())))));


    //ヒットストップがオンなら動きを止める
    isHitStop = camera->HitStopUpdate(elapsedTime);
    if (isHitStop)
    {
        return;
    }

    //**********プレイヤーの更新**********//
    player->Update(elapsedTime);
    player->SetBossPosition(boss->GetTargetPosition());

    //**********ボスの更新**********//
    boss->SetLocationOfAttackTarget(player->GetPosition());
    boss->SetAttackTarget_height(player->GetHeight());
    boss->Update(elapsedTime);

    //**********弾の更新**********//
    bulletManager.Update(elapsedTime);

    //**********ステージの更新**********//
    StageManager::Instance().Update(elapsedTime);
    
    //当たり判定
    JudgeCollision();

    //シェーダー系のデバッグのオンオフ
    if (!radialBlur->GetIsDebug())
    {
        radialBlur->radial_blur_constant->DataSet(player->GetRadialBlur());
    }
    if (!glitch_CA->GetIsDebug())
    {
        glitch_CA->glitch_CA_constant->DataSet(player->GetGlitch_CA());
    }

    //**********エフェクトの更新**********//
    EffectManager::Instance().Update(elapsedTime);

    //色収差タイマーが20フレーム以上にならないようにセット
    glitch_CA->glitch_CA_constant->data.time += elapsedTime;
    if (glitch_CA->glitch_CA_constant->data.time > 20.0f)
        glitch_CA->glitch_CA_constant->data.time = 1.0f;

    //プレイヤーかボスのHPがなくなったらゲーム終了フラグを立てる
    if (player->GetIsDead() || boss->GetIsDead())
    {
        isEnd = true;
    }

#ifdef _DEBUG
    //F1を押すとマウスカーソルを中央固定切り替え
    static bool fixed = false;
    if (mouse.GetButtonDown() & Mouse::BTN_F1)
    {
        fixed = !fixed;
        mouse.SetIsFixedCursor(fixed);
    }
#endif
}

void SceneGame::Render(float elapsedTime)
{
    //インスタンス設定
    Graphics& graphics = Graphics::Instance();
    StageManager& stageManager = StageManager::Instance();
    BulletManager& bulletManager = BulletManager::Instance();

    //ヒットストップしていない場合に各描画を更新する
    if (!isHitStop)
    {
        //フレームバッファのクリア
        framebuffers[0]->clear(graphics.Get_DC().Get(),
            FB_FLAG::COLOR_DEPTH_STENCIL);
        framebuffers[1]->clear(graphics.Get_DC().Get(),
            FB_FLAG::COLOR_DEPTH_STENCIL);
        framebuffers[2]->clear(graphics.Get_DC().Get(),
            FB_FLAG::COLOR_DEPTH_STENCIL);

        //フレームバッファ0アクティブ
        framebuffers[0]->activate(graphics.Get_DC().Get(),
            FB_FLAG::COLOR_DEPTH_STENCIL);
        //***************************************************************//
        ///		    	            			スカイマップ		                             	  ///
        //***************************************************************//

        // 現在のビューポート情報を取得
        D3D11_VIEWPORT viewport;
        UINT num_viewports{ 1 };
        graphics.Get_DC().Get()->RSGetViewports(&num_viewports, &viewport);
        
        // アスペクト比を計算
        float aspect_ratio{ viewport.Width / viewport.Height };

        // 透視投影行列を作成
        DirectX::XMMATRIX P{ DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(30), aspect_ratio, 0.1f, 1000.0f) };
        
        // カメラの視点と注視点を設定
        DirectX::XMVECTOR eye{ DirectX::XMVectorSet(0,0,0,0) };
        DirectX::XMVECTOR focus{ DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 1.0f) };
        DirectX::XMVECTOR up{ DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) };
        
        // ビュー行列を取得
        DirectX::XMMATRIX V{ DirectX::XMLoadFloat4x4(&camera->GetView()) };

        // ビュー・プロジェクション行列を作成
        DirectX::XMFLOAT4X4 view_pro{};
        DirectX::XMStoreFloat4x4(&view_pro, V * P);

        // グラフィックの状態を設定（スカイマップ用）
        graphics.SetGraphicStatePriset(ST_DEPTH::DepthOFF_WriteOFF, ST_BLEND::ALPHA, ST_RASTERIZER::CULL_NONE);
        skymap->Blit(graphics.Get_DC().Get(), view_pro);

        // IBL（Image-Based Lighting）の定数バッファをバインド
        IBL_constant->Bind(Graphics::Instance().Get_DC().Get(), 11, CB_FLAG::ALL);

        //***************************************************************//
        ///						フォワードレンダリング					///
        //***************************************************************//
        graphics.SetGraphicStatePriset(
            ST_DEPTH::DepthON_WriteON,
            ST_BLEND::ALPHA,
            ST_RASTERIZER::CULL_NONE);

        // シーン内のオブジェクトをレンダリング
        stageManager.Render(elapsedTime);
        player->Render_f(elapsedTime);
        boss->Render_f(elapsedTime);
        bulletManager.Render(elapsedTime);

        // 3Dエフェクト描画
        {
            EffectManager::Instance().Render(camera->GetView(), camera->GetProjection());
        }

        //-------------------DebugPrimitive----------------------//
        graphics.SetGraphicStatePriset(ST_DEPTH::DepthON_WriteON, ST_BLEND::ALPHA, ST_RASTERIZER::WIREFRAME_CULL_BACK);
        //graphics.GetDebugRenderer()->RenderAlFigures(graphics.Get_DC().Get());

        graphics.SetGraphicStatePriset(ST_DEPTH::DepthON_WriteON, ST_BLEND::ALPHA, ST_RASTERIZER::CULL_NONE);
        
        // フレームバッファをデアクティベート
        framebuffers[0]->deactivate(graphics.Get_DC().Get());

        // 現在のレンダーターゲットを保存
        ID3D11ShaderResourceView* shader_resource_views[2]
        { framebuffers[0]->get_color_map().Get()/*, framebuffers[0]->depth_map().Get() */ };

        //現在の画面を保存して出力
        framebuffers[1]->activate(graphics.Get_DC().Get());
        LightManager::Instance().Draw(shader_resource_views, 1);
        bit_block_transfer->Blit(graphics.Get_DC().Get(), shader_resource_views, 0, 1);
        framebuffers[1]->deactivate(graphics.Get_DC().Get());

        // ポストプロセス処理（ラジアルブラー）
        framebuffers[2]->activate(graphics.Get_DC().Get());
        radialBlur->Blit(graphics.Get_DC().Get(), framebuffers[1]->get_color_map().GetAddressOf());

        framebuffers[2]->deactivate(graphics.Get_DC().Get());
    }

    // 最終的にフレームバッファをデアクティベート
    glitch_CA->Blit(graphics.Get_DC().Get(), framebuffers[2]->get_color_map().GetAddressOf());

    //---------------------------UI----------------------------//
    player->RenderUI(elapsedTime);
    boss->RenderUI(elapsedTime);

    // ゲームの終了時、勝敗の表示処理
    if (isEnd)
    {
        if (boss->GetIsDead())
        {
            spriteVictory->begin(graphics.Get_DC().Get());
            spriteVictory->render(graphics.Get_DC().Get(),
                { 350,350 }, { 0.7f, 0.7f }, { 0.8f,0.8f,1.0f,1.0f }, 0);
            spriteVictory->end(graphics.Get_DC().Get());
        }
        else
        {
            spriteLose->begin(graphics.Get_DC().Get());
            spriteLose->render(graphics.Get_DC().Get(),
                { 500,350 }, { 0.7f, 0.7f }, { 1.0f,0,1,1.0f }, 0);
            spriteLose->end(graphics.Get_DC().Get());
        }
    }

    //ImGui描画
#if USE_IMGUI
//#if 0
    stageManager.DebugGUI();
    camera->DebugGui();
    player->DebugGUI();
    boss->DebugDUI();
    bulletManager.DebugGUI();
    radialBlur->DebugGUI();
    glitch_CA->DebugGUI();
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
    // フレーム表示
    {
        ImGui::Begin("##frame stage_rate");
    
        static float temp_value = 0;
        static float values[90] = {};
        static int values_offset = 0;
        static float refresh_time = 0.0f;
        static const float PLOT_SENSE = 0.2f;
    
        refresh_time += elapsedTime;
        if (static_cast<int>(refresh_time / PLOT_SENSE) >= 1)
        {
            values_offset = values_offset >= IM_ARRAYSIZE(values) ? 0 : values_offset;
            values[values_offset] = temp_value = elapsedTime * 1000.0f;
    
            ++values_offset;
            refresh_time = 0;
        }
    
        char overlay[32];
        sprintf_s(overlay, "now: %d fps  %.3f ms", static_cast<int>(1000.0f / temp_value), temp_value);
        ImGui::PlotLines("##frame", values, IM_ARRAYSIZE(values), values_offset, overlay, 0, 20, ImVec2(ImGui::GetWindowSize().x * 0.75f, ImGui::GetWindowSize().y * 0.5f));
    
        ImGui::End();
    }

#endif

}

void SceneGame::JudgeCollision()
{
    //ボスの攻撃当たり判定
    boss->CalcAttack_vs_Player(player->collider, player->GetHeight(), 
        player->damagedFunction);

    //プレイヤーとボスの当たり判定
    player->CalcCollision_vs_Enemy(boss->GetBodyCollision().capsule,
        boss->GetBodyCollision().height);

    //プレイヤーの攻撃当たり判定
    player->CalcAttack_vs_Enemy(boss->GetBodyCollision().capsule,
        boss->GetBodyCollision().height, boss->damagedFunction);

    //弾丸の当たり判定
    BulletManager::Instance().CollisionBullet(player.get(), boss.get());
}

void SceneGame::DebugGui()
{
}

void SceneGame::SceneReset()
{
}

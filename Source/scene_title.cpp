#include "scene_title.h"
#include "scene_loading.h"
#include "scene_game.h"
#include "device.h"
#include "scene_manager.h"
#include "imgui/imgui.h"

void SceneTitle::Initialize()
{
	Graphics& graphics = Graphics::Instance();
	spriteTitleBack = std::make_unique<SpriteBatch>(graphics.GetDevice().Get(), L"Resources/Sprite/Title/title_back.png", 1);
	selectedMenuState = TITLE_MENU::GAME_START;
	isStart = false;
}

void SceneTitle::Finalize()
{
}

void SceneTitle::Update(float elapsedTime)
{
	Mouse& mouse = Device::Instance().GetMouse();
	GamePad& gamePad = Device::Instance().GetGamePad();

	//メニューセレクト
	if (gamePad.GetAxis_LY() > 0.2f)
	{
		//上に倒したときはゲームスタート
		selectedMenuState = TITLE_MENU::GAME_START;
	}
	else if (gamePad.GetAxis_LY() < -0.2f)
	{
		//下に倒したときは抜ける
		selectedMenuState = TITLE_MENU::EXIT;
	}

	//ボタンを押したときの挙動
	switch (selectedMenuState)
	{
	case SceneTitle::TITLE_MENU::GAME_START:
		if (mouse.GetButton() & mouse.BTN_Z || gamePad.GetButton() & gamePad.BTN_A)
		{
			isStart = true;
		}
		break;
	case SceneTitle::TITLE_MENU::EXIT:
		if (mouse.GetButton() & mouse.BTN_Z || gamePad.GetButton() & gamePad.BTN_A)
		{
			PostQuitMessage(0);
		}
		break;
	default:
		break;
	}

	if (isStart)
	{
		SceneManager::Instance().ChangeScene(new SceneLoading(new SceneGame()));
		return;
	}
}

void SceneTitle::Render(float elapsedTime)
{
	Graphics& graphics = Graphics::Instance();

	graphics.SetGraphicStatePriset(
		ST_DEPTH::DepthON_WriteON,
		ST_BLEND::ALPHA,
		ST_RASTERIZER::CULL_NONE
	);
	//ID3D11DeviceContext* dc = graphics.Get_DC().Get();
	//ID3D11RenderTargetView* rtv = graphics.GetRenderTargetView().Get();
	//ID3D11DepthStencilView* dsv = graphics.GetDepthStencilView().Get();

	////画面クリア＆レンダーターゲット設定
	//FLOAT color[] = { 0.0f,0.0f,0.5f,1.0f };
	//dc->ClearRenderTargetView(rtv, color);
	//dc->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	//dc->OMSetRenderTargets(1, &rtv, dsv);


	spriteTitleBack->begin(graphics.Get_DC().Get());
	//spriteTitleBack->render(graphics.Get_DC().Get(),
	//	{ 0,0 }, { 1, 1 });
	spriteTitleBack->end(graphics.Get_DC().Get());
}

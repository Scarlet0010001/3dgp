#include "player_UI.h"
#include "camera.h"
#include "user.h"

PlayerHpGauge::PlayerHpGauge() :
	GaugeUI(L"Resources/Sprite/UI/Gauge/HPframeGauge.png",
		L"Resources/Sprite/UI/Gauge/HPGauge.png",
		nullptr)
{
	gauge.position = { 50.0f,45.0f };
	gauge.scale = { 0.45f,0.45f };
	gauge.color = { 0.0f,1.0f,0.5f,1.0f };
	
	gaugeBack.position = { 30.0f,30.0f };
	gaugeBack.scale = { 0.45f,0.45f };
	gaugeBack.color = { 0.7f,0.7f,0.7f,1.0f };

	diffColor = { 1.0f,0.0f, 0.0f, 1.0f };
}

PlayerBoostGauge::PlayerBoostGauge():
	GaugeUI(L"Resources/Sprite/UI/Gauge/BoostGauge.png",
	L"Resources/Sprite/UI/Gauge/BoostGauge.png",
	nullptr)
{
	gauge.position = { 40.0f,835.0f };
	gauge.scale = { 0.35f,0.3f };
	gauge.color = { 0.0f,1.0f,1.0f,1.0f };
	gauge.angle = -90;

	gaugeBack.position = { 26.5f,930.0f };
	gaugeBack.scale = { 0.42f,0.37f };
	gaugeBack.color = { 0.7f,0.7f,0.7f,1.0f };
	gaugeBack.angle = -90;

	diffColor = { 1.0f,0.0f, 0.0f, 1.0f };
}

PlayerLockon::PlayerLockon(const wchar_t* filename)
{
	Graphics& graphics = Graphics::Instance();
	sprite = std::make_unique<SpriteBatch>(graphics.GetDevice().Get(), filename, 1);

	element.position = {};
	element.scale = { 0.2f,0.2f };
	element.color = { 1.0f,1.0f,1.0f,1.0f };
	element.angle = 0.0f;

	element.texpos = {};
	element.texsize = { sprite->GetTexWidth(), sprite->GetTexHeight() };
	element.pivot = { sprite->GetTexWidth() * 0.5f, sprite->GetTexHeight() * 0.5f };
}

void PlayerLockon::Update(float elapsed_time)
{
	//ロックオンしてたら表示
	isDisplay = Camera::Instance().GetLockOn();

	//距離に応じて大きさを変える
	if (distance < minDistance)
	{
		//最低距離未満だった場合サイズをmaxScaleにする
		element.scale = maxScale;
	}
	else if (distance >= maxDistance)
	{
		//最大距離以上だった場合サイズをminScaleにする
		element.scale = minScale;
	}
	else
	{
		//現在のプレイヤーとボスの距離と最低距離から大きさを計算する
		float calcScale = (minDistance / distance) * 0.2f;
		element.scale = { calcScale,calcScale };
	}
}

void PlayerLockon::Render(ID3D11DeviceContext* dc)
{
	//ロックオンじゃなければ表示しない
	if (!isDisplay)return;

	//--ロックオン描画--//
	sprite->begin(dc);
	sprite->render(dc, element.position, element.scale,
		element.pivot, element.color, element.angle,
		element.texpos, element.texsize);
	sprite->end(dc);

}

void PlayerHpGauge::DebugGUI()
{
#ifdef USE_IMGUI
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
	imguiMenuBar("UI", "PlayerHP", displayImgui);

	if (displayImgui)
	{
		if (ImGui::Begin("HPGauge", nullptr, ImGuiWindowFlags_None))
		{
			//エレメント
			if (ImGui::CollapsingHeader("Gauge", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::DragFloat2("Position", &gauge.position.x);
				ImGui::DragFloat2("Scale", &gauge.scale.x);
				ImGui::DragFloat2("Pivot", &gauge.pivot.x);
				if (ImGui::CollapsingHeader("color_picker", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::ColorPicker4("Color", &gauge.color.x);
				}
				ImGui::DragFloat("Angle", &gauge.angle);
				ImGui::DragFloat2("Texpos", &gauge.texpos.x);
				ImGui::DragFloat2("Texsize", &gauge.texsize.x);
			}
			if (ImGui::CollapsingHeader("GaugeBack", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::DragFloat2("BackPosition", &gaugeBack.position.x);
				ImGui::DragFloat2("BackScale", &gaugeBack.scale.x);
				ImGui::DragFloat2("BackPivot", &gaugeBack.pivot.x);
				if (ImGui::CollapsingHeader("Backcolor_picker", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::ColorPicker4("BackColor", &gaugeBack.color.x);
				}
				ImGui::DragFloat("BackAngle", &gaugeBack.angle);
				ImGui::DragFloat2("BackTexpos", &gaugeBack.texpos.x);
				ImGui::DragFloat2("BackTexsize", &gaugeBack.texsize.x);
			}
			ImGui::ColorPicker4("DiffColor", &diffColor.x);
			ImGui::DragFloat("nowPercent", &nowPercent);
			ImGui::DragFloat("oldPercent", &oldPercent);
		}
		ImGui::End();
	}
#endif
}

void PlayerBoostGauge::DebugGUI()
{
#ifdef USE_IMGUI
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
	imguiMenuBar("UI", "PlayerBoost", displayImgui);

	if (displayImgui)
	{
		if (ImGui::Begin("BoostGauge", nullptr, ImGuiWindowFlags_None))
		{
			//エレメント
			if (ImGui::CollapsingHeader("Gauge", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::DragFloat2("Position", &gauge.position.x);
				ImGui::DragFloat2("Scale", &gauge.scale.x);
				ImGui::DragFloat2("Pivot", &gauge.pivot.x);
				if (ImGui::CollapsingHeader("color_picker", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::ColorPicker4("Color", &gauge.color.x);
				}
				ImGui::DragFloat("Angle", &gauge.angle);
				ImGui::DragFloat2("Texpos", &gauge.texpos.x);
				ImGui::DragFloat2("Texsize", &gauge.texsize.x);
			}
			if (ImGui::CollapsingHeader("GaugeBack", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::DragFloat2("BackPosition", &gaugeBack.position.x);
				ImGui::DragFloat2("BackScale", &gaugeBack.scale.x);
				ImGui::DragFloat2("BackPivot", &gaugeBack.pivot.x);
				if (ImGui::CollapsingHeader("Backcolor_picker", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::ColorPicker4("BackColor", &gaugeBack.color.x);
				}
				ImGui::DragFloat("BackAngle", &gaugeBack.angle);
				ImGui::DragFloat2("BackTexpos", &gaugeBack.texpos.x);
				ImGui::DragFloat2("BackTexsize", &gaugeBack.texsize.x);
			}
			ImGui::ColorPicker4("DiffColor", &diffColor.x);
			ImGui::DragFloat("nowPercent", &nowPercent);
			ImGui::DragFloat("oldPercent", &oldPercent);
		}
		ImGui::End();
	}
#endif
}

void PlayerLockon::DebugGUI()
{
#ifdef USE_IMGUI
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
	imguiMenuBar("UI", "Lockon", displayImgui);

	if (displayImgui)
	{
		if (ImGui::Begin("Lockon", nullptr, ImGuiWindowFlags_None))
		{
			//エレメント
			if (ImGui::CollapsingHeader("Element", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::DragFloat2("Position", &element.position.x);
				ImGui::DragFloat2("Scale", &element.scale.x);
				ImGui::DragFloat2("Pivot", &element.pivot.x);
				if (ImGui::CollapsingHeader("color_picker", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::ColorPicker4("Color", &element.color.x);
				}
				ImGui::DragFloat("Angle", &element.angle);
				ImGui::DragFloat2("Texpos", &element.texpos.x);
				ImGui::DragFloat2("Texsize", &element.texsize.x);
			}
			if (ImGui::CollapsingHeader("Others", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::DragFloat("distance", &distance);
			}
		}
		ImGui::End();
	}
#endif
}

void PlayerLockon::SetPosition(DirectX::XMFLOAT3 p)
{
	Graphics& graphics = Graphics::Instance();
	Camera& camera = Camera::Instance();

	//ビューポート
	D3D11_VIEWPORT viewport;
	UINT numViewports = 1;
	graphics.Get_DC()->RSGetViewports(&numViewports, &viewport);

	//変換行列
	DirectX::XMMATRIX View = DirectX::XMLoadFloat4x4(&camera.GetView());
	DirectX::XMMATRIX Projection = DirectX::XMLoadFloat4x4(&camera.GetProjection());
	DirectX::XMMATRIX Worid = DirectX::XMMatrixIdentity();

	DirectX::XMVECTOR lockPosition = DirectX::XMLoadFloat3(&p);
	DirectX::XMVECTOR ScreenPosition =
		DirectX::XMVector3Project(
			lockPosition,
			viewport.TopLeftX,
			viewport.TopLeftY,
			viewport.Width,
			viewport.Height,
			viewport.MinDepth,
			viewport.MaxDepth,
			Projection,
			View,
			Worid
		);

	DirectX::XMFLOAT3 scrPos;
	DirectX::XMStoreFloat3(&scrPos, ScreenPosition);

	//z深度の範囲が0～1だからそれ以外を省く
	if (scrPos.z > 1.0f || scrPos.z < 0)return;

	element.position.x = scrPos.x;
	element.position.y = scrPos.y;
}

PlayerUI::PlayerUI()
{
	//UIの生成
	hpGauge = std::make_unique <PlayerHpGauge>();
	boostGauge = std::make_unique <PlayerBoostGauge>();
	lockon = std::make_unique<PlayerLockon>(L"Resources/Sprite/UI/lockon.PNG");
}

void PlayerUI::Update(float elapsed_time)
{
	hpGauge->Update(elapsed_time);
	boostGauge->Update(elapsed_time);
	lockon->Update(elapsed_time);
}

void PlayerUI::Render()
{
	Graphics& graphics = Graphics::Instance();
	hpGauge->Render(graphics.Get_DC().Get());
	boostGauge->Render(graphics.Get_DC().Get());
	lockon->Render(graphics.Get_DC().Get());
}

void PlayerUI::DebugGUI()
{
	hpGauge->DebugGUI();
	boostGauge->DebugGUI();
	lockon->DebugGUI();
}

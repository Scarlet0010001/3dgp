#include "player_UI.h"
#include "user.h"

PlayerHpGauge::PlayerHpGauge() :
	GaugeUI(L"Resources/Sprite/UI/Player/Player_HPframe.png",
		L"Resources/Sprite/UI/Player/Player_HP.png",
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

PlayerBoostGauge::PlayerBoostGauge(): GaugeUI(L"Resources/Sprite/UI/Player/Player_Gauge.png",
	L"Resources/Sprite/UI/Player/Player_Gauge.png",
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

PlayerUI::PlayerUI()
{
	hpGauge = std::make_unique <PlayerHpGauge>();
	boostGauge = std::make_unique <PlayerBoostGauge>();
}

void PlayerUI::Update(float elapsed_time)
{
	hpGauge->Update(elapsed_time);
	boostGauge->Update(elapsed_time);
}

void PlayerUI::Render()
{
	Graphics& graphics = Graphics::Instance();
	hpGauge->Render(graphics.Get_DC().Get());
	boostGauge->Render(graphics.Get_DC().Get());
}

void PlayerUI::DebugGUI()
{
	hpGauge->DebugGUI();
	boostGauge->DebugGUI();
}


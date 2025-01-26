#include "player_UI.h"
#include "user.h"

PlayerHpGauge::PlayerHpGauge() :
	GaugeUI(L"Resources/Sprite/UI/Player/Player_HPframe_small.png",
		L"Resources/Sprite/UI/Player/Player_HP_small.png",
		nullptr)
{
	gauge.position = { 30.0f,30.0f };
	gauge.scale = { 0.15f, 0.1f };
	gauge.color = { 0.5f,1,0,1 };
	diffColor = { 1.0f,0.0f, 0.0f, 1.0f };
}

void PlayerHpGauge::DebugGUI()
{
#ifdef USE_IMGUI
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
	imguiMenuBar("UI", "player", displayImgui);

	if (displayImgui)
	{
		if (ImGui::Begin("playerUI", nullptr, ImGuiWindowFlags_None))
		{
			//ÉGÉåÉÅÉìÉg
			if (ImGui::CollapsingHeader("Element", ImGuiTreeNodeFlags_DefaultOpen))
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
}

void PlayerUI::Update(float elapsed_time)
{
	hpGauge->Update(elapsed_time);
}

void PlayerUI::Render()
{
	Graphics& graphics = Graphics::Instance();
	hpGauge->Render(graphics.Get_DC().Get());
	hpGauge->DebugGUI();
}

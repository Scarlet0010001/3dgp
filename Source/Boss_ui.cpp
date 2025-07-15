#include "Boss_ui.h"
#include "user.h"

BossHpGauge::BossHpGauge():
	GaugeUI(L"Resources/Sprite/UI/Gauge/HPframeGauge.png",
		L"Resources/Sprite/UI/Gauge/HPGauge.png",
		nullptr)
{
	gauge.position = { 285.0f,971.1f };
	gauge.scale = { 0.8f,0.15f };
	gauge.color = { 1.0f,0.0f,0.0f,1.0f };

	gaugeBack.position = { 250.0f,965.0f };
	gaugeBack.scale = { 0.8f,0.15f };
	gaugeBack.color = { 1.0f,0.79f,0.5f,1.0f };

	diffColor = { 0.9f,0.55f, 1.0f, 1.0f };

}

void BossHpGauge::DebugGUI()
{
#ifdef USE_IMGUI
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
	ImguiMenuBar("UI", "BossHP", displayImgui);

	if (displayImgui)
	{
		if (ImGui::Begin("BossHPGauge", nullptr, ImGuiWindowFlags_None))
		{
			//ÉGÉåÉÅÉìÉg
			if (ImGui::CollapsingHeader("BossGauge", ImGuiTreeNodeFlags_DefaultOpen))
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
			if (ImGui::CollapsingHeader("BossGaugeBack", ImGuiTreeNodeFlags_DefaultOpen))
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

BossUI::BossUI()
{
	//UIÇÃê∂ê¨
	hpGauge = std::make_unique <BossHpGauge>();
}

void BossUI::Update(float elapsed_time)
{
	hpGauge->Update(elapsed_time);
}

void BossUI::Render()
{
	Graphics& graphics = Graphics::Instance();
	hpGauge->Render(graphics.Get_DC().Get());
}

void BossUI::DebugGUI()
{
	hpGauge->DebugGUI();
}

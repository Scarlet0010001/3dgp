#pragma once
#include "gauge_UI.h"

class PlayerHpGauge : public GaugeUI
{
public:
	PlayerHpGauge();
	~PlayerHpGauge() override {};

	void DebugGUI();
private:
	bool displayImgui = false;

};

class PlayerUI
{
public:
	PlayerUI();
	~PlayerUI() {};
	void Update(float elapsed_time);
	void Render();

	void SetHPPercent(float arg) { hpGauge->SetPercent(arg); }
private:
	std::unique_ptr<PlayerHpGauge> hpGauge;
};
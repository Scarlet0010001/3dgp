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

class PlayerBoostGauge : public GaugeUI
{
public:
	PlayerBoostGauge();
	~PlayerBoostGauge() override {};

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
	void DebugGUI();

	void SetHPPercent(float arg) { hpGauge->SetPercent(arg); }
	void SetBoostPercent(float arg) { boostGauge->SetPercent(arg); }
private:
	std::unique_ptr<PlayerHpGauge> hpGauge;
	std::unique_ptr<PlayerBoostGauge> boostGauge;
};
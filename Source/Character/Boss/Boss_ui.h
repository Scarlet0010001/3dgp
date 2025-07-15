#pragma once
#include "Ui/gauge_ui.h"

class BossHpGauge : public GaugeUI
{
public:
	BossHpGauge();
	~BossHpGauge() override {};

	void DebugGUI()override;
};

class BossUI
{
public:
	BossUI();
	~BossUI() {};
	void Update(float elapsed_time);
	void Render();
	void DebugGUI();

	void SetHPPercent(float arg) { hpGauge->SetPercent(arg); }
private:
	std::unique_ptr<BossHpGauge> hpGauge;

};


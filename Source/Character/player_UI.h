#pragma once
#include "gauge_UI.h"

class PlayerHpGauge : public GaugeUI
{
public:
	PlayerHpGauge();
	~PlayerHpGauge() override {};

	void DebugGUI()override;
};

class PlayerBoostGauge : public GaugeUI
{
public:
	PlayerBoostGauge();
	~PlayerBoostGauge() override {};

	void DebugGUI()override;
};

class PlayerLockon : public UI
{
public:
	PlayerLockon() {}
	PlayerLockon(const wchar_t* filename);
	~PlayerLockon() override {};

	//--------< 関数 >--------//
	void Update(float elapsed_time) override;
	void Render(ID3D11DeviceContext* dc) override;

	void DebugGUI()override;

	//--------設定------------//
	void SetPosition(DirectX::XMFLOAT3 p);
	void SetIsDisplay(bool on) { isDisplay = on; };
	void SetDistance(float d) { distance = d; };

private:
	//-----------------変数-----------------//
	std::unique_ptr<SpriteBatch> sprite{ nullptr };

	Element element;

	float distance = 0.0f;

	bool isDisplay = false;

	//-----------------定数-----------------//
	//最大サイズ
	const DirectX::XMFLOAT2 maxScale{ 0.2f,0.2f };
	//最小サイズ
	const DirectX::XMFLOAT2 minScale{ 0.05f,0.05f };

	//最低距離
	const float minDistance = 40.0f;
	//最大距離
	const float maxDistance = 158.4f;
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

	void SetLockonPosition(DirectX::XMFLOAT3 p) { lockon->SetPosition(p); }
	void SetLockonDistance(float d) { lockon->SetDistance(d); }
private:
	std::unique_ptr<PlayerHpGauge> hpGauge;
	std::unique_ptr<PlayerBoostGauge> boostGauge;
	std::unique_ptr<PlayerLockon> lockon;
};
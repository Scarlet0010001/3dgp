#pragma once
#include "UI.h"
class GaugeUI :
    public UI
{
public:
	//--------<constructor/destructor>--------//
	GaugeUI() :nowPercent(1)
	{}
	GaugeUI(const wchar_t* back_filename, const wchar_t* body_filename, const wchar_t* frame_filename);
	virtual ~GaugeUI() {}

	//--------< ŠÖ” >--------//
	void Update(float elapsed_time) override;
	void Render(ID3D11DeviceContext* dc) override;

	void SetPercent(float per) { nowPercent = per; }
	void SetAnimation(bool anim) { animation = anim; }
	void SetAngle(float ang) { gauge.angle = ang; }
	void SetPosition(DirectX::XMFLOAT2 pos) { gauge.position = pos; }
	void SetScale(DirectX::XMFLOAT2 scale) { gauge.scale = scale; }
	void SetTexSize(DirectX::XMFLOAT2 texsize) { gauge.texsize = texsize; }
	void SetColor(DirectX::XMFLOAT4 color) { gauge.color = color; }
	void SetDiffColor(DirectX::XMFLOAT4 color) { diffColor = color; }

protected:
	//--------< •Ï” >--------//
	std::unique_ptr<SpriteBatch> frame{ nullptr };
	std::unique_ptr<SpriteBatch> back{ nullptr };
	std::unique_ptr<SpriteBatch> body{ nullptr };
	DirectX::XMFLOAT4 diffColor = { 1.0f,1.0f,1.0f,1.0f };
	Element gauge;
	Element gaugeBack;
	float nowPercent;
	float oldPercent;
	bool animation = false;

};


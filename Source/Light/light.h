#pragma once
#include "graphics.h"
#include "constant.h"
class Light
{
public:
	Light();
	~Light() {}

	//-------<関数>-------//
	virtual void DebugGUI(std::string name) {};
protected:
	//定数バッファ
	struct DirLightParam {
		DirectX::XMFLOAT4 dirLightDirection;
		DirectX::XMFLOAT4 dirLightColor;
	};

public:
	std::unique_ptr<Constants<DirLightParam>> lightConstants{};
	std::string name;

};

//平行光_DirectionalLight
class DirLight :public Light
{
public:
	DirLight(DirectX::XMFLOAT3 dir, DirectX::XMFLOAT3 color);
	~DirLight() {}

	void DebugGUI(std::string name) override;

	void SetDirection(DirectX::XMFLOAT3 direction);
	void SetColor(DirectX::XMFLOAT3 color);

	DirectX::XMFLOAT3 GetDirection();
	DirectX::XMFLOAT3 GetColor();
};

//点光源
class PointLight :public Light
{
public:
	PointLight(DirectX::XMFLOAT3 position, float distance, DirectX::XMFLOAT3 color);
	~PointLight() {}

	void DebugGUI(std::string name) override;

	void SetPosition(DirectX::XMFLOAT3 direction);
	void SetColor(DirectX::XMFLOAT3 color);
	void SetDistance(float d);

	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetColor();
	float GetDistance();
};
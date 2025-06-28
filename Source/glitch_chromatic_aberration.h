#pragma once
#include <DirectXMath.h>
#include "graphics.h"
#include "sprite.h"
#include "constant.h"
#include "fullscreen_quad.h"

//GlitchChromaticAberration
//色収差グリッチ
class Glitch_CA
{
public:
	Glitch_CA(ID3D11Device* device);
	~Glitch_CA() {}

	void DebugGUI();

	void Blit(ID3D11DeviceContext* immediate_context, ID3D11ShaderResourceView** shader_resource_view);

	bool GetIsDebug() { return isDebug; }

	struct Glitch_CA_constants
	{
		float				time = 0.0f; //経過時間
		float				density = 0.0f; //密度
		float				shift = 0.0f; //ずらす幅
		float				randFloat = 0.0f; //ランダム性

		DirectX::XMFLOAT2	XShift = { 0.0f,0.0f }; //xの横をどの程度ずらすのか
		DirectX::XMFLOAT2	YShift = { 0.0f,0.0f }; //yの横をどの程度ずらすのか
		float				XShifting = 0.0f; //色収差のxの位置をずらす
		float				YShifting = 0.0f; //色収差のyの位置をずらす    
		float				extension = 0.0f; //uvの拡張
		float				uvSlider = 0.0f; //左上に流す
		float				brightness = 0.0f; //明るさ
		float				glitchMaskRadius = 0; //マスク半径
		int					glitchSamplingCount = 1; //回数
		DirectX::XMFLOAT2	center = { 0.5f,0.5f }; //中心
		DirectX::XMFLOAT3	dummy;
	};
	std::unique_ptr<Constants<Glitch_CA_constants>> glitch_CA_constant{};
private:
	//	色収差
	Microsoft::WRL::ComPtr<ID3D11Buffer> glitch_CA_ConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> glitch_CA_SamplerState;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> glitch_CA_PixelShader;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> glitch_CA_ShaderResourceView[2];

	std::unique_ptr<fullscreen_quad> glitch_CA_Quad;

	bool isDebug = false;

	bool displayGlitch_CA_Imgui = false;
};


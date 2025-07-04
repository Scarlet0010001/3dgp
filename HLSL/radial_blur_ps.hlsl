// UNIT.32
#include "fullscreen_quad.hlsli"

cbuffer radialBlurConstants : register(b2)
{
	float2 blurCenter;	//ブラーの中心座標（UV）
	float blurStrength; //ブラーの強さ
	float blurRadius;   //ブラーが開始する距離
	float blurDecay;    //ブラーの減衰係数
	float blurTimer;    //アニメーション用タイマー（未使用）
};

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
#define LINEAR_BORDER_BLACK 3
#define LINEAR_BORDER_WHITE 4
#define LINEAR_CLAMP 5

SamplerState sampler_states[6] : register(s0); //6種類のサンプラーステート
Texture2D texture_maps[4] : register(t0);      //最大4枚のテクスチャ（0番目を使用）

float4 main(VS_OUT pin) : SV_TARGET
{
	const int samples = 16; //サンプリング数（精度と負荷に関わる）

	float2 center_to_pixel = pin.texcoord - blurCenter; //ブラー中心から現在ピクセルへのベクトル
	float distance = length(center_to_pixel);			//ブラー中心からの距離

	float factor = blurStrength / float(samples) * distance; //ブラーの影響係数
#if 1
	//距離に応じてブラー強度を滑らかに減衰させる
	factor *= smoothstep(blurRadius, blurRadius * blurDecay, distance);
#endif

	float3 color = 0.0; //ブラー適用後の色
	for (int i = 0; i < samples; i++)
	{
		//サンプリング位置を線形に遠ざける
		float sample_offset = 1.0 - factor * i;
		color += texture_maps[0].Sample(sampler_states[LINEAR_CLAMP], blurCenter + (center_to_pixel * sample_offset)).rgb;
	}
	color /= float(samples); //平均を取ってブラー結果とする

#if 0
	// トーンマッピング処理（HDR→SDR変換）
	const float exposure = 1.2;
	color = 1 - exp(-color * exposure);

	// ガンマ補正（γ変換）
	const float gamma = 2.2;
	color = pow(color, 1.0 / gamma);
#endif

	return float4(color, 1); //アルファは常に1（不透明）
}
#include "fullscreen_quad.hlsli"
#include "constant_buffer.hlsli"
#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
#define CLAMP 3
SamplerState sampler_states[4] : register(s0);
Texture2D texture_maps[4] : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
	float4 color = texture_maps[0].Sample(sampler_states[ANISOTROPIC], pin.texcoord);
	float alpha = color.a;

	//smoothstep をグレースケール変換に適用して、
	//RGB色の明るさを指定された範囲内で滑らかに
	//変化させています。これにより、グレースケール変換が
	//自然で視覚的に滑らかな結果を生むことができる
	color.rgb = smoothstep(smoothstep_minEdge, smoothstep_maxEdge,
		dot(color.rgb, float3(0.299, 0.587, 0.114))) * color.rgb;
	return float4(color.rgb, alpha);
}
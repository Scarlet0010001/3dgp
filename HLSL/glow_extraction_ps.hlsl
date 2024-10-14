// UNIT.99
#include "constant_buffer.hlsli"
#include "common.hlsli"

Texture2D hdr_color_buffer_texture : register(t0);
float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
	float4 sampled_color = hdr_color_buffer_texture.Sample(sampler_states[POINT], texcoord);
	return  float4(smoothstep(bloom_extraction_threshold, bloom_extraction_threshold + 0.5, dot(sampled_color.rgb, float3(0.299, 0.587, 0.114))) * sampled_color.rgb, sampled_color.a);
}

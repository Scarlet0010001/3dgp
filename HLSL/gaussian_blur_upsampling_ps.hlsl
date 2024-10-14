// UNIT.99
#include "constant_buffer.hlsli"
#include "common.hlsli"

static const uint downsampled_count = 6;
Texture2D downsampled_textures[downsampled_count] : register(t0);

float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
	float3 sampled_color = 0;
	[unroll]
	for (uint downsampled_index = 0; downsampled_index < downsampled_count; ++downsampled_index)
	{
		sampled_color += downsampled_textures[downsampled_index].Sample(sampler_states[ANISOTROPIC], texcoord).xyz;
	}
	return float4(sampled_color * blur_convolution_intensity, 1);
}

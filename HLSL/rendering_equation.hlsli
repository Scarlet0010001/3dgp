#ifndef __RENDERING_EQUATION_HLSL__
#define __RENDERING_EQUATION_HLSL__

// UNIT.99
#define PI 3.141592653

float3 fresnel_schlick(float NoL, float3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - NoL, 5.0);
}

float half_tone(float repeat_rate, float dot_size, float2 st)
{
	const float size = 1.0 / repeat_rate;
	const float2 cell_size = size;
	const float2 cell_center = cell_size * 0.5;

	const float2 st_local = fmod(abs(st), cell_size);
	const float dist = length(st_local - cell_center);
	const float radius = cell_center.x * dot_size;

	const float threshold = length(ddx(dist) - ddy(dist));

	return smoothstep(dist - threshold, dist + threshold, radius);
}

float3 blend_soft_light(float3 base, float3 blend)
{
	return lerp(
		sqrt(base) * (2.0 * blend - 1.0) + 2.0 * base * (1.0 - blend),
		2.0 * base * blend + base * base * (1.0 - 2.0 * blend),
		step(base, 0.5)
	);
}

float4 degamma(float4 color)
{
	return float4(pow(color.rgb, 2.2), color.a);
}

float4 gamma(float4 color)
{
	return float4(pow(color.rgb, 1 / 2.2), color.a);
}

#include "common.hlsli"
Texture2D ramp_map : register(t7);

float3 rendering_equation(float3 L, float3 V, float3 N, float3 Lc, float3 Kd, float3 Ks, float shininess)
{
	const float smoothness = shininess;
#if 1
	float3 irradiance = Lc * max(0, (dot(N, L) * 0.5) + 0.5);

	float3 diffuse_radiance = Kd * irradiance/* / PI*/;

	float3 H = normalize(L + V);
	float3 specular_radiance = (smoothness + 8) / (8 * PI) * pow(max(0, dot(N, H)), smoothness) * Ks * irradiance;
#else
	// Toon shading
	float diffuse_factor = max(0, ((dot(N, L) * 0.5) + 0.5));
	
	const int dh = 1.0 / 5;
	const int ramp = 4;
	const float v = ramp * dh + (dh * 0.5);
	float ramp_factor = degamma(ramp_map.Sample(sampler_states[LINEAR], float2(diffuse_factor, v))).r;
	
	//float3 half_tone_factor = max(0.0, half_tone(200, ramp_factor, pin.texcoord.xy));
	float3 half_tone_factor = 1.0;
	float3 diffuse_radiance = blend_soft_light(Kd * ramp_factor * Lc, half_tone_factor);
	
	float3 H = normalize(L + V);
	float3 specular_radiance = (smoothness + 8) / (8 * PI) * pow(max(0, dot(N, H)), smoothness) * Ks * Lc;
#endif
	
	return diffuse_radiance + specular_radiance;
}

#endif // __RENDERING_EQUATION_HLSL__
// UNIT.32
#include "fullscreen_quad.hlsli"
#include "constant_buffer.hlsli"
#include "common.hlsli"

Texture2D texture_map : register(t0);

// Tone mapping operators
// https://www.shadertoy.com/view/lslGzl
static const float gamma = 2.3;
float3 linear_tonemapping(float3 color, float exposure)
{
    //float exposure = 1.;
    color = clamp(exposure * color, 0., 1.);
    color = pow(color, 1. / gamma);
    return color;
}
float3 simple_reinhard_tonemapping(float3 color, float exposure)
{
    //float exposure = 1.5;
    color *= exposure / (1. + color / exposure);
    color = pow(color, 1. / gamma);
    return color;
}
float3 lumabased_reinhard_tonemapping(float3 color)
{
    float luma = dot(color, float3(0.2126, 0.7152, 0.0722));
    float tonemapped_luma = luma / (1. + luma);
    color *= tonemapped_luma / luma;
    color = pow(color, 1. / gamma);
    return color;
}
float3 white_preserving_lumabased_reinhard_tonemapping(float3 color)
{
    float white = 3.0;
    float luma = dot(color, float3(0.2126, 0.7152, 0.0722));
    float tonemapped_luma = luma * (1. + luma / (white * white)) / (1. + luma);
    color *= tonemapped_luma / luma;
    color = pow(color, 1. / gamma);
    return color;
}
float3 rom_bin_da_house_tonemapping(float3 color)
{
    color = exp(-1.0 / (2.72 * color + 0.15));
    color = pow(color, 1. / gamma);
    return color;
}
float3 filmic_tonemapping(float3 color)
{
    color = max(0., color - 0.004);
    color = (color * (6.2 * color + .5)) / (color * (6.2 * color + 1.7) + 0.06);
    return color;
}
float3 uncharted2_tonemapping(float3 color, float exposure)
{
    color = max(0., color);
	
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    float W = 11.2;
    //float exposure = 2.;
    color *= exposure;
    color = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
    float white = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;
    color /= white;
    color = pow(color, 1. / gamma);
    return color;
}

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 sampled_color = texture_map.Sample(sampler_states[POINT], pin.texcoord);
    float3 fragment_color = sampled_color.rgb;
    float alpha = sampled_color.a;

#if 1
	// Apply tone mapping.
	//fragment_color = 1 - exp(-fragment_color * exposure);
	//fragment_color = filmic_tonemapping(fragment_color);
	fragment_color = uncharted2_tonemapping(fragment_color, exposure);
	//fragment_color = white_preserving_lumabased_reinhard_tonemapping(fragment_color);
   
#endif
    
#if 0
	// Apply gamma correction.
	fragment_color = pow(fragment_color, 1.0 / gamma);
#endif
    
	return float4(fragment_color * colorize, alpha);
}
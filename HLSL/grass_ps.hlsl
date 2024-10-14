// UNIT.99
#include "grass.hlsli"
#include "noise.hlsli"
#include "constant_buffer.hlsli"
#include "common.hlsli"
#include "rendering_equation.hlsli"

Texture2D texture_maps[4] : register(t0);

[earlydepthstencil]
float4 main(GS_OUT pin) : SV_TARGET
{
	float4 base_color = degamma(texture_maps[0].Sample(sampler_states[ANISOTROPIC], pin.texcoord));
	const float alpha = base_color.a;

	const float3 N = normalize(pin.world_normal.xyz);
	const float3 V = normalize(camera_position.xyz - pin.world_position.xyz);

	const float3 grass_withered_color = saturate(base_color.rgb + pin.color.rgb);
	float3 diffuse_color = lerp(grass_withered_color, grass_withered_color * 0.2, pin.texcoord.y);

	diffuse_color = lerp(
		0.75,
		diffuse_color,
		exp(-(1.2 - pin.texcoord.y) * snow_factor)
	);

	// Apply specular only to the tips of the grass blade.
#if 0
	const float specular_intensity = smoothstep(0.5, 0.0, pin.texcoord.y);
#else
	const float specular_intensity = exp(-pin.texcoord.y * 0.001);
#endif
	const float4 specular_color = grass_specular_color;
	const float shininess = 32.0;

	float3 radiance = 0.0;
	float3 diffuse = 0;
	float3 specular = 0;

	float3 L = 0;
	L = normalize(-directional_light_direction[0].xyz);
	diffuse = diffuse_color * max(0, dot(N, L) * 0.5 + 0.5);
	specular = pow(max(0, dot(N, normalize(V + L))), 32) * specular_intensity * specular_color.rgb * specular_color.w;
	radiance += (diffuse + specular) * directional_light_color[0].rgb * directional_light_color[0].w;

	// Calculate the irradiance from the omni-light.
	L = omni_light_position[0].xyz - pin.world_position.xyz;
	float d = length(L); // distance between light_position and surface_position
	L = normalize(L);
	/*
			Distance	Kc		Kl		Kq
			7			1		0.7		1.8
			13			1		0.35	0.44
			20			1		0.22	0.2
			32			1		0.14	0.07
			50			1		0.09	0.032
			65			1		0.07	0.017
			100			1		0.045	0.0075
			160			1		0.027	0.0028
			200			1		0.022	0.0019
			325			1		0.014	0.0007
			600			1		0.007	0.0002
			3250		1		0.0014	0.000007
	*/
		float Kc = 1.0; // attenuation_constant
		float Kl = 0.045; // attenuation_linear
		float Kq = 0.0075; // attenuation_quadratic
		float attenuation = saturate(1.0 / (Kc + Kl * d + Kq * (d * d)));
		diffuse = diffuse_color * max(0, dot(N, L) * 0.5 + 0.5);
		specular = pow(max(0, dot(N, normalize(V + L))), 32) * specular_intensity * specular_color.rgb * specular_color.w * 0.15/*ad hoc coefficient*/;

		radiance += (diffuse + specular) * omni_light_color[0].rgb * omni_light_color[0].w * attenuation * 0.2;

		float3 ambient = diffuse_color * 0.2;
		radiance += ambient;

		return float4(max(0, radiance), alpha);
}

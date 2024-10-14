#include "geometric_substance.hlsli"
#include "constant_buffer.hlsli"
#include "rendering_equation.hlsli"
#include "common.hlsli"

Texture2D texture_maps[4] : register(t0);
TextureCube environment_map : register(t4);

struct husk_particle
{
	float4 color;
	float3 position;
	float3 normal;
};
AppendStructuredBuffer<husk_particle> husk_particle_buffer : register(u1);

[earlydepthstencil]
void main(VS_OUT pin)
{
	float4 diffuse_color = degamma(texture_maps[0].Sample(sampler_states[ANISOTROPIC], pin.texcoord));

	float3 diffuse_reflection = diffuse_color.rgb * diffuse.rgb;
	const float alpha = diffuse_color.a * diffuse.w;

	const float3 specular_reflection = specular.rgb;
	const float shininess = specular.w;


	float3 N = normalize(pin.world_normal.xyz);
#if 1
	float3 T = normalize(pin.world_tangent.xyz);
	float sigma = pin.world_tangent.w;
	T = normalize(T - dot(N, T));
	float3 B = normalize(cross(N, T) * sigma);
	// Transform to world space from tangent space
	//                 |Tx Ty Tz|
	// normal = |x y z||Bx By Bz|
	//                 |Nx Ny Nz|
	float4 normal = texture_maps[1].Sample(sampler_states[LINEAR], pin.texcoord);
	normal = (normal * 2.0) - 1.0;
	N = normalize((normal.x * T) + (normal.y * B) + (normal.z * N));
#endif
	float3 V = normalize(camera_position.xyz - pin.world_position.xyz);
	float3 L = 0.0;
	float3 Lc = 0.0;

#if 0
	// Apply snow white
	diffuse_reflection = lerp(
		diffuse_reflection,
		1.0,
		saturate(
			max(0., dot(N, float3(0, 1, 0))) * snow_factor
		)
	);
#endif
	
	float3 radiance = 0.0;

	// Calculate the irradiance from the directional-light.
	L = normalize(-directional_light_direction[0].xyz);
	Lc = directional_light_color[0].rgb * directional_light_color[0].w;
	radiance += rendering_equation(L, V, N, Lc, diffuse_reflection, specular_reflection, shininess);

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
	float Kl = 0.7; // attenuation_linear
	float Kq = 1.8; // attenuation_quadratic
	float attenuation = saturate(1.0 / (Kc + Kl * d + Kq * (d * d)));
	Lc = attenuation * omni_light_color[0].rgb * omni_light_color[0].w;
	radiance += rendering_equation(L, V, N, Lc, diffuse_reflection, specular_reflection, shininess);

	// Add the emissive to the radiance.
	radiance += diffuse_reflection * emissive.rgb * emissive.w;
	
#if 0
	const float3 ambient_reflection = diffuse_reflection * ambient.rgb * ambient.w;
    radiance += ambient_reflection;
#endif

#if 0
	// Apply reflection processing.
	radiance += degamma(environment_map.Sample(sampler_states[ANISOTROPIC], reflect(-V, N))).rgb * reflection.rgb * directional_light_color[0].w;
#endif
	
#if 1
	// Apply rimlighting.
	float rim = 1 - saturate(dot(V, N));
	float3 rimlight_radiance = rimlight_color.rgb * rimlight_color.w * smoothstep(rimlight_factor, 1.0, rim) * diffuse_reflection;
	radiance += rimlight_radiance;
#endif	

	husk_particle p;
	p.color = float4(max(0, radiance), alpha) * pin.color;
	p.position = pin.world_position.xyz;
	p.normal = N.xyz;
	husk_particle_buffer.Append(p);
}
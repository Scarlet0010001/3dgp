// UNIT.32
#include "fullscreen_quad.hlsli"
#include "constant_buffer.hlsli"
#include "noise.hlsli"
#include "common.hlsli"

#define SCENE 0
#define SCENE_DEPTH 1
Texture2D texture_maps[2] : register(t0);
TextureCube environment_map : register(t4);
Texture2D distortion_texture : register(t5);
Texture2D shader_lamps_texture : register(t6);
Texture2D noise_map : register(t8);

float3 bokeh_effect(Texture2D color_map, float aspect/*H div W*/, float depth, float2 texcoord, float aperture, float focus, float max_blur)
{
	//Bokeh Effect
	//Max Blur: The maximum amount of blurring.Ranges from 0 to 1
	//Aperture: Bigger values create a shallower depth of field
	//Focus: Controls the focus of the effect
	//Aspect: Controls the blurring effect

	//float aspect = dimensions.y / dimensions.x;
	const float2 aspect_correct = float2(1.0, aspect);

	//float4 depth1 = depth_map.Sample(sampler_states[LINEAR], texcoord);
	const float factor = depth - focus;
	const float2 dofblur = clamp(factor * aperture, -max_blur, max_blur);
	const float2 dofblur9 = dofblur * 0.9;
	const float2 dofblur7 = dofblur * 0.7;
	const float2 dofblur4 = dofblur * 0.4;

	float4 color = 0;
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(0.0, 0.4) * aspect_correct) * dofblur);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(0.15, 0.37) * aspect_correct) * dofblur);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(0.29, 0.29) * aspect_correct) * dofblur);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(-0.37, 0.15) * aspect_correct) * dofblur);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(0.40, 0.0) * aspect_correct) * dofblur);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(0.37, -0.15) * aspect_correct) * dofblur);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(0.29, -0.29) * aspect_correct) * dofblur);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(-0.15, -0.37) * aspect_correct) * dofblur);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(0.0, -0.4) * aspect_correct) * dofblur);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(-0.15, 0.37) * aspect_correct) * dofblur);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(-0.29, 0.29) * aspect_correct) * dofblur);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(0.37, 0.15) * aspect_correct) * dofblur);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(-0.4, 0.0) * aspect_correct) * dofblur);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(-0.37, -0.15) * aspect_correct) * dofblur);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(-0.29, -0.29) * aspect_correct) * dofblur);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(0.15, -0.37) * aspect_correct) * dofblur);

	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(0.15, 0.37) * aspect_correct) * dofblur9);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(-0.37, 0.15) * aspect_correct) * dofblur9);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(0.37, -0.15) * aspect_correct) * dofblur9);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(-0.15, -0.37) * aspect_correct) * dofblur9);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(-0.15, 0.37) * aspect_correct) * dofblur9);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(0.37, 0.15) * aspect_correct) * dofblur9);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(-0.37, -0.15) * aspect_correct) * dofblur9);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(0.15, -0.37) * aspect_correct) * dofblur9);

	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(0.29, 0.29) * aspect_correct) * dofblur7);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(0.40, 0.0) * aspect_correct) * dofblur7);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(0.29, -0.29) * aspect_correct) * dofblur7);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(0.0, -0.4) * aspect_correct) * dofblur7);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(-0.29, 0.29) * aspect_correct) * dofblur7);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(-0.4, 0.0) * aspect_correct) * dofblur7);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(-0.29, -0.29) * aspect_correct) * dofblur7);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(0.0, 0.4) * aspect_correct) * dofblur7);

	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(0.29, 0.29) * aspect_correct) * dofblur4);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(0.4, 0.0) * aspect_correct) * dofblur4);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(0.29, -0.29) * aspect_correct) * dofblur4);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(0.0, -0.4) * aspect_correct) * dofblur4);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(-0.29, 0.29) * aspect_correct) * dofblur4);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(-0.4, 0.0) * aspect_correct) * dofblur4);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(-0.29, -0.29) * aspect_correct) * dofblur4);
	color += color_map.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord + (float2(0.0, 0.4) * aspect_correct) * dofblur4);

	return color.rgb / 41.0;
}
//https://developer.playcanvas.com/en/user-manual/graphics/posteffects/
float3 brightness_contrast(float3 fragment_color, float brightness, float contrast)
{
	//Brightness - Contrast Effect
	//The brightness - contrast effect allows you to modify the brightness and contrast of the rendered image.
	//Brightness: The brighness of the image.Ranges from - 1 to 1 (-1 is solid black, 0 no change, 1 solid white).
	//Contrast : The contrast of the image.Ranges from - 1 to 1 (-1 is solid gray, 0 no change, 1 maximum contrast).
	fragment_color += brightness;
	if (contrast > 0.0)
	{
		fragment_color = (fragment_color - 0.5) / (1.0 - contrast) + 0.5;
	}
	else if (contrast < 0.0)
	{
		fragment_color = (fragment_color - 0.5) * (1.0 + contrast) + 0.5;
	}
	return fragment_color;
}

float3 hue_saturation(float3 fragment_color, float hue, float saturation)
{
	//Hue - Saturation Effect
	//The hue - saturation effect allows you to modify the hue and saturation of the rendered image.
	//Hue: The hue of the image.Ranges from - 1 to 1 (-1 is 180 degrees in the negative direction, 0 no change, 1 is 180 degrees in the postitive direction).
	//Saturation : The saturation of the image.Ranges from - 1 to 1 (-1 is solid gray, 0 no change, 1 maximum saturation).
	float angle = hue * 3.14159265;
	float s = sin(angle), c = cos(angle);
	float3 weights = (float3(2.0 * c, -sqrt(3.0) * s - c, sqrt(3.0) * s - c) + 1.0) / 3.0;
	fragment_color = float3(dot(fragment_color, weights.xyz), dot(fragment_color, weights.zxy), dot(fragment_color, weights.yzx));
	float average = (fragment_color.r + fragment_color.g + fragment_color.b) / 3.0;
	if (saturation > 0.0)
	{
		fragment_color += (average - fragment_color) * (1.0 - 1.0 / (1.001 - saturation));
	}
	else
	{
		fragment_color += (average - fragment_color) * (-saturation);
	}
	return fragment_color;
}
float3 atmosphere(float3 fragment_color, float3 mist_color, float3 pixel_coord/*world space*/, float3 eye_coord/*world space*/)
{
	// Using 3d noise to represent the flow of mist.
	const float3 mist_flow_direction = float3(-1.0, -.2, -0.5);
	const float3 mist_flow_coord = pixel_coord.xyz + (mist_flow_direction * mist_flow_speed * time * wind_strength);
	const float flowing_density = lerp(mist_flow_density_lower_limit, 1.0, noise(fmod(mist_flow_coord * mist_flow_noise_scale_factor, 289)));

	float3 eye_to_pixel = pixel_coord - eye_coord;

	// z is the distance from the eye to the point
	float z = length(pixel_coord - eye_coord);
	z = smoothstep(0, mist_cutoff_distance, z) * z;

	// extinction and inscattering coefficients
	const float2 coefficients = mist_density * smoothstep(0.0, mist_height_falloff, height_mist_offset - pixel_coord.y) * flowing_density;

	// extinction and inscattering factors
	const float2 factors = exp(-z * coefficients);

	const float extinction = factors.x;
	const float inscattering = factors.y;
	fragment_color = fragment_color * extinction + mist_color * (1.0 - inscattering);

#if 1
	// Find the sum highlight and use it to blend the mist color
	float3 sun_position = -normalize(directional_light_direction[0].xyz) * distance_to_sun;
	float sun_highlight_factor = max(0, dot(normalize(eye_to_pixel), normalize(sun_position - eye_coord)));
	// Affects the area of influence of the sun's highlights.
	sun_highlight_factor = pow(sun_highlight_factor, sun_highlight_exponential_factor);

	const float near = 250.0; // The distance at which the effect begins to take effect.
	const float far = distance_to_sun; // The distance at which the effect reaches its maximum value.
	float3 sunhighlight_color = lerp(0, sun_highlight_intensity * (normalize(directional_light_color[0].rgb)), sun_highlight_factor * smoothstep(near, far, z));
	fragment_color += sunhighlight_color;
#endif

	return fragment_color;
}
// https://www.shadertoy.com/view/4sX3Rs
float3 lens_flare(float2 uv, float2 pos)
{
#if 1
	const float glory_light_intensity = 1.5;
	const float lens_flare_intensity = 3.0;

	float2 main = uv - pos;
	float2 uvd = uv * (length(uv));

	float ang = atan2(main.x, main.y);
	float dist = length(main); dist = pow(dist, .1);
#if 1
	float n = noise(float2(ang * 16.0, dist * 32.0));
#else
	float n = noise_map.Sample(sampler_states[LINEAR], float2(ang * 16.0, dist * 32.0)).x;
#endif

	// Glory light
	float f0 = 1.0 / (length(uv - pos) * 16.0 + 1.0);
#if 1
	f0 = f0 + f0 * (sin(noise(sin(ang * 2. + pos.x) * 4.0 - cos(ang * 3. + pos.y)) * 16.) * .1 + dist * .1 + .8);
#else
	f0 = f0 + f0 * (sin(noise_map.Sample(sampler_states[LINEAR], float2(sin(ang * 2. + pos.x) * 4.0 - cos(ang * 3. + pos.y), 0.0)).x * 16.) * .1 + dist * .1 + .8);
#endif

	// Lens flare only 
	float f1 = max(0.01 - pow(length(uv + 1.2 * pos), 1.9), .0) * 7.0;

	float f2 = max(1.0 / (1.0 + 32.0 * pow(length(uvd + 0.8 * pos), 2.0)), .0) * 0.25;
	float f22 = max(1.0 / (1.0 + 32.0 * pow(length(uvd + 0.85 * pos), 2.0)), .0) * 0.23;
	float f23 = max(1.0 / (1.0 + 32.0 * pow(length(uvd + 0.9 * pos), 2.0)), .0) * 0.21;

	float2 uvx = lerp(uv, uvd, -0.5);
	 
	float f4 = max(0.01 - pow(length(uvx + 0.4 * pos), 2.4), .0) * 6.0;
	float f42 = max(0.01 - pow(length(uvx + 0.45 * pos), 2.4), .0) * 5.0;
	float f43 = max(0.01 - pow(length(uvx + 0.5 * pos), 2.4), .0) * 3.0;

	uvx = lerp(uv, uvd, -.4);

	float f5 = max(0.01 - pow(length(uvx + 0.2 * pos), 5.5), .0) * 2.0;
	float f52 = max(0.01 - pow(length(uvx + 0.4 * pos), 5.5), .0) * 2.0;
	float f53 = max(0.01 - pow(length(uvx + 0.6 * pos), 5.5), .0) * 2.0;

	uvx = lerp(uv, uvd, -0.5);

	float f6 = max(0.01 - pow(length(uvx - 0.3 * pos), 1.6), .0) * 6.0;
	float f62 = max(0.01 - pow(length(uvx - 0.325 * pos), 1.6), .0) * 3.0;
	float f63 = max(0.01 - pow(length(uvx - 0.35 * pos), 1.6), .0) * 5.0;

	float3 c = 0;

	c.r += f2 + f4 + f5 + f6; c.g += f22 + f42 + f52 + f62; c.b += f23 + f43 + f53 + f63;
	c = max(0, c * 1.3 - (length(uvd) * .05));

	return f0 * glory_light_intensity + c * lens_flare_intensity;
#else
	// Lens flare only 
	float intensity = 1.5;
	float2 main = uv - pos;
	float2 uvd = uv * (length(uv));

	float dist = length(main); 
	dist = pow(dist, .1);

	float f1 = max(0.01/*max radius*/ - pow(length(uv + 1.2/*distance*/ * pos), 1.9/*boost*/), .0) * 7.0;

	float f2 = max(1.0 / (1.0 + 32.0 * pow(length(uvd + 0.8 * pos), 2.0)), .0) * 00.1;
	float f22 = max(1.0 / (1.0 + 32.0 * pow(length(uvd + 0.85 * pos), 2.0)), .0) * 00.08;
	float f23 = max(1.0 / (1.0 + 32.0 * pow(length(uvd + 0.9 * pos), 2.0)), .0) * 00.06;

	float2 uvx = lerp(uv, uvd, -0.5);

	float f4 = max(0.01 - pow(length(uvx + 0.4 * pos), 2.4), .0) * 6.0;
	float f42 = max(0.01 - pow(length(uvx + 0.45 * pos), 2.4), .0) * 5.0;
	float f43 = max(0.01 - pow(length(uvx + 0.5 * pos), 2.4), .0) * 3.0;

	uvx = lerp(uv, uvd, -.4);

	float f5 = max(0.01 - pow(length(uvx + 0.2 * pos), 5.5), .0) * 2.0;
	float f52 = max(0.01 - pow(length(uvx + 0.4 * pos), 5.5), .0) * 2.0;
	float f53 = max(0.01 - pow(length(uvx + 0.6 * pos), 5.5), .0) * 2.0;

	uvx = lerp(uv, uvd, -0.5);

	float f6 = max(0.01 - pow(length(uvx - 0.3 * pos), 1.6), .0) * 6.0;
	float f62 = max(0.01 - pow(length(uvx - 0.325 * pos), 1.6), .0) * 3.0;
	float f63 = max(0.01 - pow(length(uvx - 0.35 * pos), 1.6), .0) * 5.0;

	float3 c = .0;

	c.r += f2 + f4 + f5 + f6; 
	c.g += f22 + f42 + f52 + f62; 
	c.b += f23 + f43 + f53 + f63;
	c = c * 1.3 - (length(uvd) * .05);

	return c * intensity;
#endif 
}

float3 cc(float3 color, float factor, float factor2) // color modifier
{
	float w = color.x + color.y + color.z;
	return lerp(color, w * factor, w * factor2);
}
float4 main(VS_OUT pin) : SV_TARGET
{
	uint2 dimensions;
	uint mip_level = 0, number_of_samples;
	texture_maps[SCENE].GetDimensions(mip_level, dimensions.x, dimensions.y, number_of_samples);
	const float aspect = (float)dimensions.y / dimensions.x;

	float4 sampled_color = texture_maps[SCENE].Sample(sampler_states[POINT], pin.texcoord);
	float3 fragment_color = sampled_color.rgb;
	float alpha = sampled_color.a;

	float scene_depth = texture_maps[SCENE_DEPTH].Sample(sampler_states[POINT], pin.texcoord).x;

	float4 ndc_position;
	//texture space to ndc
	ndc_position.x = pin.texcoord.x * +2 - 1;
	ndc_position.y = pin.texcoord.y * -2 + 1;
	ndc_position.z = scene_depth;
	ndc_position.w = 1;

	//ndc to world space
	float4 world_position = mul(ndc_position, inverse_view_projection);
	world_position = world_position / world_position.w;

	// Apply bokeh effect.
#if 1
	fragment_color = bokeh_effect(texture_maps[SCENE], aspect, scene_depth, pin.texcoord, bokeh_aperture, bokeh_focus, 0.1);
#endif

	// Adapt atmosphere effects.
	fragment_color = atmosphere(fragment_color, mist_color.rgb * directional_light_color[0].rgb * directional_light_color[0].w, world_position.xyz, camera_position.xyz);

	// Lens flare
	float4 ndc_sun_position = mul(float4(-normalize(directional_light_direction[0].xyz) * distance_to_sun, 1), view_projection);
	ndc_sun_position /= ndc_sun_position.w;
	if (saturate(ndc_sun_position.z) == ndc_sun_position.z)
	{
		float4 occluder;
		occluder.xy = ndc_sun_position.xy;
		occluder.z = texture_maps[SCENE_DEPTH].Sample(sampler_states[LINEAR_BORDER_BLACK], float2(ndc_sun_position.x * 0.5 + 0.5, 0.5 - ndc_sun_position.y * 0.5)).x;
		occluder = mul(float4(occluder.xyz, 1), inverse_projection);
		occluder /= occluder.w;
		float occluded_factor = step(250.0, occluder.z);

		//const float2 aspect_correct = float2(1.0, aspect);
		const float2 aspect_correct = float2(1.0 / aspect, 1.0);

		float sun_highlight_factor = max(0, dot(normalize(mul(world_position - camera_position, view)).xyz, float3(0, 0, 1)));
		float3 lens_flare_color = float3(1.4, 1.2, 1.0) * lens_flare(ndc_position.xy * aspect_correct, ndc_sun_position.xy * aspect_correct);
		lens_flare_color -= noise(ndc_position.xy * 256) * .015;
		lens_flare_color = cc(lens_flare_color, .5, .1);
		fragment_color += max(0.0, lens_flare_color) * occluded_factor * directional_light_color[0].rgb * 0.5/* * directional_light_color[0].w*/;
	}

	// Projection texture mapping.
	float4 projection_texture_position = mul(world_position, projection_texture_transforms);
	projection_texture_position /= projection_texture_position.w;
	projection_texture_position.x = projection_texture_position.x * 0.5 + 0.5;
	projection_texture_position.y = -projection_texture_position.y * 0.5 + 0.5;
	if (saturate(projection_texture_position.z) == projection_texture_position.z)
	{
		float4 projection_texture_color = shader_lamps_texture.Sample(sampler_states[LINEAR_BORDER_BLACK], projection_texture_position.xy);
		fragment_color += projection_texture_color.rgb * projection_texture_color.a * projection_texture_intensity.rgb * projection_texture_intensity.a;
	}

	// Adjusts the hue, saturation, brightness and contrast.
	fragment_color = hue_saturation(fragment_color, hue, saturation);
	fragment_color = brightness_contrast(fragment_color, brightness, contrast);

	return float4(fragment_color, alpha);
}

// UNIT.99
#include "grass.hlsli"
#include "constant_buffer.hlsli"
#include "noise.hlsli"
#include "common.hlsli"

Texture2D distortion_texture : register(t5);

#define PI 3.141592653
float4x4 angle_axis(float angle, float3 axis)
{
    float c, s;
    sincos(angle, s, c);

    float t = 1 - c;
    float x = axis.x;
    float y = axis.y;

    float z = axis.z;
    return float4x4(
		t * x * x + c, t * x * y - s * z, t * x * z + s * y, 0,
		t * x * y + s * z, t * y * y + c, t * y * z - s * x, 0,
		t * x * z - s * y, t * y * z + s * x, t * z * z + c, 0,
		0, 0, 0, 1
		);
}

#define BLADE_SEGMENTS 5
[maxvertexcount(BLADE_SEGMENTS * 2 + 1)]
void main(triangle DS_OUTPUT input[3], inout TriangleStream<GS_OUT> output)
{
    const float perlin_noise = noise(input[0].world_position.xyz * perlin_noise_distribution_factor);
    const float grass_blade_height = grass_height_factor + (perlin_noise * 2.0 - 1.0) * grass_height_variance;
    const float grass_blade_width = grass_width_factor;
    const float4 withered_color = float4(perlin_noise * grass_withered_factor, 0.0, 0.0, 1.0);

#if 0
    float4 midpoint_position = (input[0].world_position + input[1].world_position + input[2].world_position) / 3;
#else
    const float s = random(input[1].world_position.xy);
    const float t = random(input[2].world_position.yz);
    float4 midpoint_position = lerp(input[0].world_position, lerp(input[1].world_position, input[2].world_position, s), t);
	midpoint_position.w = 1;
#endif

    float4 midpoint_normal = normalize((input[0].world_normal + input[1].world_normal + input[2].world_normal) / 3);
    float4 midpoint_tangent = normalize((input[0].world_tangent + input[1].world_tangent + input[2].world_tangent) / 3);

    const float random_facing = random(input[0].world_position.zx);
    const row_major float4x4 R = angle_axis(random_facing * PI, midpoint_normal.xyz);
    midpoint_tangent = mul(midpoint_tangent, R);

#if 1
    const float2 distortion_texcoord = midpoint_position.xz + wind_frequency * time;
    const float4 distortion = distortion_texture.SampleLevel(sampler_states[ANISOTROPIC], distortion_texcoord * 0.001, 0) * 2 - 1;
    const float wind_avatar_bending_angle = distortion.y * PI * 0.5 * wind_strength;
    const float3 wind_bending_axis = normalize(float3(distortion.x, 0, distortion.z));
    const float4x4 W = angle_axis(wind_avatar_bending_angle, wind_bending_axis);

    const float4 avatar_offset = avatar_position - midpoint_position;
    const float avatar_distance = length(avatar_offset);
    const float impact_radius = 5;
    const float avatar_bending_angle = smoothstep(impact_radius, 0, avatar_distance);
    const float3 avatar_bending_axis = normalize(cross(midpoint_normal.xyz, normalize(avatar_offset).xyz));
    const row_major float4x4 A = angle_axis(avatar_bending_angle * PI * 0.2, avatar_bending_axis);
    const row_major float4x4 B = mul(W, A);
#else
	const row_major float4x4 B = float4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
#endif

    const float random_curvature = random(input[0].world_position.xy * 0.01);
    float curvature = PI * 0.5 * (random_curvature * 2.0 - 1.0) * grass_curvature;
    const row_major float4x4 C = angle_axis(curvature / BLADE_SEGMENTS, midpoint_tangent.xyz);
    float4 segment_normal = midpoint_normal;

    GS_OUT element;
    for (int i = 0; i < BLADE_SEGMENTS; i++)
    {
        float t = i / (float) BLADE_SEGMENTS;
        float segment_height = grass_blade_height * t;
        float segment_width = grass_blade_width * (1 - t);
#if 1
        element.world_normal = segment_normal;
#else
		element.world_normal = float4(normalize(cross(segment_normal.xyz, midpoint_tangent.xyz)), 0.0);
#endif
        element.color = withered_color;

        element.world_position = midpoint_position + segment_normal * segment_height + midpoint_tangent * segment_width;
        element.world_position = mul(element.world_position - midpoint_position, B) + midpoint_position;
        element.position = mul(element.world_position, view_projection);
        element.texcoord = float2(0, 1 - t);
        output.Append(element);

        element.world_position = midpoint_position + segment_normal * segment_height - midpoint_tangent * segment_width;
        element.world_position = mul(element.world_position - midpoint_position, B) + midpoint_position;
        element.position = mul(element.world_position, view_projection);
        element.texcoord = float2(1, 1 - t);
        output.Append(element);

        segment_normal = mul(segment_normal, C);
    }
    element.world_position = midpoint_position + segment_normal * grass_blade_height;
    element.world_position = mul(element.world_position - midpoint_position, B) + midpoint_position;
    element.position = mul(element.world_position, view_projection);
    element.world_normal = segment_normal;
    element.color = withered_color;
    element.texcoord = float2(0.5, 0);
    output.Append(element);

    output.RestartStrip();
}

#include "sprite.hlsli"

//#include "scene_constant_buffer.hlsli"
cbuffer SCENE_CONSTANT_BUFFER : register(b1)
{
    float4 options; //	xy : マウスの座標値, z : タイマー, w : フラグ
    float4 z_buffer_parameteres; // 非線形深度から線形深度へ変換するためのパラメーター
    float4 camera_position;
    float4 camera_direction;
    float4 camera_clip_distance;
    float4 viewport_size; //  xy : ビューポートサイズ, zw : 逆ビューポートサイズ
    row_major float4x4 view_transform;
    row_major float4x4 projection_transform;
    row_major float4x4 view_projection_transform;
    row_major float4x4 inverse_view_transform;
    row_major float4x4 inverse_projection_transform;
    row_major float4x4 inverse_view_projection_transform;

    row_major float4x4 previous_view_projection_transform;
};

cbuffer RADIAL_BLUR_CONSTANT_BUFFER : register(b2)
{
    float blur_radius;
    int blur_sampling_count;
    float2 blur_center;

    float blur_mask_radius;
    float3 blur_dummy;

};

Texture2D scene_map : register(t0);
SamplerState linear_sampler_state : register(s0);

float4 main(VS_OUT pin) : SV_TARGET
{
    float2 scene_map_size;
    scene_map.GetDimensions(scene_map_size.x, scene_map_size.y);

    float4 color = scene_map.Sample(linear_sampler_state, pin.texcoord);
    float4 result_color = color;

    float2 blur_vector = (blur_center - pin.texcoord);
    blur_vector *= (blur_radius / scene_map_size.xy) / blur_sampling_count;
    for (int index = 1; index < blur_sampling_count; ++index)
    {
        result_color += scene_map.Sample(linear_sampler_state, pin.texcoord + blur_vector * index);
    }

    //return result_color / blur_sampling_count;

    //  指定の範囲内は適応量を変える
    float mask_radius = blur_mask_radius / min(scene_map_size.x, scene_map_size.y);
    float mask_value = saturate(length(pin.texcoord - blur_center) / mask_radius);
    return lerp(color, result_color / blur_sampling_count, mask_value);
}

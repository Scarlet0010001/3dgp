#include "fullscreen_quad.hlsli"
#include "math_utils.hlsli"

cbuffer GLITCH_SHADER_CONSTANT_BUFFER : register(b1)
{
    float iTime; //経過時間
    float density; //密度
    float shift; //ずらす幅
    float rand_float; //ランダム性
    float2 x_shift; //xの横をどの程度ずらすのか
    float2 y_shift; //yの横をどの程度ずらすのか
    float x_shifting; //色収差のxの位置をずらす
    float y_shifting; //色収差のyの位置をずらす    
    float extension; //uvの拡張
    float uv_slider; //左上に流す
    float brightness; //明るさ
    float glitch_mask_radius; //マスク半径
    int glitch_sampling_count; //回数
    float2 center; //中心
};

Texture2D image_map : register(t0);
Texture2D noise[2] : register(t1);
SamplerState smpl : register(s5);

float4 main(VS_OUT pin) : SV_TARGET
{
    //時間制御
    float t = iTime;

    float2 scene_map_size;

    //画像サイズ取得
    image_map.GetDimensions(scene_map_size.x, scene_map_size.y);

    float2 uv = pin.texcoord.xy;

    float4 source_color = image_map.Sample(smpl, uv);

    // t * 密度
    float s = noise[0].Sample(smpl, float2(t * density, 0.5)).r;

    // 横にどのくらいズレるか
    uv = interlace(image_map, uv, s * shift);

    float r = noise[1].Sample(smpl, float2(t, 0.0)).x;

    //ランダム性
    uv = rnd(noise[0], smpl, uv, s * rand_float);

    //色収差
    float3 color = colorSplit(image_map,smpl,uv, float2(s * x_shifting, y_shifting)); //横分身、縦分身

    //uvの拡張,uvの左上に流す,明るさ
    color = lerp(color, noise[0].Sample(smpl, extension + t * uv_slider).rgb, brightness);

    float4 result_color = float4(color, 1.0);

    //  指定の範囲内は適応量を変える
    float mask_value = saturate(length(pin.texcoord - center) / glitch_mask_radius);

    return lerp(source_color, result_color, mask_value);
}
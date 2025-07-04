#ifndef __MATH_UTILS
#define __MATH_UTILS

#define pi 3.1415926

//指定されたUV座標に基づいた疑似乱数を生成
float random(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453123);
}

//色ずれ（RGBシフト）を行う関数
float3 colorSplit(Texture2D tex, SamplerState sample, float2 uv, float2 s)
{
    float3 color;
    color.r = tex.Sample(sample, uv - s).r;
    color.g = tex.Sample(sample, uv).g;
    color.b = tex.Sample(sample, uv + s).b;
    return color;
}

//インターレース効果をUVに加える
float2 interlace(Texture2D tex, float2 uv, float s)
{
    float2 scene_map_size;
    tex.GetDimensions(scene_map_size.x, scene_map_size.y);
    uv.x += s * (4.0 * frac((uv.y * scene_map_size.y) / 2.0) - 1.0);
    return uv;
}

//水平方向にスキャンラインのような歪みを加える
float2 fault(float2 uv, float s)
{
    float v = pow(0.5 - 0.5 * cos(2.0 * pi * uv.y), 100.0) * sin(2.0 * pi * uv.y);
    uv.x += v * s;
    return uv;
}

//画像の輝度に応じてUVをランダムにずらす
float2 rnd(Texture2D image, SamplerState sample, float2 uv, float s)
{
    uv.x += s * (2.0 * image.Sample(sample, uv * 0.05).x - 1.0);
    return uv;
}


#endif // __MATH_UTILS
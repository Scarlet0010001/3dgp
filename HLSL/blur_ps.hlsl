#include "fullscreen_quad.hlsli"
#include "constant_buffer.hlsli"

// サンプラーステートのタイプを定義
#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
#define CLAMP 3

// サンプラーステートとテクスチャを登録
SamplerState sampler_states[4] : register(s0);
Texture2D texture_maps[4] : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
    // ミップレベル、幅、高さ、レベル数を取得
    int mip_level = 0, width, height, number_of_levels;
    texture_maps[1].GetDimensions(mip_level, width, height, number_of_levels);
    
    // ベーステクスチャからカラーとアルファをサンプリング
    float4 color = texture_maps[0].Sample(sampler_states[ANISOTROPIC], pin.texcoord);
    float alpha = color.a;

    // ブラー処理用変数
    float3 blur_color = 0;
    float gaussian_kernel_total = 0;

    // ガウシアンブラーフィルタのサイズ
    const int gaussian_half_kernel_size = 3;

    // ガウシアンブラーフィルタのループ
    [unroll]
    for (int x = -gaussian_half_kernel_size; x <= +gaussian_half_kernel_size; x += 1)
    {
        [unroll]
        for (int y = -gaussian_half_kernel_size; y <= +gaussian_half_kernel_size; y += 1)
        {
            // ガウシアンカーネルを計算
            float gaussian_kernel = exp(-(x * x + y * y) / (2.0 * gaussian_sigma * gaussian_sigma)) /
                (2 * 3.14159265358979 * gaussian_sigma * gaussian_sigma);
            // ブラーカラーを更新
            blur_color += texture_maps[1].Sample(sampler_states[CLAMP], pin.texcoord +
                float2(x * 1.0 / width, y * 1.0 / height)).rgb * gaussian_kernel;
            // カーネル合計を更新
            gaussian_kernel_total += gaussian_kernel;
        }
        // ガウシアンブラーカラーを正規化
        blur_color /= gaussian_kernel_total;

        // ベースカラーにブラーカラーとブルームの強度を加算
        color.rgb += blur_color * bloom_intensity;
        
        // トーンマッピング
#if 1
        // Tone mapping : HDR -> SDR(トーンマッピングを施す)
        //値が大きいほど、画像が明るく見え、値が小さいほど暗く見える
        //const float exposure = 1.2;
        color.rgb = 1 - exp(-color.rgb * exposure);
#endif
#if 1
        // ガンマ補正
        const float GAMMA = 2.2;
        color.rgb = pow(color.rgb, 1.0 / GAMMA);
#endif
        // 最終的なカラーを返す
        return float4(color.rgb, alpha);
    }
}

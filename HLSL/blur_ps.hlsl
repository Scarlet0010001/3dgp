#include "fullscreen_quad.hlsli"
#include "constant_buffer.hlsli"

// �T���v���[�X�e�[�g�̃^�C�v���`
#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
#define CLAMP 3

// �T���v���[�X�e�[�g�ƃe�N�X�`����o�^
SamplerState sampler_states[4] : register(s0);
Texture2D texture_maps[4] : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
    // �~�b�v���x���A���A�����A���x�������擾
    int mip_level = 0, width, height, number_of_levels;
    texture_maps[1].GetDimensions(mip_level, width, height, number_of_levels);
    
    // �x�[�X�e�N�X�`������J���[�ƃA���t�@���T���v�����O
    float4 color = texture_maps[0].Sample(sampler_states[ANISOTROPIC], pin.texcoord);
    float alpha = color.a;

    // �u���[�����p�ϐ�
    float3 blur_color = 0;
    float gaussian_kernel_total = 0;

    // �K�E�V�A���u���[�t�B���^�̃T�C�Y
    const int gaussian_half_kernel_size = 3;

    // �K�E�V�A���u���[�t�B���^�̃��[�v
    [unroll]
    for (int x = -gaussian_half_kernel_size; x <= +gaussian_half_kernel_size; x += 1)
    {
        [unroll]
        for (int y = -gaussian_half_kernel_size; y <= +gaussian_half_kernel_size; y += 1)
        {
            // �K�E�V�A���J�[�l�����v�Z
            float gaussian_kernel = exp(-(x * x + y * y) / (2.0 * gaussian_sigma * gaussian_sigma)) /
                (2 * 3.14159265358979 * gaussian_sigma * gaussian_sigma);
            // �u���[�J���[���X�V
            blur_color += texture_maps[1].Sample(sampler_states[CLAMP], pin.texcoord +
                float2(x * 1.0 / width, y * 1.0 / height)).rgb * gaussian_kernel;
            // �J�[�l�����v���X�V
            gaussian_kernel_total += gaussian_kernel;
        }
        // �K�E�V�A���u���[�J���[�𐳋K��
        blur_color /= gaussian_kernel_total;

        // �x�[�X�J���[�Ƀu���[�J���[�ƃu���[���̋��x�����Z
        color.rgb += blur_color * bloom_intensity;
        
        // �g�[���}�b�s���O
#if 1
        // Tone mapping : HDR -> SDR(�g�[���}�b�s���O���{��)
        //�l���傫���قǁA�摜�����邭�����A�l���������قǈÂ�������
        //const float exposure = 1.2;
        color.rgb = 1 - exp(-color.rgb * exposure);
#endif
#if 1
        // �K���}�␳
        const float GAMMA = 2.2;
        color.rgb = pow(color.rgb, 1.0 / GAMMA);
#endif
        // �ŏI�I�ȃJ���[��Ԃ�
        return float4(color.rgb, alpha);
    }
}

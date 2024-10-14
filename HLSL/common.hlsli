#ifndef __COMMON_HLSL__
#define __COMMON_HLSL__

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
#define LINEAR_BORDER_BLACK 3
#define LINEAR_BORDER_WHITE 4
#define LINEAR_MIRROR 5
SamplerState sampler_states[6] : register(s0);
SamplerComparisonState comparison_sampler_state : register(s6);

#endif // __COMMON_HLSL__
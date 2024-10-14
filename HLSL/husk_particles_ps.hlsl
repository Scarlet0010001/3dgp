#include "husk_particles.hlsli"
#include "common.hlsli"

Texture2D texture_map : register(t0);

[earlydepthstencil]
float4 main(GS_OUT pin) : SV_TARGET
{
	return pin.color;
}
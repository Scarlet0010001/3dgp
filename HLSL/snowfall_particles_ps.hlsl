#include "snowfall_particles.hlsli"
#include "constant_buffer.hlsli"

[earlydepthstencil]
float4 main(GS_OUT pin) : SV_TARGET
{
	return float4(pin.color.rgb/* * directional_light_color[0].rgb */* directional_light_color[0].w, pin.color.a);
}

// UNIT.99
#include "grass.hlsli"
#include "constant_buffer.hlsli"

VS_OUT main(VS_IN vin)
{
	VS_OUT vout;
	vout.world_position = mul(vin.position, world);
	vin.normal.w = 0;
	vout.world_normal = normalize(mul(vin.normal, world));
	float sigma = vin.tangent.w;
	vin.tangent.w = 0;
	vout.world_tangent = normalize(mul(vin.tangent, world));
	vout.world_tangent.w = sigma;
	return vout;
}

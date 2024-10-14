// UNIT.99
#include "geometric_substance.hlsli"
#include "constant_buffer.hlsli"

VS_OUT_CSM main(float4 position : POSITION, uint instance_id : SV_InstanceID)
{
	VS_OUT_CSM vout;

	vout.position = mul(float4(position.xyz, 1.0f), mul(world, csm_data.view_projection_matrices[instance_id]));
	vout.slice = instance_id;

	return vout;

}

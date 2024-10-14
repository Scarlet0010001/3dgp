// UNIT.99
#include "geometric_substance.hlsli"
#include "constant_buffer.hlsli"

struct VS_IN
{
	float4 position : POSITION;
	float4 normal : NORMAL;
	float4 tangent : TANGENT;
	float2 texcoord : TEXCOORD;
	float4 bone_weights : WEIGHTS;
	uint4 bone_indices : BONES;
};

VS_OUT_CSM main(VS_IN vin, uint instance_id : SV_INSTANCEID)
{
	VS_OUT_CSM vout;

	float4 blended_position = { 0, 0, 0, 1 };
	for (int bone_index = 0; bone_index < 4; ++bone_index)
	{
		blended_position += vin.bone_weights[bone_index] * mul(vin.position, bone_transforms[vin.bone_indices[bone_index]]);
	}
	//vout.position = mul(mul(float4(blended_position.xyz, 1.0f), mul(world, view_projection)), csm_data.view_projection_matrices[instance_id]);
	vout.position = mul(float4(blended_position.xyz, 1.0f), mul(world, csm_data.view_projection_matrices[instance_id]));
	vout.slice = instance_id;

	return vout;
}

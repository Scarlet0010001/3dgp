#include "grass.hlsli"
#include "constant_buffer.hlsli"

struct HS_CONSTANT_DATA_OUTPUT
{
    float tess_factor[3] : SV_TessFactor;
    float inside_tess_factor : SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 3

HS_CONSTANT_DATA_OUTPUT patch_constant_function(InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> input_patch, uint patch_id : SV_PrimitiveID)
{
    HS_CONSTANT_DATA_OUTPUT output;
#define ENABLE_LOD
#ifdef ENABLE_LOD
    float3 midpoint = (input_patch[0].world_position.xyz + input_patch[1].world_position.xyz + input_patch[2].world_position.xyz) / 3.0;
    const float distance = length(avatar_position.xyz - midpoint);
    const float subdivision_factor = saturate((tesselation_max_distance - distance) / tesselation_max_distance);
    const float subdivision = tesselation_max_subdivision * subdivision_factor + 1.0;
#else
    const float subdivision = tesselation_max_division;
#endif
    output.tess_factor[0] = subdivision;
    output.tess_factor[1] = subdivision;
    output.tess_factor[2] = subdivision;
    output.inside_tess_factor = subdivision;
    return output;
}

[domain("tri")]
[partitioning("integer")] // integer, fractional_odd, fractional_even, pow2
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("patch_constant_function")]
HS_CONTROL_POINT_OUTPUT main(
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> input_patch,
	uint output_control_point_id : SV_OutputControlPointID,
	uint primitive_id : SV_PrimitiveID)
{
    HS_CONTROL_POINT_OUTPUT output;
    output = input_patch[output_control_point_id];
    return output;
}

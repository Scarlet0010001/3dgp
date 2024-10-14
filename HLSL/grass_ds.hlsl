#include "grass.hlsli"

struct HS_CONSTANT_DATA_OUTPUT
{
	float tess_factor[3] : SV_TessFactor;
	float inside_tess_factor : SV_InsideTessFactor;
};

#define CONTROL_POINT_COUNT 3

[domain("tri")]
DS_OUTPUT main(HS_CONSTANT_DATA_OUTPUT input, float3 domain : SV_DomainLocation, const OutputPatch<HS_CONTROL_POINT_OUTPUT, CONTROL_POINT_COUNT> patch)
{
	DS_OUTPUT output;

	output.world_position = patch[0].world_position * domain.x + patch[1].world_position * domain.y + patch[2].world_position * domain.z;
	output.world_position.w = 1;

	output.world_normal = patch[0].world_normal * domain.x + patch[1].world_normal * domain.y + patch[2].world_normal * domain.z;
	output.world_normal.w = 0;

	output.world_tangent = patch[0].world_tangent * domain.x + patch[1].world_tangent * domain.y + patch[2].world_tangent * domain.z;
	output.world_tangent.w = 0;

	return output;
}

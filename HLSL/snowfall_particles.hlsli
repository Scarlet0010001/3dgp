struct VS_OUT
{
	uint vertex_id : VERTEXID;
};
struct GS_OUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
};
struct particle
{
	float3 position;
	float3 velocity;
};

cbuffer PARTICLE_CONSTANTS : register(b9)
{
	float4 current_eye_position;
	float4 previous_eye_position;

	// Radius of outermost orbit 
	float outermost_radius;
	// Height of snowfall area
	float snowfall_area_height;

	float particle_size;
	uint particle_count;
};


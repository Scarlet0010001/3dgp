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

struct husk_particle
{
	float4 color;
	float3 position;
	float3 normal;
};
struct particle
{
	float4 color;
	float3 position;
	float3 velocity;
	float age;
	int state;
};

cbuffer PARTICLE_CONSTANTS : register(b9)
{
	float4 emitter_location;
	float4 target_location;
	uint particle_count;
	float particle_size;
	float streak_factor;
	float threshold_level;
};
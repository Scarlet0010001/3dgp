// UNIT.99
struct VS_IN
{
	float4 position : POSITION;
	float4 normal : NORMAL;
	float4 tangent : TANGENT;
};

struct VS_OUT
{
	float4 world_position : POSITION;
	float4 world_normal : NORMAL;
	float4 world_tangent : TANGENT;
};
typedef VS_OUT VS_CONTROL_POINT_OUTPUT;
typedef VS_OUT HS_CONTROL_POINT_OUTPUT;
typedef VS_OUT DS_OUTPUT;
struct GS_OUT
{
	float4 position : SV_POSITION;
	float4 world_position : POSITION;
	float4 world_normal : NORMAL;
	float2 texcoord : TEXCOORD;
	float4 color : COLOR;
};

cbuffer OBJECT_CONSTANTS : register(b0)
{
	row_major float4x4 world;
};
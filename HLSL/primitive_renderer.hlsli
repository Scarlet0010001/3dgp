struct VS_IN
{
	float4 position : POSITION;
	float4 color : COLOR;
};

struct VS_OUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

cbuffer PRIMITIVE_CONSTANT_BUFFER : register(b0)
{
	row_major float4x4	viewProjection;
};

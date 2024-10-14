#include "debug_renderer.hlsli"
#include "constant_buffer.hlsli"

VS_OUT main(float4 position : POSITION)
{
    VS_OUT vout;
    vout.position = mul(position, mul(world, view_projection));
    vout.color = material_color;

    return vout;
}

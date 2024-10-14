#include "husk_particles.hlsli"
#include "constant_buffer.hlsli"

RWStructuredBuffer<husk_particle> husk_particle_buffer : register(u0);
RWStructuredBuffer<particle> particle_buffer : register(u1);
RWStructuredBuffer<uint> completed_particle_buffer : register(u2);

[numthreads(256, 1, 1)]
void main(uint3 dtid : SV_DISPATCHTHREADID)
{
	particle_buffer[dtid.x].position = husk_particle_buffer[dtid.x].position;
	particle_buffer[dtid.x].color = husk_particle_buffer[dtid.x].color;
	particle_buffer[dtid.x].velocity = 0;
	particle_buffer[dtid.x].state = 0;
	particle_buffer[dtid.x].age = 0;
}
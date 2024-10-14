#include "husk_particles.hlsli"
#include "constant_buffer.hlsli"
#include "noise.hlsli"

RWStructuredBuffer<husk_particle> husk_particle_buffer : register(u0);
RWStructuredBuffer<particle> updated_particle_buffer : register(u1);
RWStructuredBuffer<uint> completed_particle_buffer : register(u2);

[numthreads(256, 1, 1)]
void main(uint3 dtid : SV_DISPATCHTHREADID)
{
	uint id = dtid.x;
	if (id <= particle_count)
	{
		particle p = updated_particle_buffer[id];

		switch (p.state)
		{
		case 0:
			//p = husk_particle_buffer[id];
			p.color.a = 0.8;
			p.state = 1;
			p.age = 0;
			completed_particle_buffer.IncrementCounter();
			break;
		case 1:
			p.velocity += husk_particle_buffer[id].normal * 1.0 * delta_time;
			p.position += p.velocity * delta_time;
			if (p.age > 2.0)
			{
				p.state = 2;
				completed_particle_buffer.IncrementCounter();
			}
			break;
		case 2:
			float3 v = normalize(target_location.xyz - emitter_location.xyz);
			float3 z = target_location.xyz - p.position;
			float3 x = normalize(dot(v, z) * v - z);
			z = normalize(z);
			float3 y = normalize(cross(z, x));
			p.velocity = normalize(v * 5.0 + y * 2.0 - x * 0.2) * 20;
			p.position += p.velocity * delta_time;
			if (dot(v, z) < 0.0)
			{
				p.color.rgb = husk_particle_buffer[id].color.xyz * 2;
				//p.color.a = 0.5;
				p.age = 0;
				p.state = 3;
				completed_particle_buffer.IncrementCounter();
			}
			break;
		case 3:
			const float force = 50.0 * snoise(p.position * 0.054);
			float3 acceleration = normalize(target_location.xyz - p.position) * force - p.velocity * 0.02;
			p.velocity += acceleration * delta_time;
			p.position += p.velocity * delta_time;
			if (p.age > 4.0)
			{
				p.color = husk_particle_buffer[id].color;
				//p.color.a = 1.0;
				p.age = 0;
				p.state = 4;
				completed_particle_buffer.IncrementCounter();
			}
			break;
		case 4:
			float3 d = husk_particle_buffer[id].position - p.position;
			p.velocity = normalize(d) * lerp(1.0, 5.0, length(d));
			p.position += p.velocity * delta_time;
			if (length(husk_particle_buffer[id].position - p.position) < 0.01)
			{
				p.position = husk_particle_buffer[id].position;
				p.velocity = 0;
				p.age = 0;
				p.state = 5;
				completed_particle_buffer.IncrementCounter();
			}
			break;
		case 5:
			//p.color.a = 1.0f;
			break;
		default:
			break;
		}

		p.age += delta_time;
		updated_particle_buffer[id] = p;
	}
}

#pragma once
#include <DirectXMath.h>

struct Sphere
{
	DirectX::XMFLOAT3 center;
	float radius;
};

struct Cylinder
{
	DirectX::XMFLOAT3 center;
	float radius;
	float height;
};

struct Cube
{
	DirectX::XMFLOAT3 center;
	float radius;
};

struct Capsule
{
	DirectX::XMFLOAT3 start;
	DirectX::XMFLOAT3 end;
	float radius;


};

struct Ring
{
	DirectX::XMFLOAT3 center;
	float width;
	float height;
	float radius;


};


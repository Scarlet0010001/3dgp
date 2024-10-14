#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl.h>
#include <vector>
#include <memory>
#include "constant.h"

class DebugRenderer
{
public:
	DebugRenderer(ID3D11Device* device);
	~DebugRenderer() {}
	
public:
	// �`����s
	void RenderAlFigures(ID3D11DeviceContext* context);
	// ���쐬
	void CreateSphere(const DirectX::XMFLOAT3& center, float radius, const DirectX::XMFLOAT4& color);
	// �~���쐬
	void CreateCylinder(const DirectX::XMFLOAT3& position, float radius, float height, const DirectX::XMFLOAT4& color);
	// �����̍쐬
	void CreateCuboid(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& radius, const DirectX::XMFLOAT4& color);
	// �J�v�Z���쐬
	void CreateCapsule(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, float radius, const DirectX::XMFLOAT4& color);
private:
	// �����b�V���쐬
	void CreateSphereMesh(ID3D11Device* device, float radius, int slices, int stacks);
	// �~�����b�V���쐬
	void CreateCylinderMesh(ID3D11Device* device, float radius1, float radius2, float start, float height, int slices, int stacks);
	// �����̃��b�V���쐬
	void CreateCuboidMesh(ID3D11Device* device, float radius1, float radius2, float radius3);
	// �J�v�Z�����b�V���쐬
	void CreateCapsuleMesh(ID3D11Device* device, float radius1, float radius2,
		const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, int slices);
private:
	struct FigureConstants
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4 material_color;
	};
	std::unique_ptr<Constants<FigureConstants>> figure_constants;
	struct Sphere
	{
		DirectX::XMFLOAT4	color;
		DirectX::XMFLOAT3	center;
		float				radius;
	};
	struct Cylinder
	{
		DirectX::XMFLOAT4	color;
		DirectX::XMFLOAT3	position;
		float				radius;
		float				height;
	};
	struct Cuboid
	{
		DirectX::XMFLOAT4	color;
		DirectX::XMFLOAT3	position;
		DirectX::XMFLOAT3	radius;
	};
	struct Capsule
	{
		DirectX::XMFLOAT4	color;
		DirectX::XMFLOAT3	start{ 0,0,0 };	// �~���̒��S���̎n�[
		DirectX::XMFLOAT3	end{ 0,1,0 };	// �~���̒��S���̏I�[
		float				radius;	// ���a
	};

	Microsoft::WRL::ComPtr<ID3D11Buffer> sphereVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> cylinderVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> cuboidVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> cuboidIndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> capsuleVertexBuffer;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	std::vector<Sphere>	spheres;
	std::vector<Cylinder> cylinders;
	std::vector<Cuboid> cuboids;
	std::vector<Capsule> capsules;

	UINT sphereVertexCount = 0;
	UINT cylinderVertexCount = 0;
	UINT cuboidVertexCount = 0;
	UINT cuboidIndexCount = 0;
	UINT capsuleVertexCount = 0;

};


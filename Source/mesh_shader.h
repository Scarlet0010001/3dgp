#pragma once
#include "shader.h"
#include "constant.h"
#include "gltf_model.h"

class MeshShader :
    public Shader
{
public:
	MeshShader() {}
	MeshShader(ID3D11Device* device);
	virtual ~MeshShader() {}
	static const int MAX_BONES{ 256 };
public:

	//描画タイプ
	enum class RenderType
	{
		Forward,
		Deferred,
	};

	// 描画開始
	virtual void active(ID3D11DeviceContext* immediate_context, RenderType rt);
	virtual void active(ID3D11DeviceContext* immediate_context, ID3D11VertexShader* vertex_shader, ID3D11PixelShader* pixcel_shader);
	virtual void render(ID3D11DeviceContext* immediate_context, gltf_model* model, const DirectX::XMFLOAT4X4& world = {});
	virtual void render(ID3D11DeviceContext* immediate_context, gltf_model* model, DirectX::XMFLOAT4X4 camera_view, DirectX::XMFLOAT4X4 camara_proj, const DirectX::XMFLOAT4X4& world = {});



protected:
	struct OBJECT_CONSTANTS
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4 material_color;
	};


	struct MATERIAL_CONSTANTS
	{
		DirectX::XMFLOAT2 texcoord_offset{ 0.0f, 0.0f };
		DirectX::XMFLOAT2 texcoord_scale{ 1.0f, 1.0f };
		float emissive_power = 1.0f;
		DirectX::XMFLOAT3 pad;
	};

	struct BONE_CONSTANTS
	{
		DirectX::XMFLOAT4X4 bone_transforms[MAX_BONES]{ { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 } }; //最初は単位行列
	};

	std::unique_ptr<Constants<BONE_CONSTANTS>> bone_constants{};
	struct shader_resources
	{
		MATERIAL_CONSTANTS material_data;
		ID3D11ShaderResourceView* shader_resource_views[4];
	};

	std::unique_ptr<Constants<OBJECT_CONSTANTS>> object_constants{};
	std::unique_ptr<Constants<MATERIAL_CONSTANTS>> material_constants{};
	Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> d_pixel_shader;//ディファ―ドレンダリングの場合のPS


};


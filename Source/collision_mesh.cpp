#include "collision_mesh.h"

#include <stack>
#include <functional>


#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tinygltf-release/tiny_gltf.h"
#include <fbxsdk.h>

#include "user.h"

inline DirectX::XMFLOAT4X4 to_xmfloat4x4(const FbxAMatrix& fbxamatrix)
{
	DirectX::XMFLOAT4X4 xmfloat4x4;
	for (int row = 0; row < 4; row++)
	{
		for (int column = 0; column < 4; column++)
		{
			xmfloat4x4.m[row][column] = static_cast<float>(fbxamatrix[row][column]);
		}
	}
	return xmfloat4x4;
}

static bool null_load_image_data(tinygltf::Image*, const int, std::string*, std::string*, int, int, const unsigned char*, int, void*)
{
	return true;
}

CollisionMesh::CollisionMesh(ID3D11Device* device, const std::string& filename, bool triangulate)
{
	using namespace DirectX;

	if (filename.find(".fbx") != std::string::npos)
	{
		// Load from FBX file.
		FbxManager* fbx_manager = FbxManager::Create();
		FbxScene* fbx_scene = FbxScene::Create(fbx_manager, "");
		FbxImporter* fbx_importer = FbxImporter::Create(fbx_manager, "");
		bool import_status = false;
		import_status = fbx_importer->Initialize(filename.c_str());
		assert(import_status && "Failed to call FbxImporter::Initialize");
		import_status = fbx_importer->Import(fbx_scene);
		assert(import_status && "Failed to call FbxImporter::Import");

		FbxGeometryConverter fbx_converter(fbx_manager);
		if (triangulate)
		{
			fbx_converter.Triangulate(fbx_scene, true/*replace*/, false/*legacy*/);
			fbx_converter.RemoveBadPolygonsFromMeshes(fbx_scene);
		}

		std::function<void(FbxNode*)> traverse = [&](FbxNode* fbx_node) {
			if (fbx_node->GetNodeAttribute() && fbx_node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::EType::eMesh)
			{
				decltype(collision_meshes)::reference mesh = collision_meshes.emplace_back();
				mesh.name = fbx_node->GetName();

				XMMATRIX global_transform = XMLoadFloat4x4(&to_xmfloat4x4(fbx_node->EvaluateGlobalTransform()));

				FbxMesh* fbx_mesh = fbx_node->GetMesh();
				const int material_count = fbx_mesh->GetNode()->GetMaterialCount();
				mesh.subsets.resize(material_count > 0 ? material_count : 1);
				for (int material_index = 0; material_index < material_count; ++material_index)
				{
					const FbxSurfaceMaterial* fbx_material = fbx_mesh->GetNode()->GetMaterial(material_index);
					mesh.subsets.at(material_index).material_name = fbx_material->GetName();
				}

				const FbxVector4* control_points = fbx_mesh->GetControlPoints();
				const int polygon_count = fbx_mesh->GetPolygonCount();
				for (int polygon_index = 0; polygon_index < polygon_count; ++polygon_index)
				{
					const int material_index = material_count > 0 ? fbx_mesh->GetElementMaterial()->GetIndexArray().GetAt(polygon_index) : 0;
					decltype(mesh.subsets)::reference subset = mesh.subsets.at(material_index);

					for (int position_in_polygon = 0; position_in_polygon < 3; ++position_in_polygon)
					{
						DirectX::XMFLOAT3 position;
						const int polygon_vertex = fbx_mesh->GetPolygonVertex(polygon_index, position_in_polygon);
						position.x = static_cast<float>(control_points[polygon_vertex][0]);
						position.y = static_cast<float>(control_points[polygon_vertex][1]);
						position.z = static_cast<float>(control_points[polygon_vertex][2]);
						XMStoreFloat3(&position, XMVector3TransformCoord(XMLoadFloat3(&position), global_transform));
						subset.positions.emplace_back(position);
					}

				}
				for (decltype(mesh.subsets)::const_reference subset : mesh.subsets)
				{
					for (decltype(subset.positions)::const_reference position : subset.positions)
					{
						mesh.bounding_box[0].x = std::min<float>(mesh.bounding_box[0].x, position.x);
						mesh.bounding_box[0].y = std::min<float>(mesh.bounding_box[0].y, position.y);
						mesh.bounding_box[0].z = std::min<float>(mesh.bounding_box[0].z, position.z);
						mesh.bounding_box[1].x = std::max<float>(mesh.bounding_box[1].x, position.x);
						mesh.bounding_box[1].y = std::max<float>(mesh.bounding_box[1].y, position.y);
						mesh.bounding_box[1].z = std::max<float>(mesh.bounding_box[1].z, position.z);
					}
				}
			}
			for (int child_index = 0; child_index < fbx_node->GetChildCount(); ++child_index)
			{
				traverse(fbx_node->GetChild(child_index));
			}
		};
		traverse(fbx_scene->GetRootNode());

		fbx_manager->Destroy();
	}
	else
	{
		// Load from glTF file.
		tinygltf::TinyGLTF tiny_gltf;
		tiny_gltf.SetImageLoader(null_load_image_data, nullptr);

		tinygltf::Model gltf_model;
		std::string error, warning;
		bool succeeded{ false };

		if (filename.find(".glb") != std::string::npos)
		{
			succeeded = tiny_gltf.LoadBinaryFromFile(&gltf_model, &error, &warning, filename.c_str());
		}
		else if (filename.find(".gltf") != std::string::npos)
		{
			succeeded = tiny_gltf.LoadASCIIFromFile(&gltf_model, &error, &warning, filename.c_str());
		}
		assert(succeeded && "Failed to load glTF file");

		std::stack<DirectX::XMFLOAT4X4> parent_global_transforms;
		std::function<void(int)> traverse = [&](int node_index)
		{
			decltype(gltf_model.nodes)::value_type gltf_node = gltf_model.nodes.at(node_index);

			DirectX::XMFLOAT4X4 global_transform = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
			if (!gltf_node.matrix.empty())
			{
				DirectX::XMFLOAT4X4 transform = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
				for (size_t row = 0; row < 4; row++)
				{
					for (size_t column = 0; column < 4; column++)
					{
						transform(row, column) = static_cast<float>(gltf_node.matrix.at(4 * row + column));
					}
				}
				DirectX::XMStoreFloat4x4(&global_transform, DirectX::XMLoadFloat4x4(&transform) * DirectX::XMLoadFloat4x4(&parent_global_transforms.top()));
			}
			else
			{
				DirectX::XMMATRIX S = gltf_node.scale.size() > 0 ? DirectX::XMMatrixScaling(static_cast<float>(gltf_node.scale.at(0)), static_cast<float>(gltf_node.scale.at(1)), static_cast<float>(gltf_node.scale.at(2))) : DirectX::XMMatrixIdentity();
				DirectX::XMMATRIX R = gltf_node.rotation.size() > 0 ? DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(static_cast<float>(gltf_node.rotation.at(0)), static_cast<float>(gltf_node.rotation.at(1)), static_cast<float>(gltf_node.rotation.at(2)), static_cast<float>(gltf_node.rotation.at(3)))) : DirectX::XMMatrixIdentity();
				DirectX::XMMATRIX T = gltf_node.translation.size() > 0 ? DirectX::XMMatrixTranslation(static_cast<float>(gltf_node.translation.at(0)), static_cast<float>(gltf_node.translation.at(1)), static_cast<float>(gltf_node.translation.at(2))) : DirectX::XMMatrixIdentity();
				DirectX::XMStoreFloat4x4(&global_transform, S * R * T * DirectX::XMLoadFloat4x4(&parent_global_transforms.top()));
			}

			if (gltf_node.mesh > -1)
			{
				std::vector<tinygltf::Mesh>::const_reference gltf_mesh = gltf_model.meshes.at(gltf_node.mesh);
				decltype(collision_meshes)::reference mesh = collision_meshes.emplace_back();
				mesh.name = gltf_mesh.name;
				for (std::vector<tinygltf::Primitive>::const_reference gltf_primitive : gltf_mesh.primitives)
				{
					std::vector<tinygltf::Material>::const_reference gltf_material = gltf_model.materials.at(gltf_primitive.material);
					decltype(mesh.subsets)::reference subset = mesh.subsets.emplace_back();
					subset.material_name = gltf_material.name;

					const tinygltf::Accessor& index_accessor = gltf_model.accessors.at(gltf_primitive.indices);
					const tinygltf::BufferView& index_buffer_view = gltf_model.bufferViews.at(index_accessor.bufferView);
					assert(index_accessor.type == TINYGLTF_TYPE_SCALAR);
					assert(index_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT ||
								index_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT);
					
					unsigned int* indices = reinterpret_cast<unsigned int*>
						(gltf_model.buffers.at(index_buffer_view.buffer).data.data() + index_buffer_view.byteOffset + index_accessor.byteOffset);
					unsigned short* indices_short{};
					if (index_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
					{
						indices_short = reinterpret_cast<unsigned short*>(gltf_model.buffers.at(index_buffer_view.buffer).data.data() + index_buffer_view.byteOffset + index_accessor.byteOffset);
					}

					const tinygltf::Accessor& position_accessor = gltf_model.accessors.at(gltf_primitive.attributes.at("POSITION"));
					const tinygltf::BufferView& position_buffer_view = gltf_model.bufferViews.at(position_accessor.bufferView);
					DirectX::XMFLOAT3* positions = reinterpret_cast<DirectX::XMFLOAT3*>(gltf_model.buffers.at(position_buffer_view.buffer).data.data() + position_buffer_view.byteOffset + position_accessor.byteOffset);

					for (size_t index_index = 0; index_index < index_accessor.count; ++index_index)
					{
						unsigned int index = 0;
						if (index_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
						{//componentTypeがshortだった場合
							index = static_cast<unsigned int>(indices_short[index_index]);
						}
						else
						{//componentTypeがintだった場合
							index = indices[index_index];
						}
						DirectX::XMFLOAT3 position = positions[index];
						DirectX::XMStoreFloat3(&position, XMVector3TransformCoord(XMLoadFloat3(&position), DirectX::XMLoadFloat4x4(&global_transform)));
						subset.positions.emplace_back(position);
					}
				}
			}
			for (decltype(gltf_node.children)::value_type child_index : gltf_node.children)
			{
				parent_global_transforms.push(global_transform);
				traverse(child_index);
				parent_global_transforms.pop();
			}
		};
		decltype(gltf_model.scenes)::const_reference gltf_scene = gltf_model.scenes.at(0);
		for (decltype(gltf_scene.nodes)::value_type root_node : gltf_scene.nodes)
		{
			parent_global_transforms.push({ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 });
			traverse(root_node);
			parent_global_transforms.pop();
		}
	}

	// Rebuild bounding box
	for (decltype(collision_meshes)::reference mesh : collision_meshes)
	{
		for (decltype(mesh.subsets)::reference subset : mesh.subsets)
		{
			for (decltype(subset.positions)::reference position : subset.positions)
			{
				mesh.bounding_box[0].x = std::min<float>(mesh.bounding_box[0].x, position.x);
				mesh.bounding_box[0].y = std::min<float>(mesh.bounding_box[0].y, position.y);
				mesh.bounding_box[0].z = std::min<float>(mesh.bounding_box[0].z, position.z);
				mesh.bounding_box[1].x = std::max<float>(mesh.bounding_box[1].x, position.x);
				mesh.bounding_box[1].y = std::max<float>(mesh.bounding_box[1].y, position.y);
				mesh.bounding_box[1].z = std::max<float>(mesh.bounding_box[1].z, position.z);
			}
		}
	}

}

inline bool intersect_ray_aabb(const float p[3], const float d[3], const float p0[3], const float p1[3])
{
	float t_min = 0;
	float t_max = +FLT_MAX;

	for (size_t a = 0; a < 3; ++a)
	{
		float inv_d = 1.0f / d[a];
		float t0 = (p0[a] - p[a]) * inv_d;
		float t1 = (p1[a] - p[a]) * inv_d;
		if (inv_d < 0.0f)
		{
			std::swap<float>(t0, t1);
		}
		t_min = std::max<float>(t0, t_min);
		t_max = std::min<float>(t1, t_max);

		if (t_max <= t_min)
		{
			return false;
		}
	}
	return true;
}

bool CollisionMesh::RayCast(DirectX::XMFLOAT3 ray_start, 
	DirectX::XMFLOAT3 ray_end, 
	const DirectX::XMFLOAT4X4& transform,
	//DirectX::XMFLOAT3& intersection_position,
	//DirectX::XMFLOAT3& intersection_normal,
	//std::string& intersection_mesh,
	//std::string& intersection_material,
	HitResult& hit,
	 bool skip_if) const
{
	using namespace DirectX;

	XMMATRIX T = XMLoadFloat4x4(&transform);
	XMMATRIX _T = XMMatrixInverse(nullptr, T);

	// Convert to model space.
	//XMStoreFloat3(&ray_position, XMVector3TransformCoord(XMLoadFloat3(&ray_position), _T));
	//XMStoreFloat3(&ray_direction, XMVector3TransformNormal(XMLoadFloat3(&ray_direction), _T));

	XMVECTOR T_start =
		XMVector3TransformCoord(XMLoadFloat3(&ray_start), _T);
	XMVECTOR T_end =
		XMVector3TransformCoord(XMLoadFloat3(&ray_end), _T);
	XMVECTOR T_vec = XMVectorSubtract(T_end, T_start);

	const XMVECTOR P = T_start;
	const XMVECTOR D = XMVector3Normalize(T_vec);

	int intersection_count = 0;
	float closest_distance = FLT_MAX;

	for (decltype(collision_meshes)::const_reference mesh : collision_meshes)
	{
#if 0
		const float* aabb_min = reinterpret_cast<const float*>(&mesh.bounding_box[0]);
		const float* aabb_max = reinterpret_cast<const float*>(&mesh.bounding_box[1]);
		if (!intersect_ray_aabb(reinterpret_cast<const float*>(&ray_position), reinterpret_cast<const float*>(&ray_direction), aabb_min, aabb_max))
		{
			continue;
		}
#endif
		for (decltype(mesh.subsets)::const_reference subset : mesh.subsets)
		{
			const XMFLOAT3* positions = subset.positions.data();
			const size_t triangle_count = subset.positions.size() / 3;
			for (size_t triangle_index = 0; triangle_index < triangle_count; triangle_index++)
			{
				const XMVECTOR A = XMLoadFloat3(&positions[triangle_index * 3 + 0]);
				const XMVECTOR B = XMLoadFloat3(&positions[triangle_index * 3 + 1]);
				const XMVECTOR C = XMLoadFloat3(&positions[triangle_index * 3 + 2]);

				const XMVECTOR N = XMVector3Normalize(XMVector3Cross(XMVectorSubtract(B, A), XMVectorSubtract(C, A)));
				const float d = XMVectorGetByIndex(XMVector3Dot(N, A), 0);
				const float denominator{ XMVectorGetByIndex(XMVector3Dot(N, D), 0) };
				if (denominator < 0) // Note that if N.D = 0 , then D is parallel to the plane and the ray does not intersect the plane.
				{
					const float numerator = d - XMVectorGetByIndex(XMVector3Dot(N, P), 0);
					const float t = numerator / denominator;

					if (t > 0 && t < XMVectorGetX(XMVector3Length(T_vec))) // Forward and Length limit of Ray
					{
						XMVECTOR Q = XMVectorAdd(P, XMVectorScale(D, t));

						const XMVECTOR QA = XMVectorSubtract(A, Q);
						const XMVECTOR QB = XMVectorSubtract(B, Q);
						const XMVECTOR QC = XMVectorSubtract(C, Q);

						XMVECTOR U = XMVector3Cross(QB, QC);
						XMVECTOR V = XMVector3Cross(QC, QA);
						if (XMVectorGetByIndex(XMVector3Dot(U, V), 0) < 0)
						{
							continue;
						}

						XMVECTOR W = XMVector3Cross(QA, QB);
						if (XMVectorGetByIndex(XMVector3Dot(U, W), 0) < 0)
						{
							continue;
						}
						if (XMVectorGetByIndex(XMVector3Dot(V, W), 0) < 0)
						{
							continue;
						}
						intersection_count++;

						if (t < closest_distance)
						{
							XMStoreFloat3(&hit.position, XMVector3TransformCoord(Q, T));
							XMStoreFloat3(&hit.normal, XMVector3Normalize(XMVector3TransformNormal(N, T)));
							//もう一度ここで距離を計算しなおすかワールドに戻す
							//closest_distance = t;
							//hit.distance = closest_distance;
							hit.distance = Math::calc_vector_AtoB_length(ray_start, hit.position);
							hit.meshName = mesh.name;
							hit.materialName = subset.material_name;
							if (skip_if)
							{
								return true;
							}
						}
					}
				}
			}
		}
	}
	return intersection_count > 0;
}
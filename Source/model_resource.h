#pragma once
#define NOMINMAX
#include <d3d11.h>
#include <wrl.h>
#include <directxmath.h>
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tinygltf-release/tiny_gltf.h"
#include <unordered_map>

class ModelResource
{
private:
    std::string filename;   // ファイル名

public:
    ModelResource(ID3D11Device* device, std::string filename,
        float sampling_rate, bool triangulate = false);
    virtual ~ModelResource() = default;

public: //構造体
    struct scene
    {
        std::string name;
        std::vector<int> nodes;// 'root'ノードのインデックス配列
    };
    std::vector<scene> scenes;

    struct node
    {
        std::string name;
        int skin{ -1 }; // このノードが参照するスキンのインデックス
        int mesh{ -1 }; // このノードが参照するメッシュのインデックス

        std::vector<int> children; // このノードの子ノードのインデックス配列

        // Local transforms
        DirectX::XMFLOAT4 rotation{ 0,0,0,1 };
        DirectX::XMFLOAT3 scale{ 1,1,1 };
        DirectX::XMFLOAT3 translation{ 0,0,0 };

        DirectX::XMFLOAT4X4 global_transform{
            1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            0,0,0,1,
        };
    };
    std::vector<node> nodes;

    struct buffer_view
    {
        DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
        Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
        size_t stride_in_bytes{ 0 };
        size_t size_in_bytes{ 0 };

        // 頂点数を計算する関数
        size_t count() const
        {
            return size_in_bytes / stride_in_bytes;
        }
    };

    struct mesh
    {
        std::string name;
        struct primitive
        {
            int material;
            std::map<std::string, buffer_view> vertex_buffer_views;
            buffer_view index_buffer_view;
        };
        std::vector<primitive> primitives;
    };
    std::vector<mesh> meshes;

    // テクスチャ情報構造体
    struct texture_info
    {
        int index = -1; // テクスチャのインデックス（デフォルトは -1）
        int texcoord = 0; // UV マッピング用のテクスチャ座標セット（デフォルトは 0）
    };
    // 法線マップのテクスチャ情報構造体
    struct normal_texture_info
    {
        int index = -1;    // 法線マップのテクスチャのインデックス（デフォルトは -1）
        int texcoord = 0;  // UV マッピング用のテクスチャ座標セット（デフォルトは 0）
        float scale = 1;   // 法線マップのスケール係数（デフォルトは 1）
    };
    // オクルージョンマップのテクスチャ情報構造体
    struct occlusion_texture_info
    {
        int index = -1;    // オクルージョンマップのテクスチャのインデックス（デフォルトは -1）
        int texcoord = 0;  // UV マッピング用のテクスチャ座標セット（デフォルトは 0）
        float strength = 1; // オクルージョンの強度（デフォルトは 1）
    };
    // PBR（Physically Based Rendering）メタリックローファネス情報構造体
    struct pbr_metallic_roughness
    {
        float basecolor_factor[4] = { 1, 1, 1, 1 };   // 基本色の係数（RGBA 形式）
        texture_info basecolor_texture;                 // 基本色のテクスチャ情報
        float metallic_factor = 1;                     // メタリック度
        float roughness_factor = 1;                    // ローファネス度
        texture_info metallic_roughness_texture;       // メタリックローファネステクスチャ情報
    };


    // マテリアル構造体
    struct material
    {
        std::string name;  // マテリアルの名前
        struct cbuffer
        {
            float emissive_factor[3] = { 0, 0, 0 };    // 発光色の係数（RGB 形式）
            int alpha_mode = 0;                         // アルファモード（"OPAQUE" : 0, "MASK" : 1, "BLEND" : 2）
            float alpha_cutoff = 0.5f;                  // アルファカットオフ値
            bool double_sided = false;                  // 両面表示かどうか

            pbr_metallic_roughness pbr_metallic_roughness; // PBRメタリックローファネス情報
            normal_texture_info normal_texture;            // 法線マップのテクスチャ情報
            occlusion_texture_info occlusion_texture;      // オクルージョンマップのテクスチャ情報
            texture_info emissive_texture;                 // 発光テクスチャ情報
        };
        cbuffer data; // マテリアルの定数バッファ
    };
    // マテリアルのリスト
    std::vector<material> materials;
    // マテリアルデータのシェーダリソースビュー
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> material_resource_view;

    // テクスチャ構造体
    struct texture
    {
        std::string name; // テクスチャの名前
        int source{ -1 }; // テクスチャのソース（インデックス）
    };
    std::vector<texture> textures; // テクスチャのリスト
    
    // 画像情報構造体
    struct image
    {
        std::string name;
        int width{ -1 };
        int height{ -1 };
        int component{ -1 };
        int bits{ -1 };
        int pixel_type{ -1 };
        int buffer_view;
        std::string mime_type;
        std::string uri;
        bool as_is{ false };
    };
    std::vector<image> images;
    std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> texture_resource_views;


    struct skin
    {
        std::vector<DirectX::XMFLOAT4X4> inverse_bind_matrices;
        std::vector<int> joints;
    };
    std::vector<skin> skins;

    struct animation
    {
        std::string name;

        struct channel
        {
            int sampler{ -1 };
            int target_node{ -1 };
            std::string target_path;
        };
        std::vector<channel> channels;

        struct sampler
        {
            int input{ -1 };
            int output{ -1 };
            std::string interpolation;
        };
        std::vector<sampler> samplers;

        std::unordered_map<int, std::vector<float>> timelines;
        std::unordered_map<int, std::vector<DirectX::XMFLOAT3>> scales;
        std::unordered_map<int, std::vector<DirectX::XMFLOAT4>> rotations;
        std::unordered_map<int, std::vector<DirectX::XMFLOAT3>> translations;
    };
    std::vector<animation> animations;


    struct primitive_constants
    {
        DirectX::XMFLOAT4X4 world;
        int material{ -1 };
        int has_tangent{ 0 };
        int skin{ -1 };
        int pad;
    };

    static const size_t PRIMITIVE_MAX_JOINTS = 512;
    struct primitive_joint_constants
    {
        DirectX::XMFLOAT4X4 matrices[PRIMITIVE_MAX_JOINTS];
    };
    Microsoft::WRL::ComPtr<ID3D11Buffer> primitive_joint_cbuffer;

private:
    Microsoft::WRL::ComPtr<ID3D11Buffer> primitive_cbuffer;

    // glTFノードの情報を取得
    void fetch_nodes(const tinygltf::Model& gltf_model);

    // glTFメッシュ情報を取得
    void fetch_meshes(ID3D11Device* device, const tinygltf::Model& gltf_model);

    // glTFマテリアル情報を取得
    void fetch_materials(ID3D11Device* device, const tinygltf::Model& gltf_model);

    // glTFテクスチャ情報を取得
    void fetch_textures(ID3D11Device* device, const tinygltf::Model& gltf_model);

    // glTFアニメーション情報を取得
    void fetch_animations(const tinygltf::Model& gltf_model);

    // ノードのグローバル変換行列を累積する関数
    void cumulate_transforms(std::vector<node>& nodes);

    void animate(size_t animation_index, float time, std::vector<node>& animated_nodes, bool loopback);

    // glTFアクセッサからバッファビューを作成
    buffer_view make_buffer_view(const tinygltf::Accessor& accessor);
    
    //gltf_modelを元にmodel_resourceを作る
};


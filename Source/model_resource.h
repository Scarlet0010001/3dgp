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
    std::string filename;   // �t�@�C����

public:
    ModelResource(ID3D11Device* device, std::string filename,
        float sampling_rate, bool triangulate = false);
    virtual ~ModelResource() = default;

public: //�\����
    struct scene
    {
        std::string name;
        std::vector<int> nodes;// 'root'�m�[�h�̃C���f�b�N�X�z��
    };
    std::vector<scene> scenes;

    struct node
    {
        std::string name;
        int skin{ -1 }; // ���̃m�[�h���Q�Ƃ���X�L���̃C���f�b�N�X
        int mesh{ -1 }; // ���̃m�[�h���Q�Ƃ��郁�b�V���̃C���f�b�N�X

        std::vector<int> children; // ���̃m�[�h�̎q�m�[�h�̃C���f�b�N�X�z��

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

        // ���_�����v�Z����֐�
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

    // �e�N�X�`�����\����
    struct texture_info
    {
        int index = -1; // �e�N�X�`���̃C���f�b�N�X�i�f�t�H���g�� -1�j
        int texcoord = 0; // UV �}�b�s���O�p�̃e�N�X�`�����W�Z�b�g�i�f�t�H���g�� 0�j
    };
    // �@���}�b�v�̃e�N�X�`�����\����
    struct normal_texture_info
    {
        int index = -1;    // �@���}�b�v�̃e�N�X�`���̃C���f�b�N�X�i�f�t�H���g�� -1�j
        int texcoord = 0;  // UV �}�b�s���O�p�̃e�N�X�`�����W�Z�b�g�i�f�t�H���g�� 0�j
        float scale = 1;   // �@���}�b�v�̃X�P�[���W���i�f�t�H���g�� 1�j
    };
    // �I�N���[�W�����}�b�v�̃e�N�X�`�����\����
    struct occlusion_texture_info
    {
        int index = -1;    // �I�N���[�W�����}�b�v�̃e�N�X�`���̃C���f�b�N�X�i�f�t�H���g�� -1�j
        int texcoord = 0;  // UV �}�b�s���O�p�̃e�N�X�`�����W�Z�b�g�i�f�t�H���g�� 0�j
        float strength = 1; // �I�N���[�W�����̋��x�i�f�t�H���g�� 1�j
    };
    // PBR�iPhysically Based Rendering�j���^���b�N���[�t�@�l�X���\����
    struct pbr_metallic_roughness
    {
        float basecolor_factor[4] = { 1, 1, 1, 1 };   // ��{�F�̌W���iRGBA �`���j
        texture_info basecolor_texture;                 // ��{�F�̃e�N�X�`�����
        float metallic_factor = 1;                     // ���^���b�N�x
        float roughness_factor = 1;                    // ���[�t�@�l�X�x
        texture_info metallic_roughness_texture;       // ���^���b�N���[�t�@�l�X�e�N�X�`�����
    };


    // �}�e���A���\����
    struct material
    {
        std::string name;  // �}�e���A���̖��O
        struct cbuffer
        {
            float emissive_factor[3] = { 0, 0, 0 };    // �����F�̌W���iRGB �`���j
            int alpha_mode = 0;                         // �A���t�@���[�h�i"OPAQUE" : 0, "MASK" : 1, "BLEND" : 2�j
            float alpha_cutoff = 0.5f;                  // �A���t�@�J�b�g�I�t�l
            bool double_sided = false;                  // ���ʕ\�����ǂ���

            pbr_metallic_roughness pbr_metallic_roughness; // PBR���^���b�N���[�t�@�l�X���
            normal_texture_info normal_texture;            // �@���}�b�v�̃e�N�X�`�����
            occlusion_texture_info occlusion_texture;      // �I�N���[�W�����}�b�v�̃e�N�X�`�����
            texture_info emissive_texture;                 // �����e�N�X�`�����
        };
        cbuffer data; // �}�e���A���̒萔�o�b�t�@
    };
    // �}�e���A���̃��X�g
    std::vector<material> materials;
    // �}�e���A���f�[�^�̃V�F�[�_���\�[�X�r���[
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> material_resource_view;

    // �e�N�X�`���\����
    struct texture
    {
        std::string name; // �e�N�X�`���̖��O
        int source{ -1 }; // �e�N�X�`���̃\�[�X�i�C���f�b�N�X�j
    };
    std::vector<texture> textures; // �e�N�X�`���̃��X�g
    
    // �摜���\����
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

    // glTF�m�[�h�̏����擾
    void fetch_nodes(const tinygltf::Model& gltf_model);

    // glTF���b�V�������擾
    void fetch_meshes(ID3D11Device* device, const tinygltf::Model& gltf_model);

    // glTF�}�e���A�������擾
    void fetch_materials(ID3D11Device* device, const tinygltf::Model& gltf_model);

    // glTF�e�N�X�`�������擾
    void fetch_textures(ID3D11Device* device, const tinygltf::Model& gltf_model);

    // glTF�A�j���[�V���������擾
    void fetch_animations(const tinygltf::Model& gltf_model);

    // �m�[�h�̃O���[�o���ϊ��s���ݐς���֐�
    void cumulate_transforms(std::vector<node>& nodes);

    void animate(size_t animation_index, float time, std::vector<node>& animated_nodes, bool loopback);

    // glTF�A�N�Z�b�T����o�b�t�@�r���[���쐬
    buffer_view make_buffer_view(const tinygltf::Accessor& accessor);
    
    //gltf_model������model_resource�����
};


#include "model_resource.h"
#define TINYGLTF_IMPLEMENTATION
#include "gltf_model.h"
#include <stack>
#include "shader.h"
#include "texture.h"
#include "misc.h"

// 画像データを読み込まないtinygltf用の画像ローダー関数
bool null_load_image_data(tinygltf::Image*, const int, std::string*, std::string*,
    int, int, const unsigned char*, int, void*)
{
    return true;    // 画像データの読み込みを無視し、常に成功として返す
}

ModelResource::ModelResource(ID3D11Device* device, std::string filename, float sampling_rate, bool triangulate)
{
    // tinygltfのインスタンスを生成し、画像ローダー関数を設定
    tinygltf::TinyGLTF tiny_gltf;
    tiny_gltf.SetImageLoader(null_load_image_data, nullptr);

    // glTFモデルのための変数を宣言
    tinygltf::Model gltf_model;
    std::string error, warning;
    bool succeeded{ false };

    this->filename = filename;

    // ファイルの拡張子によって、バイナリ形式またはASCII形式で読み込む
    if (filename.find(".glb") != std::string::npos)
    {
        succeeded = tiny_gltf.LoadBinaryFromFile(
            &gltf_model, &error, &warning, filename.c_str());
    }
    else if (filename.find(".gltf") != std::string::npos)
    {
        succeeded = tiny_gltf.LoadASCIIFromFile(
            &gltf_model, &error, &warning, filename.c_str());
    }

    // 警告やエラーメッセージの表示と読み込みの成功を確認
    _ASSERT_EXPR_A(warning.empty(), warning.c_str());
    _ASSERT_EXPR_A(error.empty(), error.c_str());
    _ASSERT_EXPR_A(succeeded, L"Failed to load glTF file");

    // glTFモデルからシーン情報を抽出してクラス内のシーンvectorに追加
    for (std::vector<tinygltf::Scene>::const_reference gltf_scene : gltf_model.scenes)
    {
        // シーン情報の追加
        scene& scene{ scenes.emplace_back() };
        scene.name = gltf_scene.name;
        scene.nodes = gltf_scene.nodes;
    }

    // ノード情報の取得
    fetch_nodes(gltf_model);

    // メッシュ情報の取得
    fetch_meshes(device, gltf_model);

    // マテリアル情報の取得
    fetch_materials(device, gltf_model);

    // テクスチャ情報の取得
    fetch_textures(device, gltf_model);

    // アニメーション情報の取得
    fetch_animations(gltf_model);

    //これは強制的なプログラミングであり、バグが発生する可能性があります。
    // メッシュから頂点バッファビューを取得
    const std::map<std::string, buffer_view>& vertex_buffer_views{
        meshes.at(0).primitives.at(0).vertex_buffer_views
    };
    // 入力レイアウトの定義
    D3D11_INPUT_ELEMENT_DESC input_element_desc[]
    {
        { "POSITION", 0, vertex_buffer_views.at("POSITION").format, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, vertex_buffer_views.at("NORMAL").format,1,0,D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, vertex_buffer_views.at("TANGENT").format,2,0,D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, vertex_buffer_views.at("TEXCOORD_0").format,3,0,D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"JOINTS", 0, vertex_buffer_views.at("JOINTS_0").format,4,0,D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"WEIGHTS", 0, vertex_buffer_views.at("WEIGHTS_0").format,5,0,D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    // 頂点シェーダーとピクセルシェーダーをコンパイルして生成
    //create_vs_from_cso(device, "gltf_model_vs.cso", vertex_shader.ReleaseAndGetAddressOf(),
    //    input_layout.ReleaseAndGetAddressOf(), input_element_desc, _countof(input_element_desc));
    //create_ps_from_cso(device, "gltf_model_ps.cso", pixel_shader.ReleaseAndGetAddressOf());

    // プリミティブ用の定数バッファの作成
    D3D11_BUFFER_DESC buffer_desc{};
    buffer_desc.ByteWidth = sizeof(primitive_constants);
    buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    HRESULT hr;
    hr = device->CreateBuffer(&buffer_desc, nullptr, primitive_cbuffer.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    // プリミティブのジョイント用定数バッファの作成
    buffer_desc.ByteWidth = sizeof(primitive_joint_constants);
    buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    hr = device->CreateBuffer(&buffer_desc, NULL, primitive_joint_cbuffer.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

}

void ModelResource::fetch_nodes(const tinygltf::Model& gltf_model)
{
    // glTFモデルからノード情報を抽出してクラス内のノードvectorに追加
    for (std::vector<tinygltf::Node>::const_reference gltf_node
        : gltf_model.nodes)
    {
        // クラス内のノードvectorに新しいノードを追加
        node& node{ nodes.emplace_back() };

        // ノードの基本情報を設定
        node.name = gltf_node.name;
        node.skin = gltf_node.skin;
        node.mesh = gltf_node.mesh;
        node.children = gltf_node.children;

        // ノードの変換情報を取得
        if (!gltf_node.matrix.empty())
        {
            // マトリックスが指定されている場合、それを分解して変換情報を取得
            DirectX::XMFLOAT4X4 matrix;
            for (size_t row = 0; row < 4; row++)
            {
                for (size_t column = 0; column < 4; column++)
                {
                    matrix(row, column) = static_cast<float>(gltf_node.matrix.at(4 * row + column));
                }
            }
            DirectX::XMVECTOR S, T, R;
            bool succeed = DirectX::XMMatrixDecompose(&S, &R, &T, DirectX::XMLoadFloat4x4(&matrix));
            _ASSERT_EXPR(succeed, L"Failed to decompose matrix.");

            // 取得した変換情報をノードに設定
            DirectX::XMStoreFloat3(&node.scale, S);
            DirectX::XMStoreFloat4(&node.rotation, R);
            DirectX::XMStoreFloat3(&node.translation, T);
        }
        else
        {
            // マトリックスが指定されていない場合、各変換情報を直接取得
            if (gltf_node.scale.size() > 0)
            {
                node.scale.x = static_cast<float>(gltf_node.scale.at(0));
                node.scale.y = static_cast<float>(gltf_node.scale.at(1));
                node.scale.z = static_cast<float>(gltf_node.scale.at(2));
            }
            if (gltf_node.translation.size() > 0)
            {
                node.translation.x = static_cast<float>(gltf_node.translation.at(0));
                node.translation.y = static_cast<float>(gltf_node.translation.at(1));
                node.translation.z = static_cast<float>(gltf_node.translation.at(2));
            }
            if (gltf_node.rotation.size() > 0)
            {
                node.rotation.x = static_cast<float>(gltf_node.rotation.at(0));
                node.rotation.y = static_cast<float>(gltf_node.rotation.at(1));
                node.rotation.z = static_cast<float>(gltf_node.rotation.at(2));
            }
        }
    }

    // 取得したノード情報を元に、ノードの変換情報を累積させる
    cumulate_transforms(nodes);

}

void ModelResource::fetch_meshes(ID3D11Device* device, const tinygltf::Model& gltf_model)
{
    HRESULT hr;

    // glTFモデルからメッシュ情報を抽出してクラス内のメッシュvectorに追加
    for (std::vector<tinygltf::Mesh>::const_reference gltf_mesh : gltf_model.meshes)
    {
        // クラス内のメッシュvectorに新しいメッシュを追加
        mesh& mesh{ meshes.emplace_back() };
        mesh.name = gltf_mesh.name;

        // メッシュ内のプリミティブごとに処理
        for (std::vector<tinygltf::Primitive>::const_reference gltf_primitive : gltf_mesh.primitives)
        {
            // メッシュ内のプリミティブごとに新しいプリミティブを追加
            mesh::primitive& primitive{ mesh.primitives.emplace_back() };
            primitive.material = gltf_primitive.material;

            // インデックスバッファの作成
            const tinygltf::Accessor& gltf_accessor{
                gltf_model.accessors.at(gltf_primitive.indices) };
            const tinygltf::BufferView& gltf_buffer_view{
                gltf_model.bufferViews.at(gltf_accessor.bufferView) };

            primitive.index_buffer_view = make_buffer_view(gltf_accessor);

            // インデックスバッファの設定
            D3D11_BUFFER_DESC buffer_desc{};
            buffer_desc.ByteWidth = static_cast<UINT>(
                primitive.index_buffer_view.size_in_bytes);
            buffer_desc.Usage = D3D11_USAGE_DEFAULT;
            buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

            // インデックスバッファの初期データを設定
            D3D11_SUBRESOURCE_DATA subresource_data{};
            subresource_data.pSysMem = gltf_model.buffers.at(gltf_buffer_view.buffer).data.data()
                + gltf_buffer_view.byteOffset + gltf_accessor.byteOffset;

            hr = device->CreateBuffer(&buffer_desc, &subresource_data,
                primitive.index_buffer_view.buffer.ReleaseAndGetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

            // 頂点バッファの作成
            for (std::map<std::string, int>::const_reference gltf_attribute : gltf_primitive.attributes)
            {
                // glTFアクセッサから頂点バッファの情報を取得
                const tinygltf::Accessor& gltf_accessor{ gltf_model.accessors.at(gltf_attribute.second) };
                // glTFバッファビューから頂点バッファのビュー情報を取得
                const tinygltf::BufferView& gltf_buffer_view{ gltf_model.bufferViews.at(gltf_accessor.bufferView) };

                // 頂点バッファのビューを作成
                buffer_view vertex_buffer_view{ make_buffer_view(gltf_accessor) };

                // 頂点バッファの設定を行う構造体
                D3D11_BUFFER_DESC buffer_desc{};
                buffer_desc.ByteWidth = static_cast<UINT>(vertex_buffer_view.size_in_bytes);
                buffer_desc.Usage = D3D11_USAGE_DEFAULT;
                buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

                // 頂点バッファの初期データを設定する構造体
                D3D11_SUBRESOURCE_DATA subresource_data{};
                subresource_data.pSysMem = gltf_model.buffers.at(gltf_buffer_view.buffer).data.data()
                    + gltf_buffer_view.byteOffset + gltf_accessor.byteOffset;

                // 頂点バッファを作成
                hr = device->CreateBuffer(&buffer_desc, &subresource_data,
                    vertex_buffer_view.buffer.ReleaseAndGetAddressOf());
                _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

                primitive.vertex_buffer_views.emplace(std::make_pair(gltf_attribute.first, vertex_buffer_view));
            }

            // 不足している場合は、ダミーの属性を追加
            const std::unordered_map<std::string, buffer_view> attributes{
                {"TANGENT",{DXGI_FORMAT_R32G32B32A32_FLOAT}},
                {"TEXCOORD_0",{DXGI_FORMAT_R32G32_FLOAT}},
                {"JOINTS_0",{DXGI_FORMAT_R16G16B16A16_UINT}},
                {"WEIGHTS_0",{DXGI_FORMAT_R32G32B32A32_FLOAT}},
            };
            for (std::unordered_map<std::string, buffer_view>::const_reference attribute : attributes)
            {
                // プリミティブの頂点バッファビューにダミー属性が存在しない場合、追加
                if (primitive.vertex_buffer_views.find(attribute.first) == primitive.vertex_buffer_views.end())
                {
                    primitive.vertex_buffer_views.insert(std::make_pair(attribute.first, attribute.second));
                }
            }
        }
    }
}

void ModelResource::fetch_materials(ID3D11Device* device, const tinygltf::Model& gltf_model)
{
    // GLTFモデルのマテリアルごとに処理
    for (std::vector<tinygltf::Material>::const_reference gltf_material : gltf_model.materials)
    {
        // マテリアルデータを新しいマテリアルに追加
        std::vector<material>::reference material = materials.emplace_back();

        // マテリアルの名前を設定
        material.name = gltf_material.name;

        //cbufferにマテリアルの各プロパティデータを送る
        material.data.emissive_factor[0] = static_cast<float>(gltf_material.emissiveFactor.at(0));
        material.data.emissive_factor[1] = static_cast<float>(gltf_material.emissiveFactor.at(1));
        material.data.emissive_factor[2] = static_cast<float>(gltf_material.emissiveFactor.at(2));

        material.data.alpha_mode = gltf_material.alphaMode == "OPAQUE" ?
            0 : gltf_material.alphaMode == "MASK" ? 1 : gltf_material.alphaMode == "BLEND" ? 2 : 0;
        material.data.alpha_cutoff = static_cast<float>(gltf_material.alphaCutoff);
        material.data.double_sided = gltf_material.doubleSided ? 1 : 0;

        material.data.pbr_metallic_roughness.basecolor_factor[0] =
            static_cast<float>(gltf_material.pbrMetallicRoughness.baseColorFactor.at(0));
        material.data.pbr_metallic_roughness.basecolor_factor[1] =
            static_cast<float>(gltf_material.pbrMetallicRoughness.baseColorFactor.at(1));
        material.data.pbr_metallic_roughness.basecolor_factor[2] =
            static_cast<float>(gltf_material.pbrMetallicRoughness.baseColorFactor.at(2));
        material.data.pbr_metallic_roughness.basecolor_factor[3] =
            static_cast<float>(gltf_material.pbrMetallicRoughness.baseColorFactor.at(3));

        material.data.pbr_metallic_roughness.basecolor_texture.index =
            gltf_material.pbrMetallicRoughness.baseColorTexture.index;
        material.data.pbr_metallic_roughness.basecolor_texture.texcoord =
            gltf_material.pbrMetallicRoughness.baseColorTexture.texCoord;

        material.data.pbr_metallic_roughness.metallic_factor =
            static_cast<float>(gltf_material.pbrMetallicRoughness.metallicFactor);
        material.data.pbr_metallic_roughness.roughness_factor =
            static_cast<float>(gltf_material.pbrMetallicRoughness.roughnessFactor);

        material.data.pbr_metallic_roughness.metallic_roughness_texture.index =
            gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index;
        material.data.pbr_metallic_roughness.metallic_roughness_texture.texcoord =
            gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.texCoord;

        material.data.normal_texture.index = gltf_material.normalTexture.index;
        material.data.normal_texture.texcoord = gltf_material.normalTexture.texCoord;
        material.data.normal_texture.scale = static_cast<float>(gltf_material.normalTexture.scale);

        material.data.occlusion_texture.index = gltf_material.occlusionTexture.index;
        material.data.occlusion_texture.texcoord = gltf_material.occlusionTexture.texCoord;
        material.data.occlusion_texture.strength =
            static_cast<float>(gltf_material.occlusionTexture.strength);

        material.data.emissive_texture.index = gltf_material.emissiveTexture.index;
        material.data.emissive_texture.texcoord = gltf_material.emissiveTexture.texCoord;
    }

    //GPU上でマテリアルデータをシェーダリソースビューとして作成
    std::vector<material::cbuffer> material_data;
    for (std::vector<material>::const_reference material : materials)
    {
        material_data.emplace_back(material.data);
    }

    HRESULT hr;
    // マテリアルデータをバッファとして作成
    Microsoft::WRL::ComPtr<ID3D11Buffer> material_buffer;
    D3D11_BUFFER_DESC buffer_desc{};
    buffer_desc.ByteWidth = static_cast<UINT>(sizeof(material::cbuffer) * material_data.size());
    buffer_desc.StructureByteStride = sizeof(material::cbuffer);
    buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    D3D11_SUBRESOURCE_DATA subresource_data{};
    subresource_data.pSysMem = material_data.data();
    hr = device->CreateBuffer(&buffer_desc, &subresource_data, material_buffer.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    // マテリアルデータをシェーダリソースビューとして作成
    D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc{};
    shader_resource_view_desc.Format = DXGI_FORMAT_UNKNOWN;
    shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    shader_resource_view_desc.Buffer.NumElements = static_cast<UINT>(material_data.size());
    hr = device->CreateShaderResourceView(material_buffer.Get(),
        &shader_resource_view_desc, material_resource_view.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

}

void ModelResource::fetch_textures(ID3D11Device* device, const tinygltf::Model& gltf_model)
{
    HRESULT hr{ S_OK };
    // glTFモデルからテクスチャを取得
    for (const tinygltf::Texture& gltf_texture : gltf_model.textures)
    {
        texture& texture{ textures.emplace_back() };
        texture.name = gltf_texture.name;
        texture.source = gltf_texture.source;
    }
    // glTFモデルからイメージを取得
    for (const tinygltf::Image& gltf_image : gltf_model.images)
    {
        image& image{ images.emplace_back() };
        image.name = gltf_image.name;
        image.width = gltf_image.width;
        image.height = gltf_image.height;
        image.component = gltf_image.component;
        image.bits = gltf_image.bits;
        image.pixel_type = gltf_image.pixel_type;
        image.buffer_view = gltf_image.bufferView;
        image.mime_type = gltf_image.mimeType;
        image.uri = gltf_image.uri;
        image.as_is = gltf_image.as_is;

        // イメージがバッファビューを持っている場合
        if (gltf_image.bufferView > -1)
        {
            const tinygltf::BufferView& buffer_view{
                gltf_model.bufferViews.at(gltf_image.bufferView) };
            const tinygltf::Buffer& buffer{
                gltf_model.buffers.at(buffer_view.buffer) };
            const byte* data = buffer.data.data() + buffer_view.byteOffset;

            // メモリからテクスチャをロード
            ID3D11ShaderResourceView* texture_resource_view{};
            hr = load_texture_from_memory(
                device, data, buffer_view.byteLength,
                &texture_resource_view);
            if (hr == S_OK)
            {
                texture_resource_views.emplace_back().Attach(
                    texture_resource_view);
            }
        }
        else // イメージがファイルを指す場合
        {
            const std::filesystem::path path(filename);
            ID3D11ShaderResourceView* shader_resource_view{};
            D3D11_TEXTURE2D_DESC texture2d_desc;
            std::wstring filename{
                path.parent_path().concat(L"/").wstring() +
                std::wstring(gltf_image.uri.begin(), gltf_image.uri.end()) };

            // ファイルからテクスチャをロード
            hr = load_texture_from_file(device, filename.c_str(),
                &shader_resource_view, &texture2d_desc);

            if (hr == S_OK)
            {
                texture_resource_views.emplace_back().Attach(
                    shader_resource_view);
            }
        }
    }
}

void ModelResource::fetch_animations(const tinygltf::Model& gltf_model)
{
    using namespace std;
    using namespace tinygltf;
    using namespace DirectX;

    // スキンの処理
    for (vector<Skin>::const_reference transmission_skin : gltf_model.skins)
    {
        // スキンの追加
        skin& skin{ skins.emplace_back() };

        // インバースバインド行列の取得
        const Accessor& gltf_accessor{ gltf_model.accessors.at(transmission_skin.inverseBindMatrices) };
        const BufferView& gltf_buffer_view{ gltf_model.bufferViews.at(gltf_accessor.bufferView) };
        skin.inverse_bind_matrices.resize(gltf_accessor.count);
        memcpy(skin.inverse_bind_matrices.data(), gltf_model.buffers.at(gltf_buffer_view.buffer).data.data() +
            gltf_buffer_view.byteOffset + gltf_accessor.byteOffset, gltf_accessor.count * sizeof(XMFLOAT4X4));
        skin.joints = transmission_skin.joints;
    }

    // アニメーションの処理
    for (vector<Animation>::const_reference gltf_animation : gltf_model.animations)
    {
        // アニメーションの追加
        animation& animation{ animations.emplace_back() };
        animation.name = gltf_animation.name;
        // サンプラーの処理
        for (vector<AnimationSampler>::const_reference gltf_sampler : gltf_animation.samplers)
        {
            // サンプラーの追加
            animation::sampler& sampler{ animation.samplers.emplace_back() };
            sampler.input = gltf_sampler.input;
            sampler.output = gltf_sampler.output;
            sampler.interpolation = gltf_sampler.interpolation;

            // 入力アクセサの取得
            const Accessor& gltf_accessor{ gltf_model.accessors.at(gltf_sampler.input) };
            const BufferView& gltf_buffer_view{ gltf_model.bufferViews.at(gltf_accessor.bufferView) };

            // タイムラインの処理
            pair<unordered_map<int, vector<float>>::iterator, bool>& timelines{
                animation.timelines.emplace(gltf_sampler.input, gltf_accessor.count) };
            if (timelines.second)
            {
                memcpy(timelines.first->second.data(), gltf_model.buffers.at(gltf_buffer_view.buffer).data.data() +
                    gltf_buffer_view.byteOffset + gltf_accessor.byteOffset, gltf_accessor.count * sizeof(FLOAT));
            }
        }
        // チャンネルの処理
        for (vector<AnimationChannel>::const_reference gltf_channel : gltf_animation.channels)
        {
            // チャンネルの追加
            animation::channel& channel{ animation.channels.emplace_back() };
            channel.sampler = gltf_channel.sampler;
            channel.target_node = gltf_channel.target_node;
            channel.target_path = gltf_channel.target_path;

            // 出力アクセサの取得
            const AnimationSampler& gltf_sampler{ gltf_animation.samplers.at(gltf_channel.sampler) };
            const Accessor& gltf_accessor{ gltf_model.accessors.at(gltf_sampler.output) };
            const BufferView& gltf_buffer_view{ gltf_model.bufferViews.at(gltf_accessor.bufferView) };

            // パスに基づいた処理
            if (gltf_channel.target_path == "scale")
            {
                pair<unordered_map<int, vector<XMFLOAT3>>::iterator, bool>& scales{
                    animation.scales.emplace(gltf_sampler.output, gltf_accessor.count) };
                if (scales.second)
                {
                    memcpy(scales.first->second.data(), gltf_model.buffers.at(gltf_buffer_view.buffer).data.data() +
                        gltf_buffer_view.byteOffset + gltf_accessor.byteOffset, gltf_accessor.count * sizeof(XMFLOAT3));
                }
            }
            else if (gltf_channel.target_path == "rotation")
            {
                pair<unordered_map<int, vector<XMFLOAT4>>::iterator, bool>& rotations{
                    animation.rotations.emplace(gltf_sampler.output, gltf_accessor.count) };
                if (rotations.second)
                {
                    memcpy(rotations.first->second.data(), gltf_model.buffers.at(gltf_buffer_view.buffer).data.data() +
                        gltf_buffer_view.byteOffset + gltf_accessor.byteOffset, gltf_accessor.count * sizeof(XMFLOAT4));
                }
            }
            else if (gltf_channel.target_path == "translation")
            {
                pair<unordered_map<int, vector<XMFLOAT3>>::iterator, bool>& translations{
                    animation.translations.emplace(gltf_sampler.output, gltf_accessor.count) };
                if (translations.second)
                {
                    memcpy(translations.first->second.data(), gltf_model.buffers.at(gltf_buffer_view.buffer).data.data() +
                        gltf_buffer_view.byteOffset + gltf_accessor.byteOffset, gltf_accessor.count * sizeof(XMFLOAT3));
                }
            }
        }
    }
}

void ModelResource::cumulate_transforms(std::vector<node>& nodes)
{
    using namespace DirectX;

    // 親のグローバル変換行列を管理するスタック
    std::stack<XMFLOAT4X4> parent_global_transforms;

    // ノードを再帰的にトラバースしてグローバル変換行列を累積するラムダ関数
    std::function<void(int)> traverse{ [&](int node_index)->void
    {
            // 現在のノード
            node& node{nodes.at(node_index)};

            // ノードのローカル変換行列（スケーリング、回転、平行移動）を計算
            XMMATRIX S{ XMMatrixScaling(node.scale.x, node.scale.y, node.scale.z) };
            XMMATRIX R{ XMMatrixRotationQuaternion(
                XMVectorSet(node.rotation.x, node.rotation.y, node.rotation.z, node.rotation.w)) };
            XMMATRIX T{ XMMatrixTranslation(node.translation.x, node.translation.y, node.translation.z) };

            // ノードのローカル変換行列を親のグローバル変換行列に乗じて、グローバル変換行列を計算
            XMStoreFloat4x4(&node.global_transform, S * R * T * XMLoadFloat4x4(&parent_global_transforms.top()));

            // 子ノードを処理するためにスタックに現在のグローバル変換行列をプッシュ
            for (int child_index : node.children)
            {

                parent_global_transforms.push(node.global_transform);
                traverse(child_index);
                parent_global_transforms.pop();
            }
    } };

    // シーンのルートノードからトラバースを開始
    for (std::vector<int>::value_type node_index : scenes.at(0).nodes)
    {
        // ルートノードの初期のグローバル変換行列をスタックにプッシュ
        parent_global_transforms.push({ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 });
        // トラバースを開始
        traverse(node_index);

        // ルートノードのグローバル変換行列をスタックからポップ
        parent_global_transforms.pop();
    }

}

void ModelResource::animate(size_t animation_index, float time, std::vector<node>& animated_nodes, bool loopback)
{
    using namespace std;
    using namespace DirectX;

    // インターポレーションに基づいてタイムラインのインデックスを取得するラムダ関数
    function<size_t(const vector<float>&, float, float&, bool)> indexof{
        [](const vector<float>& timelines, float time, float& interpolation_factor, bool loopback)->size_t
        { const size_t keyframe_count{timelines.size() };
    // ループバックが有効で、時間が最後のキーフレームを超えている場合は時間を調整する
    if (time > timelines.at(keyframe_count - 1))
    {
        if (loopback)
        {
            time = fmodf(time, timelines.at(keyframe_count - 1));
        }
        else
        {
            // ループバックが無効で、最後のキーフレームを超えている場合は最後のキーフレームを設定する
            interpolation_factor = 1.0f;
            return keyframe_count - 2;
        }
    }
    // 時間が最初のキーフレームよりも小さい場合は最初のキーフレームを設定する
    else if (time < timelines.at(0))
    {
        interpolation_factor = 0.0f;
        return 0;
    }

    // タイムラインを走査して適切なキーフレームのインデックスを見つける
    size_t keyframe_index{ 0 };
    for (size_t time_index = 1; time_index < keyframe_count; ++time_index)
    {
        if (time < timelines.at(time_index))
        {
            keyframe_index = max<size_t>(0LL, time_index - 1);
            break;
        }
    }

    // インターポレーションファクターを計算する
    interpolation_factor = (time - timelines.at(keyframe_index + 0)) /
        (timelines.at(keyframe_index + 1) - timelines.at(keyframe_index + 0));
    return keyframe_index;
}
    };

    if (animations.size() > 0)
    {
        const animation& animation{ animations.at(animation_index) };
        // アニメーションの各チャンネルを処理する
        for (vector<animation::channel>::const_reference channel : animation.channels)
        {
            const animation::sampler& sampler{ animation.samplers.at(channel.sampler) };
            const vector<float>& timeline{ animation.timelines.at(sampler.input) };
            // タイムラインが空であればスキップする
            if (timeline.size() == 0)
            {
                continue;
            }
            float interpolation_factor{};
            size_t keyframe_index{ indexof(timeline, time, interpolation_factor, loopback) };

            // ターゲットのパスに基づいてアニメーションを適用する
            if (channel.target_path == "scale")
            {
                const vector<XMFLOAT3>& scales{ animation.scales.at(sampler.output) };
                XMStoreFloat3(&animated_nodes.at(channel.target_node).scale,
                    XMVectorLerp(XMLoadFloat3(&scales.at(keyframe_index + 0)),
                        XMLoadFloat3(&scales.at(keyframe_index + 1)), interpolation_factor));
            }
            else if (channel.target_path == "rotation")
            {
                const vector<XMFLOAT4>& rotations{ animation.rotations.at(sampler.output) };
                XMStoreFloat4(&animated_nodes.at(channel.target_node).rotation,
                    XMQuaternionNormalize(XMQuaternionSlerp(XMLoadFloat4(&rotations.at(keyframe_index + 0)),
                        XMLoadFloat4(&rotations.at(keyframe_index + 1)), interpolation_factor)));
            }
            else if (channel.target_path == "translation")
            {
                const vector<XMFLOAT3>& translations{ animation.translations.at(sampler.output) };
                XMStoreFloat3(&animated_nodes.at(channel.target_node).translation,
                    XMVectorLerp(XMLoadFloat3(&translations.at(keyframe_index + 0)),
                        XMLoadFloat3(&translations.at(keyframe_index + 1)), interpolation_factor));
            }
        }
        // アニメーションの変換を累積する
        cumulate_transforms(animated_nodes);
    }
    else
    {
        // アニメーションがない場合は、ノードをそのまま使用する
        animated_nodes = nodes;
    }
}


ModelResource::buffer_view ModelResource::make_buffer_view(const tinygltf::Accessor& accessor)
{
    buffer_view buffer_view;
    // Accessorのタイプに基づいてバッファビューを作成する
    switch (accessor.type)
    {
    case TINYGLTF_TYPE_SCALAR:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            buffer_view.format = DXGI_FORMAT_R16_UINT;
            buffer_view.stride_in_bytes = sizeof(USHORT);
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            buffer_view.format = DXGI_FORMAT_R32_UINT;
            buffer_view.stride_in_bytes = sizeof(UINT);
            break;
        default:
            _ASSERT_EXPR(FALSE,
                L"This accessor component type is not supported.");
            break;
        }
        break;
    case TINYGLTF_TYPE_VEC2:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            buffer_view.format = DXGI_FORMAT_R32G32_FLOAT;
            buffer_view.stride_in_bytes = sizeof(FLOAT) * 2;
            break;
        default:
            _ASSERT_EXPR(FALSE,
                L"This accessor component type is not supported.");
            break;
        }
        break;
    case TINYGLTF_TYPE_VEC3:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            buffer_view.format = DXGI_FORMAT_R32G32B32_FLOAT;
            buffer_view.stride_in_bytes = sizeof(FLOAT) * 3;
            break;
        default:
            _ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
            break;
        }
        break;
    case TINYGLTF_TYPE_VEC4:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            buffer_view.format = DXGI_FORMAT_R16G16B16A16_UINT;
            buffer_view.stride_in_bytes = sizeof(USHORT) * 4;
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            buffer_view.format = DXGI_FORMAT_R32G32B32A32_UINT;
            buffer_view.stride_in_bytes = sizeof(UINT) * 4;
            break;
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            buffer_view.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            buffer_view.stride_in_bytes = sizeof(FLOAT) * 4;
            break;
        default:
            _ASSERT_EXPR(FALSE,
                L"This accessor component type is not supported.");
            break;
        }
        break;
    default:
        _ASSERT_EXPR(FALSE,
            L"This accessor type is not supported.");
        break;
    }
    // バッファビューのサイズを計算
    buffer_view.size_in_bytes = static_cast<UINT>(accessor.count * buffer_view.stride_in_bytes);
    return buffer_view;
}


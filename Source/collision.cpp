#include "collision.h"
#include "Graphics.h"
#include "user.h"
#include "operators.h"

bool Collision::HitCheckCircle(const DirectX::XMFLOAT2& pos1, float r1, const DirectX::XMFLOAT2& pos2, float r2)
{
    float x = pos1.x - pos2.x;
    float y = pos1.y - pos2.y;
    float r = r1 + r2;

    return (x * x) + (y * y) <= (r * r);
}

bool Collision::HitCheckRect(const DirectX::XMFLOAT2& center_a, const DirectX::XMFLOAT2& radius_a, const DirectX::XMFLOAT2& center_b, const DirectX::XMFLOAT2& radius_b)
{
    const DirectX::XMFLOAT2 dis = { center_a.x - center_b.x, center_a.y - center_b.y };  //���S�̍������߂�
    const DirectX::XMFLOAT2 center_dif = { radius_a.x + radius_b.x, radius_a.y + radius_b.y };

    return abs(dis.x) <= center_dif.x && abs(dis.y) <= center_dif.y;
}

bool Collision::SphereVsSphere(const DirectX::XMFLOAT3& center_a, float radius_a, const DirectX::XMFLOAT3& center_b, float radius_b, DirectX::XMFLOAT3* out_center_b)
{
    using namespace DirectX;
    //debug_figure->create_sphere(center_a, radius_a, { 0,0,1,1 });
    //debug_figure->create_sphere(center_b, radius_b, { 1,0,0,1 });

    // B �� A �̒P�ʃx�N�g�����Z�o
    XMVECTOR position_a_vec = XMLoadFloat3(&center_a);
    XMVECTOR position_b_vec = XMLoadFloat3(&center_b);
    XMVECTOR vec = XMVectorSubtract(position_b_vec, position_a_vec);
    XMVECTOR length_sq_vec = XMVector3LengthSq(vec);   // XMVector3LengthSq()�͓�悳�ꂽ���̂��o��(sq��squad�̈�)
    float length_sq;
    XMStoreFloat(&length_sq, length_sq_vec);

    // ��������
    float range = radius_a + radius_b;
    if (length_sq >= range * range) return false;

    // A��B�������o��
    XMVECTOR norm_sq_vec = XMVector3Normalize(vec);
    XMVECTOR out_position_add = XMVectorScale(norm_sq_vec, range);
    XMVECTOR out_position_b_vec = XMVectorAdd(position_a_vec, out_position_add);

    XMStoreFloat3(out_center_b, out_position_b_vec);

    return true;
}

bool Collision::CylinderVsCylinder(const DirectX::XMFLOAT3& position_a, float radius_a, float height_a, const DirectX::XMFLOAT3& position_b, float radius_b, float height_b, DirectX::XMFLOAT3* out_position_b)

{
    using namespace DirectX;
    //debug_figure->create_cylinder(position_a, radius_a, height_a, { 0,0,1,1 });
    //debug_figure->create_cylinder(position_b, radius_b, height_b, { 1,0,0,1 });

    // A�̑�����B�̓�����Ȃ瓖�����ĂȂ�
    if (position_a.y > position_b.y + height_b) { return false; }
    // A�̓���B�̑�����艺�Ȃ瓖�����ĂȂ�
    if (position_a.y + height_a < position_b.y) { return false; }
    // XZ���ʂł͈̔̓`�F�b�N
    float x = position_a.x - position_b.x;
    float z = position_a.z - position_b.z;
    float range = radius_a + radius_b;
    if ((x * x) + (z * z) >= (range * range)) return false;
    // A��B�������o��
    XMFLOAT3 positionA = { position_a.x, 0, position_a.z };
    XMFLOAT3 positionB = { position_b.x, 0, position_b.z };
    // XMVECTOR�ɕϊ�
    XMVECTOR position_a_vec = XMLoadFloat3(&positionA);
    XMVECTOR position_b_vec = XMLoadFloat3(&positionB);
    // �|�W�V����A�ƃ|�W�V����B�̍��̃x�N�g�������߂�
    XMVECTOR vec = XMVectorSubtract(position_b_vec, position_a_vec);
    // ���K��
    XMVECTOR norm_sq_vec = XMVector3Normalize(vec);
    XMVECTOR out_position_add = XMVectorScale(norm_sq_vec, range);
    XMVECTOR out_position_b_vec = XMVectorAdd(position_a_vec, out_position_add);

    XMFLOAT3 out_positionB;
    XMStoreFloat3(&out_positionB, out_position_b_vec);

    out_position_b->x = out_positionB.x;
    out_position_b->y = position_b.y;
    out_position_b->z = out_positionB.z;

    return true;
}

bool Collision::SphereVsCylinder(const DirectX::XMFLOAT3& sphere_position, float sphere_radius, const DirectX::XMFLOAT3& cylinder_position, float cylinder_radius, float cylinder_height, DirectX::XMFLOAT3* out_cylinder_position)
{
    using namespace DirectX;
    // A�̋��̉���B�̓�����Ȃ瓖�����ĂȂ�
    if (sphere_position.y - sphere_radius > cylinder_position.y + cylinder_height) { return false; }
    //  A�̋��̏オB�̑�����艺�Ȃ瓖�����ĂȂ�
    if (sphere_position.y + sphere_radius < cylinder_position.y) { return false; }

    // XZ���ʂł͈̔̓`�F�b�N
    float x = sphere_position.x - cylinder_position.x;
    float z = sphere_position.z - cylinder_position.z;
    float range = sphere_radius + cylinder_radius;
    if ((x * x) + (z * z) >= (range * range)) return false;

    // A��B�������o��
    XMFLOAT3 S_position = { sphere_position.x, 0, sphere_position.z };
    XMFLOAT3 C_position = { cylinder_position.x, 0, cylinder_position.z };
    
    // XMVECTOR�ɕϊ�
    XMVECTOR s_position_vec = XMLoadFloat3(&S_position);
    XMVECTOR c_position_vec = XMLoadFloat3(&C_position);
    
    // �|�W�V����S�ƃ|�W�V����C�̍��̃x�N�g�������߂�
    XMVECTOR vec = XMVectorSubtract(c_position_vec, s_position_vec);
   
    // ���K��
    XMVECTOR norm_sq_vec = XMVector3Normalize(vec);
    XMVECTOR out_position_add = XMVectorScale(norm_sq_vec, range);
    XMVECTOR out_c_position_vec = XMVectorAdd(s_position_vec, out_position_add);

    XMFLOAT3 out_c_position;
    XMStoreFloat3(&out_c_position, out_c_position_vec);

    out_cylinder_position->x = out_c_position.x;
    out_cylinder_position->y = cylinder_position.y;
    out_cylinder_position->z = out_c_position.z;

    return true;
}

bool Collision::CuboidVsCuboid(const DirectX::XMFLOAT3& center_a, const DirectX::XMFLOAT3& radius_a, const DirectX::XMFLOAT3& center_b, const DirectX::XMFLOAT3& radius_b, DirectX::XMFLOAT3* velocity_b)
{
    using namespace DirectX;
    //debug_figure->create_cuboid(center_a, radius_a, { 0,0,1,1 });
    //debug_figure->create_cuboid(center_b, radius_b, { 1,0,0,1 });

    // �e����position,radius���Z�o
    /*cuboid a*/
    // pos
    XMFLOAT2 pos_a_xy = { center_a.x, center_a.y };
    XMFLOAT2 pos_a_xz = { center_a.x, center_a.z };
    XMFLOAT2 pos_a_yz = { center_a.y, center_a.z };
    // radius
    XMFLOAT2 radius_a_xy = { radius_a.x, radius_a.y };
    XMFLOAT2 radius_a_xz = { radius_a.x, radius_a.z };
    XMFLOAT2 radius_a_yz = { radius_a.y, radius_a.z };
    /*cuboid b*/
    
    // pos
    XMFLOAT2 pos_b_xy = { center_b.x, center_b.y };
    XMFLOAT2 pos_b_xz = { center_b.x, center_b.z };
    XMFLOAT2 pos_b_yz = { center_b.y, center_b.z };
    // radius
    XMFLOAT2 radius_b_xy = { radius_b.x, radius_b.y };
    XMFLOAT2 radius_b_xz = { radius_b.x, radius_b.z };
    XMFLOAT2 radius_b_yz = { radius_b.y, radius_b.z };
    /*arrival cuboid b*/
    
    // pos
    XMFLOAT3 arrival_position = { center_b.x + velocity_b->x, center_b.y + velocity_b->y, center_b.z + velocity_b->z };
    XMFLOAT2 arrival_pos_b_xy = { arrival_position.x, arrival_position.y };
    XMFLOAT2 arrival_pos_b_xz = { arrival_position.x, arrival_position.z };
    XMFLOAT2 arrival_pos_b_yz = { arrival_position.y, arrival_position.z };
    
    // radius
    XMFLOAT3 arrival_radius = { radius_b.x + fabsf(arrival_position.x - center_b.x + FLT_EPSILON),
        radius_b.y + fabsf(arrival_position.y - center_b.y + FLT_EPSILON), radius_b.z + fabsf(arrival_position.z - center_b.z + FLT_EPSILON) };
    XMFLOAT2 arrival_radius_b_xy = { arrival_radius.x, arrival_radius.y };
    XMFLOAT2 arrival_radius_b_xz = { arrival_radius.x, arrival_radius.z };
    XMFLOAT2 arrival_radius_b_yz = { arrival_radius.y, arrival_radius.z };
    //debug_figure->create_cuboid(arrival_position, arrival_radius, { 0,1,0,1 });

    //----��������----//
    /*cuboid a vs cuboid b*/
    if (!HitCheckRect(pos_a_xy, radius_a_xy, pos_b_xy, radius_b_xy)) return false;
    if (!HitCheckRect(pos_a_xz, radius_a_xz, pos_b_xz, radius_b_xz)) return false;
    if (!HitCheckRect(pos_a_yz, radius_a_yz, pos_b_yz, radius_b_yz)) return false;
    
    /*cuboid a vs arrival cuboid b*/
    if (!HitCheckRect(pos_a_xy, radius_a_xy, arrival_pos_b_xy, radius_b_xy)) return false;
    if (!HitCheckRect(pos_a_xz, radius_a_xz, arrival_pos_b_xz, radius_b_xz)) return false;
    if (!HitCheckRect(pos_a_yz, radius_a_yz, arrival_pos_b_yz, radius_b_yz)) return false;

    return true;
}

bool Collision::FrustumVsCuboid(DirectX::XMFLOAT4X4 camara_view, DirectX::XMFLOAT4X4 camara_proj, const DirectX::XMFLOAT3& cuboid_min_pos, const DirectX::XMFLOAT3& cuboid_max_pos)
{
    using namespace DirectX;
    
    //----- �����̂̃p�����[�^�[ -----//
    XMFLOAT3 cuboid_radius = { (cuboid_max_pos.x - cuboid_min_pos.x) / 2, (cuboid_max_pos.y - cuboid_min_pos.y) / 2, (cuboid_max_pos.z - cuboid_min_pos.z) / 2 };
    XMFLOAT3 cuboid_center = { cuboid_max_pos.x - cuboid_radius.x, cuboid_max_pos.y - cuboid_radius.y, cuboid_max_pos.z - cuboid_radius.z };
    //debug_figure->create_cuboid(cuboid_center, cuboid_radius, { 1,1,0,1 });
    
    //----- ������̃p�����[�^�[ -----//
    // �r���[�v���W�F�N�V�����s����擾����
    XMMATRIX matrix = {};
    XMFLOAT4X4 view = camara_view;
    XMMATRIX view_mat = XMLoadFloat4x4(&view);
    XMFLOAT4X4 proj = camara_proj;
    XMMATRIX proj_mat = XMLoadFloat4x4(&proj);
    matrix = view_mat * proj_mat;
    
    //�r���[�v���W�F�N�V�����s��̋t�s��
    XMMATRIX inv_matrix = XMMatrixInverse(nullptr, matrix);
    
    //�r���[�v���W�F�N�V�������̒��_�Z�o�p�ʒu�x�N�g��
    XMVECTOR verts[8] =
    {
        // near plane corners
        { -1, -1, 0 },	// [0]:����
        {  1, -1, 0 },	// [1]:�E��
        {  1,  1, 0 },	// [2]:�E��
        { -1,  1 ,0 },	// [3]:����

        // far plane corners.
        { -1, -1, 1 },	// [4]:����
        {  1, -1, 1 },	// [5]:�E��
        {  1,  1, 1 },	// [6]:�E��
        { -1,  1, 1 } 	// [7]:����
    };
    
    // �r���[�v���W�F�N�V�����s��̋t�s���p���āA�e���_���Z�o����
    XMFLOAT3 near_p[4] = {};	// Near�̎l�p�`�̂S���_�̍��W
    XMFLOAT3 far_p[4] = {}; 	// Far�̎l�p�`�̂S���_�̍��W
    for (int i = 0; i < 4; ++i) // near
    {
        verts[i] = XMVector3TransformCoord(verts[i], inv_matrix);
        XMStoreFloat3(&near_p[i], verts[i]);
    }
    for (int i = 4; i < 8; ++i) // far
    {
        verts[i] = XMVector3TransformCoord(verts[i], inv_matrix);
        XMStoreFloat3(&far_p[i - 4], verts[i]);
    }
    
    // ������i�t���X�^���j���\������U���ʂ��Z�o����
    XMFLOAT4X4 matrix4X4 = {};
    XMStoreFloat4x4(&matrix4X4, matrix);
    
    // 0:������, 1:�E����, 2:������, 3:�㑤��, 4:������, 5:��O����
    // �S�Ă̖ʂ̖@���͓����������悤�ɐݒ肷�邱��
    XMFLOAT3 normals[6];	// �@��
    float	 distances[6];	// ���_����̍ŒZ����
    // ������
    {
        normals[0].x = { matrix4X4._14 + matrix4X4._11 };
        normals[0].y = { matrix4X4._24 + matrix4X4._21 };
        normals[0].z = { matrix4X4._34 + matrix4X4._31 };
        XMVECTOR n_vec = XMVector3Normalize(XMLoadFloat3(&normals[0]));
        XMStoreFloat3(&normals[0], n_vec);

        XMVECTOR p_vec = XMLoadFloat3(&near_p[0]);
        XMStoreFloat(&distances[0], XMVector3Dot(p_vec, n_vec));
    }
    // �E����
    {
        normals[1].x = { matrix4X4._14 - matrix4X4._11 };
        normals[1].y = { matrix4X4._24 - matrix4X4._21 };
        normals[1].z = { matrix4X4._34 - matrix4X4._31 };
        XMVECTOR n_vec = XMVector3Normalize(XMLoadFloat3(&normals[1]));
        XMStoreFloat3(&normals[1], n_vec);

        XMVECTOR p_vec = XMLoadFloat3(&near_p[1]);
        XMStoreFloat(&distances[1], XMVector3Dot(p_vec, n_vec));
    }
    // ������
    {
        normals[2].x = { matrix4X4._14 + matrix4X4._12 };
        normals[2].y = { matrix4X4._24 + matrix4X4._22 };
        normals[2].z = { matrix4X4._34 + matrix4X4._32 };
        XMVECTOR n_vec = XMVector3Normalize(XMLoadFloat3(&normals[2]));
        XMStoreFloat3(&normals[2], n_vec);

        XMVECTOR p_vec = XMLoadFloat3(&near_p[1]);
        XMStoreFloat(&distances[2], XMVector3Dot(p_vec, n_vec));
    }
    // �㑤��
    {
        normals[3].x = { matrix4X4._14 - matrix4X4._12 };
        normals[3].y = { matrix4X4._24 - matrix4X4._22 };
        normals[3].z = { matrix4X4._34 - matrix4X4._32 };
        XMVECTOR n_vec = XMVector3Normalize(XMLoadFloat3(&normals[3]));
        XMStoreFloat3(&normals[3], n_vec);

        XMVECTOR p_vec = XMLoadFloat3(&near_p[2]);
        XMStoreFloat(&distances[3], XMVector3Dot(p_vec, n_vec));
    }
    // ������
    {
        normals[4].x = { -matrix4X4._14 - matrix4X4._13 };
        normals[4].y = { -matrix4X4._24 - matrix4X4._23 };
        normals[4].z = { -matrix4X4._34 - matrix4X4._33 };
        XMVECTOR n_vec = XMVector3Normalize(XMLoadFloat3(&normals[4]));
        XMStoreFloat3(&normals[4], n_vec);

        XMVECTOR p_vec = XMLoadFloat3(&far_p[0]);
        XMStoreFloat(&distances[4], XMVector3Dot(p_vec, n_vec));
    }
    // ��O����
    {
        normals[5].x = { -matrix4X4._14 + matrix4X4._13 };
        normals[5].y = { -matrix4X4._24 + matrix4X4._23 };
        normals[5].z = { -matrix4X4._34 + matrix4X4._33 };
        XMVECTOR n_vec = XMVector3Normalize(XMLoadFloat3(&normals[5]));
        XMStoreFloat3(&normals[5], n_vec);

        XMVECTOR p_vec = XMLoadFloat3(&near_p[0]);
        XMStoreFloat(&distances[5], XMVector3Dot(p_vec, n_vec));
    }
    
    //-----��������-----//
    for (int i = 0; i < 6; i++)
    {
        // �e���ʂ̖@���̐�����p����AABB�̂W���_�̒�����ŋߓ_�ƍŉ��_�����߂�
        // �ŉ��_
        XMFLOAT3 posi_pos = cuboid_center;
        posi_pos.x += cuboid_radius.x * (normals[i].x / fabsf(normals[i].x + FLT_EPSILON));
        posi_pos.y += cuboid_radius.y * (normals[i].y / fabsf(normals[i].y + FLT_EPSILON));
        posi_pos.z += cuboid_radius.z * (normals[i].z / fabsf(normals[i].z + FLT_EPSILON));
        
        // �ŋߓ_
        XMFLOAT3 nega_pos = cuboid_center;
        nega_pos.x -= cuboid_radius.x * (normals[i].x / fabsf(normals[i].x + FLT_EPSILON));
        nega_pos.y -= cuboid_radius.y * (normals[i].y / fabsf(normals[i].y + FLT_EPSILON));
        nega_pos.z -= cuboid_radius.z * (normals[i].z / fabsf(normals[i].z + FLT_EPSILON));
        
        // �e���ʂƂ̓��ς��v�Z���A�����E���O����(�\������)���s��
        float posi_dot;
        XMStoreFloat(&posi_dot, XMVector3Dot(XMLoadFloat3(&normals[i]),
            XMLoadFloat3(&posi_pos)));
        float nega_dot;
        XMStoreFloat(&nega_dot, XMVector3Dot(XMLoadFloat3(&normals[i]),
            XMLoadFloat3(&nega_pos)));
        if (posi_dot >= distances[i] && nega_dot >= distances[i]) /*����*/ {}
        else if (posi_dot < distances[i] && nega_dot < distances[i]) /*�O��*/ { return false; }
        else /*����*/ {}
    }
    return true;
}

bool Collision::ForefrontFrustumVsCuboid(DirectX::XMFLOAT4X4 camara_view, float camera_range, const DirectX::XMFLOAT3& cuboid_min_pos, const DirectX::XMFLOAT3& cuboid_max_pos)
{
    using namespace DirectX;
    
    //----- �����̂̃p�����[�^�[ -----//
    XMFLOAT3 cuboid_radius = { (cuboid_max_pos.x - cuboid_min_pos.x) / 2, (cuboid_max_pos.y - cuboid_min_pos.y) / 2, (cuboid_max_pos.z - cuboid_min_pos.z) / 2 };
    XMFLOAT3 cuboid_center = { cuboid_max_pos.x - cuboid_radius.x, cuboid_max_pos.y - cuboid_radius.y, cuboid_max_pos.z - cuboid_radius.z };
    //debug_figure->create_cuboid(cuboid_center, cuboid_radius, { 1,1,0,1 });

    //----- ������̃p�����[�^�[ -----//
    // �r���[�v���W�F�N�V�����s����擾����
    XMMATRIX matrix = {};
    XMFLOAT4X4 view = camara_view;
    XMMATRIX view_mat = XMLoadFloat4x4(&view);

    float width = static_cast<float>(SCREEN_WIDTH);
    float height = static_cast<float>(SCREEN_HEIGHT);
    float aspect_ratio{ width / height };
    XMMATRIX proj_mat = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(45), aspect_ratio, 0.1f, camera_range * 0.9f); // P
    matrix = view_mat * proj_mat;
    
    //�r���[�v���W�F�N�V�����s��̋t�s��
    XMMATRIX inv_matrix = XMMatrixInverse(nullptr, matrix);
    
    //�r���[�v���W�F�N�V�������̒��_�Z�o�p�ʒu�x�N�g��
    XMVECTOR verts[8] =
    {
        // near plane corners
        { -1, -1, 0 },	// [0]:����
        {  1, -1, 0 },	// [1]:�E��
        {  1,  1, 0 },	// [2]:�E��
        { -1,  1 ,0 },	// [3]:����

        // far plane corners.
        { -1, -1, 1 },	// [4]:����
        {  1, -1, 1 },	// [5]:�E��
        {  1,  1, 1 },	// [6]:�E��
        { -1,  1, 1 } 	// [7]:����
    };
    
    // �r���[�v���W�F�N�V�����s��̋t�s���p���āA�e���_���Z�o����
    XMFLOAT3 near_p[4] = {};	// Near�̎l�p�`�̂S���_�̍��W
    XMFLOAT3 far_p[4] = {}; 	// Far�̎l�p�`�̂S���_�̍��W
    for (int i = 0; i < 4; ++i) // near
    {
        verts[i] = XMVector3TransformCoord(verts[i], inv_matrix);
        XMStoreFloat3(&near_p[i], verts[i]);
    }
    for (int i = 4; i < 8; ++i) // far
    {
        verts[i] = XMVector3TransformCoord(verts[i], inv_matrix);
        XMStoreFloat3(&far_p[i - 4], verts[i]);
    }
    
    // ������i�t���X�^���j���\������U���ʂ��Z�o����
    XMFLOAT4X4 matrix4X4 = {};
    XMStoreFloat4x4(&matrix4X4, matrix);
    
    // 0:������, 1:�E����, 2:������, 3:�㑤��, 4:������, 5:��O����
    // �S�Ă̖ʂ̖@���͓����������悤�ɐݒ肷�邱��
    XMFLOAT3 normals[6];	// �@��
    float	 distances[6];	// ���_����̍ŒZ����
    // ������
    {
        normals[0].x = { matrix4X4._14 + matrix4X4._11 };
        normals[0].y = { matrix4X4._24 + matrix4X4._21 };
        normals[0].z = { matrix4X4._34 + matrix4X4._31 };
        XMVECTOR n_vec = XMVector3Normalize(XMLoadFloat3(&normals[0]));
        XMStoreFloat3(&normals[0], n_vec);

        XMVECTOR p_vec = XMLoadFloat3(&near_p[0]);
        XMStoreFloat(&distances[0], XMVector3Dot(p_vec, n_vec));
    }
    // �E����
    {
        normals[1].x = { matrix4X4._14 - matrix4X4._11 };
        normals[1].y = { matrix4X4._24 - matrix4X4._21 };
        normals[1].z = { matrix4X4._34 - matrix4X4._31 };
        XMVECTOR n_vec = XMVector3Normalize(XMLoadFloat3(&normals[1]));
        XMStoreFloat3(&normals[1], n_vec);

        XMVECTOR p_vec = XMLoadFloat3(&near_p[1]);
        XMStoreFloat(&distances[1], XMVector3Dot(p_vec, n_vec));
    }
    // ������
    {
        normals[2].x = { matrix4X4._14 + matrix4X4._12 };
        normals[2].y = { matrix4X4._24 + matrix4X4._22 };
        normals[2].z = { matrix4X4._34 + matrix4X4._32 };
        XMVECTOR n_vec = XMVector3Normalize(XMLoadFloat3(&normals[2]));
        XMStoreFloat3(&normals[2], n_vec);

        XMVECTOR p_vec = XMLoadFloat3(&near_p[1]);
        XMStoreFloat(&distances[2], XMVector3Dot(p_vec, n_vec));
    }
    // �㑤��
    {
        normals[3].x = { matrix4X4._14 - matrix4X4._12 };
        normals[3].y = { matrix4X4._24 - matrix4X4._22 };
        normals[3].z = { matrix4X4._34 - matrix4X4._32 };
        XMVECTOR n_vec = XMVector3Normalize(XMLoadFloat3(&normals[3]));
        XMStoreFloat3(&normals[3], n_vec);

        XMVECTOR p_vec = XMLoadFloat3(&near_p[2]);
        XMStoreFloat(&distances[3], XMVector3Dot(p_vec, n_vec));
    }
    // ������
    {
        normals[4].x = { -matrix4X4._14 - matrix4X4._13 };
        normals[4].y = { -matrix4X4._24 - matrix4X4._23 };
        normals[4].z = { -matrix4X4._34 - matrix4X4._33 };
        XMVECTOR n_vec = XMVector3Normalize(XMLoadFloat3(&normals[4]));
        XMStoreFloat3(&normals[4], n_vec);

        XMVECTOR p_vec = XMLoadFloat3(&far_p[0]);
        XMStoreFloat(&distances[4], XMVector3Dot(p_vec, n_vec));
    }
    // ��O����
    {
        normals[5].x = { -matrix4X4._14 + matrix4X4._13 };
        normals[5].y = { -matrix4X4._24 + matrix4X4._23 };
        normals[5].z = { -matrix4X4._34 + matrix4X4._33 };
        XMVECTOR n_vec = XMVector3Normalize(XMLoadFloat3(&normals[5]));
        XMStoreFloat3(&normals[5], n_vec);

        XMVECTOR p_vec = XMLoadFloat3(&near_p[0]);
        XMStoreFloat(&distances[5], XMVector3Dot(p_vec, n_vec));
    }
    
    //-----��������-----//
    for (int i = 0; i < 6; i++)
    {
        // �e���ʂ̖@���̐�����p����AABB�̂W���_�̒�����ŋߓ_�ƍŉ��_�����߂�
        // �ŉ��_
        XMFLOAT3 posi_pos = cuboid_center;
        posi_pos.x += cuboid_radius.x * (normals[i].x / fabsf(normals[i].x + FLT_EPSILON));
        posi_pos.y += cuboid_radius.y * (normals[i].y / fabsf(normals[i].y + FLT_EPSILON));
        posi_pos.z += cuboid_radius.z * (normals[i].z / fabsf(normals[i].z + FLT_EPSILON));
       
        // �ŋߓ_
        XMFLOAT3 nega_pos = cuboid_center;
        nega_pos.x -= cuboid_radius.x * (normals[i].x / fabsf(normals[i].x + FLT_EPSILON));
        nega_pos.y -= cuboid_radius.y * (normals[i].y / fabsf(normals[i].y + FLT_EPSILON));
        nega_pos.z -= cuboid_radius.z * (normals[i].z / fabsf(normals[i].z + FLT_EPSILON));
        
        // �e���ʂƂ̓��ς��v�Z���A�����E���O����(�\������)���s��
        float posi_dot;
        XMStoreFloat(&posi_dot, XMVector3Dot(XMLoadFloat3(&normals[i]),
            XMLoadFloat3(&posi_pos)));
        float nega_dot;
        XMStoreFloat(&nega_dot, XMVector3Dot(XMLoadFloat3(&normals[i]),
            XMLoadFloat3(&nega_pos)));
        if (posi_dot >= distances[i] && nega_dot >= distances[i]) /*����*/ {}
        else if (posi_dot < distances[i] && nega_dot < distances[i]) /*�O��*/ { return false; }
        else /*����*/ {}
    }
    return true;
}

bool Collision::SphereVsCapsule(const DirectX::XMFLOAT3& sphere_center, float sphere_radius, const DirectX::XMFLOAT3& capsule_start, const DirectX::XMFLOAT3& capsule_end, float capsule_radius)
{
    using namespace DirectX;
    //debug_figure->create_sphere(sphere_center, sphere_radius, { 0,0,1,1 });
    //debug_figure->create_capsule(capsule_start, capsule_end, capsule_radius, { 1,0,0,1 });
    XMFLOAT3 cp_to_sp;
    cp_to_sp.x = sphere_center.x - capsule_start.x;
    cp_to_sp.y = sphere_center.y - capsule_start.y;
    cp_to_sp.z = sphere_center.z - capsule_start.z;
    XMVECTOR vec_cp_to_sp = XMLoadFloat3(&cp_to_sp);
    // �J�v�Z���̎n�_����I�_�ւ̃x�N�g��
    XMFLOAT3 cpstart_to_cpend;
    cpstart_to_cpend.x = capsule_end.x - capsule_start.x;
    cpstart_to_cpend.y = capsule_end.y - capsule_start.y;
    cpstart_to_cpend.z = capsule_end.z - capsule_start.z;
    XMVECTOR vec_cpstart_to_cpend = XMLoadFloat3(&cpstart_to_cpend);
    XMFLOAT3 cpstart_to_cpend_norm;
    XMStoreFloat3(&cpstart_to_cpend_norm, XMVector3Normalize(vec_cpstart_to_cpend));
    // �ˉe�x�N�g��
    float projection_vector_length;
    XMStoreFloat(&projection_vector_length, XMVector3Dot(vec_cp_to_sp, XMVector3Normalize(vec_cpstart_to_cpend)));

    float cpstart_to_cpend_length;
    XMStoreFloat(&cpstart_to_cpend_length, XMVector3Length(vec_cpstart_to_cpend));

    // �ŒZ�_
    XMFLOAT3 shortest_point;
    if (projection_vector_length < 0) // �ˉe�����}�C�i�X
    {
        shortest_point = capsule_start;
    }
    else if (projection_vector_length > cpstart_to_cpend_length) // �~�����̒����𒴂���
    {
        shortest_point = capsule_end;
    }
    else // �~�����̒����̒��ɂ���
    {
        shortest_point.x = capsule_start.x + cpstart_to_cpend_norm.x * projection_vector_length;
        shortest_point.y = capsule_start.y + cpstart_to_cpend_norm.y * projection_vector_length;
        shortest_point.z = capsule_start.z + cpstart_to_cpend_norm.z * projection_vector_length;
    }
    // ���ƃJ�v�Z���Ƃ̍ŒZ���������߂�
    XMFLOAT3 shortest_cp_to_sp;
    shortest_cp_to_sp.x = shortest_point.x - sphere_center.x;
    shortest_cp_to_sp.y = shortest_point.y - sphere_center.y;
    shortest_cp_to_sp.z = shortest_point.z - sphere_center.z;

    float length_sp_to_cp;
    XMStoreFloat(&length_sp_to_cp, XMVector3Length(XMLoadFloat3(&shortest_cp_to_sp)));

    if (length_sp_to_cp < (sphere_radius + capsule_radius)) { return true; }
    return false;
}

bool Collision::CapsuleVsCapsule(const DirectX::XMFLOAT3& start_a, const DirectX::XMFLOAT3& end_a, float radius_a, const DirectX::XMFLOAT3& start_b, const DirectX::XMFLOAT3& end_b, float radius_b)
{
    using namespace DirectX;
    //debug_figure->create_capsule(start_a, end_a, radius_a, { 0,0,1,1 });
    //debug_figure->create_capsule(start_b, end_b, radius_b, { 1,0,0,1 });
    // �J�v�Z��c0�̎n�_����I�_�ւ̃x�N�g��(d0)
    XMFLOAT3 d0;
    d0.x = end_a.x - start_a.x;
    d0.y = end_a.y - start_a.y;
    d0.z = end_a.z - start_a.z;
    XMVECTOR d0_vec = XMLoadFloat3(&d0);
    XMFLOAT3 d0_norm;
    XMStoreFloat3(&d0_norm, XMVector3Normalize(d0_vec));
    // �J�v�Z��c1�̎n�_����I�_�ւ̃x�N�g��(d1)
    XMFLOAT3 d1;
    d1.x = end_b.x - start_b.x;
    d1.y = end_b.y - start_b.y;
    d1.z = end_b.z - start_b.z;
    XMVECTOR d1_vec = XMLoadFloat3(&d1);
    XMFLOAT3 d1_norm;
    XMStoreFloat3(&d1_norm, XMVector3Normalize(d1_vec));
    // �J�v�Z��c1�̎n�_����J�v�Z��c0�̎n�_�ւ̃x�N�g��(r)
    XMFLOAT3 r;
    r.x = start_a.x - start_b.x;
    r.y = start_a.y - start_b.y;
    r.z = start_a.z - start_b.z;
    XMVECTOR r_vec = XMLoadFloat3(&r);
    // ��̒����Ɖ��肵�A�J�v�Z��c0��̍ŒZ�_���Z�o
    // |d0|
    float d0_length;
    XMStoreFloat(&d0_length, XMVector3Length(d0_vec));
    // |d1|
    float d1_length;
    XMStoreFloat(&d1_length, XMVector3Length(d1_vec));
    // d0�Ed1
    float d0_dot_d1;
    XMStoreFloat(&d0_dot_d1, XMVector3Dot(d0_vec, d1_vec));
    // d0�Er
    float d0_dot_r;
    XMStoreFloat(&d0_dot_r, XMVector3Dot(d0_vec, r_vec));
    // d1�Er
    float d1_dot_r;
    XMStoreFloat(&d1_dot_r, XMVector3Dot(d1_vec, r_vec));
    // d0�Ed0
    float d0_dot_d0;
    XMStoreFloat(&d0_dot_d0, XMVector3Dot(d0_vec, d0_vec));
    // d1�Ed1
    float d1_dot_d1;
    XMStoreFloat(&d1_dot_d1, XMVector3Dot(d1_vec, d1_vec));

    float t0_numer = (d0_dot_d1 * d1_dot_r) - (d0_dot_r * d1_dot_d1); // ���q
    float denom = (d0_dot_d0 * d1_dot_d1) - (d0_dot_d1 * d0_dot_d1); // ����

    float t0, t1;
    if (d0_dot_d0 < FLT_EPSILON && d1_dot_d1 < FLT_EPSILON) { t0 = 0; } // �������_
    else if (d0_dot_d0 < FLT_EPSILON) { t0 = 0; }  // d1�̂ݐ���
    else if (d1_dot_d1 < FLT_EPSILON) { t0 = -d0_dot_r / d0_dot_d0; } // d0�̂ݐ���
    else // ����������
    {
        t0 = t0_numer / denom;
        // ���s
        if (denom < FLT_EPSILON) { t0 = 0; }
    }
    t0 = Math::Clamp(t0, 0.0f, 1.0f);
    // t1�̎Z�o
    t1 = ((t0 * d0_dot_d1) + d1_dot_r) / d1_dot_d1;
    // d1��̍ŒZ�_
    XMFLOAT3 d1_shortest_point;
    if (t1 < 0)
    {
        d1_shortest_point = start_b;

        t0 = (-d0_dot_r) / d0_dot_d0;
        t0 = Math::Clamp(t0, 0.0f, 1.0f);
    }
    else if (t1 > 1)
    {
        d1_shortest_point = end_b;

        t0 = (d0_dot_d1 - d0_dot_r) / d0_dot_d0;
        t0 = Math::Clamp(t0, 0.0f, 1.0f);
    }
    else
    {
        d1_shortest_point.x = start_b.x + d1_norm.x * t1 * d1_length;
        d1_shortest_point.y = start_b.y + d1_norm.y * t1 * d1_length;
        d1_shortest_point.z = start_b.z + d1_norm.z * t1 * d1_length;
    }
    // d0��̍ŒZ�_
    XMFLOAT3 d0_shortest_point;
    if (t0 <= 0) { d0_shortest_point = start_a; }
    else if (t0 >= 1) { d0_shortest_point = end_a; }
    else
    {
        d0_shortest_point.x = start_a.x + d0_norm.x * t0 * d0_length;
        d0_shortest_point.y = start_a.y + d0_norm.y * t0 * d0_length;
        d0_shortest_point.z = start_a.z + d0_norm.z * t0 * d0_length;
    }
    // �J�v�Z��c0�ƃJ�v�Z��c1�Ƃ̍ŒZ���������߂�
    XMFLOAT3 shortest_cp_to_cp;
    shortest_cp_to_cp.x = d0_shortest_point.x - d1_shortest_point.x;
    shortest_cp_to_cp.y = d0_shortest_point.y - d1_shortest_point.y;
    shortest_cp_to_cp.z = d0_shortest_point.z - d1_shortest_point.z;

    float length_cp_to_cp;
    XMStoreFloat(&length_cp_to_cp, XMVector3Length(XMLoadFloat3(&shortest_cp_to_cp)));

    if (length_cp_to_cp < (radius_a + radius_b)) { return true; }
    return false;
}

bool Collision::RayVsModel(const DirectX::XMFLOAT3& start,
    const DirectX::XMFLOAT3& end,
    const gltf_model* model,
    const DirectX::XMFLOAT4X4 model_world_mat, 
    HitResult& result)
{
    using namespace DirectX;

    bool hit = false;
    if (model->collisionMesh->RayCast(start, end,
        model_world_mat, result
        //,Math::calc_vector_AtoB_length(start, end))
        ))
    {
        hit = true;
    }

    return hit;
}

bool Collision::RingVsCapsule(
    const DirectX::XMFLOAT3& center_ring_position,
    float ring_radius, float ring_width, float ring_height, 
    const DirectX::XMFLOAT3& capsule_start,
    const DirectX::XMFLOAT3& capsule_end, float capsule_radius)
{
    //debug_figure->create_cylinder(center_ring_position, ring_radius + ring_width, ring_height, { 0,0,1,1 });
    //debug_figure->create_cylinder(center_ring_position, ring_radius - ring_width, ring_height, { 0,0,0.7f,1 });
    //debug_figure->create_capsule(capsule_start, capsule_end, capsule_radius, { 1,0,0,1 });

    //�㉺�̔���
    if (center_ring_position.y + ring_height < capsule_start.y) return false;
    if (center_ring_position.y - ring_height > capsule_end.y) return false;

    //�����O�̓����ƊO���̔���
    float ring_capsule_length = Math::calc_vector_AtoB_length({ center_ring_position.x, center_ring_position.z }, { capsule_start.x, capsule_start.z });
    if (ring_capsule_length > ring_radius + ring_width) return false;
    if (ring_capsule_length < ring_radius - ring_width) return false;

    //�����O�̕����ɂ������Ă�����true
    return true;
}

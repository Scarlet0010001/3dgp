#pragma once

#include "gltf_model.h"
#include "hit_result.h"

class Collision
{
public:
    /*----< 2D >----*/
   //--�~���m�̓����蔻��--//
    static bool HitCheckCircle(const DirectX::XMFLOAT2& pos1, float r1, const DirectX::XMFLOAT2& pos2, float r2);
    // ��`���m�̓����蔻��
    static bool HitCheckRect(const DirectX::XMFLOAT2& center_a, const DirectX::XMFLOAT2& radius_a,
        const DirectX::XMFLOAT2& center_b, const DirectX::XMFLOAT2& radius_b);
    /*----< 3D >----*/
   //--���Ƌ��̌�������--//
    static bool SphereVsSphere(
        const DirectX::XMFLOAT3& center_a, float radius_a,
        const DirectX::XMFLOAT3& center_b, float radius_b,
        DirectX::XMFLOAT3* out_center_b = nullptr);
    //--�~���Ɖ~���̌�������--//
    static bool CylinderVsCylinder(
        const DirectX::XMFLOAT3& position_a, float radius_a, float height_a,
        const DirectX::XMFLOAT3& position_b, float radius_b, float height_b,
        DirectX::XMFLOAT3* out_position_b = nullptr);
    //--���Ɖ~���̌�������--//
    static bool SphereVsCylinder(
        const DirectX::XMFLOAT3& sphere_position, float sphere_radius,
        const DirectX::XMFLOAT3& cylinder_position, float cylinder_radius, float cylinder_height,
        DirectX::XMFLOAT3* out_cylinder_position = nullptr);
    //--�����̂ƒ����̂̌�������--//
    static bool CuboidVsCuboid(
        const DirectX::XMFLOAT3& center_a, const DirectX::XMFLOAT3& radius_a,
        const DirectX::XMFLOAT3& center_b, const DirectX::XMFLOAT3& radius_b,
        DirectX::XMFLOAT3* velocity_b = nullptr);
    //-----�����̂̊e�p�����[�^�[�ϊ�-----//
    // center,radius �� min position
    static const DirectX::XMFLOAT3& CalcCuboidMinPos(const DirectX::XMFLOAT3& center, const DirectX::XMFLOAT3& radius)
    {
        return DirectX::XMFLOAT3(center.x - radius.x, center.y - radius.y, center.z - radius.z);
    }
    // center,radius �� max position
    static const DirectX::XMFLOAT3& CalcCuboidMaxPos(const DirectX::XMFLOAT3& center, const DirectX::XMFLOAT3& radius)
    {
        return DirectX::XMFLOAT3(center.x + radius.x, center.y + radius.y, center.z + radius.z);
    }
    // min,max position �� radius
    static const DirectX::XMFLOAT3& CalcCuboidRadius(const DirectX::XMFLOAT3& min_pos, const DirectX::XMFLOAT3& max_pos)
    {
        return DirectX::XMFLOAT3((max_pos.x - min_pos.x) / 2, (max_pos.y - min_pos.y) / 2, (max_pos.z - min_pos.z) / 2);
    }
    // min,max position �� center
    static const DirectX::XMFLOAT3& CalcCuboidCenter(const DirectX::XMFLOAT3& min_pos, const DirectX::XMFLOAT3& max_pos)
    {
        DirectX::XMFLOAT3 radius = CalcCuboidRadius(min_pos, max_pos);
        return DirectX::XMFLOAT3(max_pos.x - radius.x, max_pos.y - radius.y, max_pos.z - radius.z);
    }
    //--������ƒ����̂̌�������--//
    static bool FrustumVsCuboid(DirectX::XMFLOAT4X4 camara_view, DirectX::XMFLOAT4X4 camara_proj, const DirectX::XMFLOAT3& cuboid_min_pos, const DirectX::XMFLOAT3& cuboid_max_pos);
    static bool ForefrontFrustumVsCuboid(DirectX::XMFLOAT4X4 camara_view, float camera_range, const DirectX::XMFLOAT3& cuboid_min_pos, const DirectX::XMFLOAT3& cuboid_max_pos); // �v���C���[�ƃJ�����̊Ԃ̎�����
    //--���ƃJ�v�Z���̌�������--//
    static bool SphereVsCapsule(
        const DirectX::XMFLOAT3& sphere_center, float sphere_radius,
        const DirectX::XMFLOAT3& capsule_start, const DirectX::XMFLOAT3& capsule_end, float capsule_radius);
    //--�J�v�Z���ƃJ�v�Z���̌�������--//
    static bool CapsuleVsCapsule(
        const DirectX::XMFLOAT3& start_a, const DirectX::XMFLOAT3& end_a, float radius_a,
        const DirectX::XMFLOAT3& start_b, const DirectX::XMFLOAT3& end_b, float radius_b);

    //--���C�ƃ��f���̌�������--//
    static bool RayVsModel(
        const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, const gltf_model* model,
        const DirectX::XMFLOAT4X4 model_world_mat,
        HitResult& result);

    static bool RingVsCapsule(const DirectX::XMFLOAT3& center_ring_position, float ring_radius, float ring_width, float ring_height,
        const DirectX::XMFLOAT3& capsule_start, const DirectX::XMFLOAT3& capsule_end, float capsule_radius);
};

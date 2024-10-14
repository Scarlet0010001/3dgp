#pragma once


//******************************************************************************
//
//
//      ���[�U�[�i���[�e�B���e�B�[�j
//
//
//******************************************************************************

//------< �C���N���[�h >----------------------------------------------------------
#include <algorithm>
#include <sstream>
#include <bitset>
#include <assert.h>
#include <DirectXMath.h>
#include<vector>
#include "imgui/imgui.h"
//------< inline function >-----------------------------------------------------
namespace Math
{
    //--------------------------------------------------------------
    //  �����l�̃C�R�[������
    //--------------------------------------------------------------
    inline float equal_check(float value1, float value2, float ep = FLT_EPSILON)
    {
        return (value1 >= value2 - ep) && (value1 < value2 + ep);
    }
    //--------------------------------------------------------------
    //  �l��͈͓��Ɏ��߂�֐�
    //--------------------------------------------------------------
    //    �����Fconst float& v  ���͂��鐔�l
    //        �Fconst float& lo �ŏ��l
    //        �Fconst float& hi �ő�l
    //  �߂�l�Fconst float&    �͈͓��Ɏ��܂������l
    //--------------------------------------------------------------
    inline const float& Clamp(const float& v, const float& lo, const float& hi)
    {
        assert(hi >= lo);
        return (std::max)((std::min)(v, hi), lo);
    }
    inline const int& Clamp(const int& v, const int& lo, const int& hi)
    {
        assert(hi >= lo);
        return (std::max)((std::min)(v, hi), lo);
    }
    //--------------------------------------------------------------
    //  �l�����b�v�A���E���h�i�͈͂��J��Ԃ��j������
    //--------------------------------------------------------------
    //  const int v  �͈͂��J��Ԃ��������l
    //  const int lo �����l
    //  const int hi ����l�ilo���傫���l�łȂ���΂Ȃ�Ȃ��j
    //--------------------------------------------------------------
    //  �߂�l�Fint    v��lo����hi�܂ł͈̔͂Ń��b�v�A���E���h���������l
    //--------------------------------------------------------------
    inline int Wrap(const int v, const int lo, const int hi)
    {
        assert(hi > lo);
        const int n = (v - lo) % (hi - lo); // �����̏�]��c++11����g�p�ɂȂ���
        return n >= 0 ? (n + lo) : (n + hi);
    }
    // float��
    inline float Wrap(const float v, const float lo, const float hi)
    {
        assert(hi > lo);
        const float n = std::fmod(v - lo, hi - lo);
        return n >= 0 ? (n + lo) : (n + hi);
    }
    //--------------------------------------------------------------
    //  �C�ӂ̃t���[�����true��Ԃ�
    //--------------------------------------------------------------
    inline bool StopWatch(const int frame, int& timer)
    {
        assert(frame > 0);
        assert(frame >= timer);

        if (timer < frame) ++timer;
        return timer >= frame;
    }
    //--------------------------------------------------------------
    //  3D���f���̍��W�n��ύX����
    //--------------------------------------------------------------
    enum COORDINATE_SYSTEM
    {
        RHS_YUP, LHS_YUP, RHS_ZUP, LHS_ZUP,
    };
    inline const DirectX::XMFLOAT4X4& conversion_coordinate_system(
        COORDINATE_SYSTEM coordinate_system, float scale_factor = 1.0f)
    {
        const DirectX::XMFLOAT4X4 coordinate_system_transforms[]{
            {-1,0,0,0,   0,1,0,0,   0,0,1,0,   0,0,0,1},    // 0:RHS Y-UP
            { 1,0,0,0,   0,1,0,0,   0,0,1,0,   0,0,0,1},	// 1:LHS Y-UP
            {-1,0,0,0,   0,0,-1,0,  0,1,0,0,   0,0,0,1},	// 2:RHS Z-UP
            { 1,0,0,0,   0,0,1,0,   0,1,0,0,   0,0,0,1},	// 3:LHS Z-UP
        };
        // To change the units from centimeters to meters, set 'scale_factor' to 0.01.
        DirectX::XMFLOAT4X4 mat;
        DirectX::XMStoreFloat4x4(&mat, DirectX::XMMATRIX{ DirectX::XMLoadFloat4x4(&coordinate_system_transforms[coordinate_system])
            * DirectX::XMMatrixScaling(scale_factor, scale_factor, scale_factor) });
        return mat;
    }
    //--------------------------------------------------------------
    //  ���[���h�s����v�Z����(�I�C���[)
    //--------------------------------------------------------------
    // �߂�l�F���[���h�s��
    //--------------------------------------------------------------
    inline auto calc_world_matrix(const DirectX::XMFLOAT3& scale,
        const DirectX::XMFLOAT3& rotate, const DirectX::XMFLOAT3& trans, COORDINATE_SYSTEM coordinate_system = RHS_YUP)
    {
        DirectX::XMMATRIX C{ DirectX::XMLoadFloat4x4(&conversion_coordinate_system(coordinate_system, 1.0f)) };
        DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) };
        DirectX::XMMATRIX R{ DirectX::XMMatrixRotationRollPitchYaw(rotate.x, rotate.y, rotate.z) };
        DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(trans.x, trans.y, trans.z) };
        DirectX::XMFLOAT4X4 world;
        DirectX::XMStoreFloat4x4(&world, C * S * R * T);
        return world;
    }
    //--------------------------------------------------------------
    // ���[���h�s����v�Z����(�I�C���[�̐e�q�֌W)
    //--------------------------------------------------------------
    // �߂�l�F�e�q�֌W���l���������[���h�s��
    //--------------------------------------------------------------
    inline auto calc_world_matrix(const DirectX::XMFLOAT3& parent_scale,
        const DirectX::XMFLOAT3& parent_rotate, const DirectX::XMFLOAT3& parent_trans,
        const DirectX::XMFLOAT3& child_scale, const DirectX::XMFLOAT3& child_rotate,
        const DirectX::XMFLOAT3& child_trans, COORDINATE_SYSTEM coordinate_system = RHS_YUP)
    {
        DirectX::XMMATRIX C{ DirectX::XMLoadFloat4x4(&conversion_coordinate_system(coordinate_system, 1.0f)) };
        DirectX::XMMATRIX P_S{ DirectX::XMMatrixScaling(parent_scale.x, parent_scale.y, parent_scale.z) };
        DirectX::XMMATRIX P_R{ DirectX::XMMatrixRotationRollPitchYaw(parent_rotate.x, parent_rotate.y, parent_rotate.z) };
        DirectX::XMMATRIX P_T{ DirectX::XMMatrixTranslation(parent_trans.x, parent_trans.y, parent_trans.z) };

        DirectX::XMMATRIX C_S{ DirectX::XMMatrixScaling(child_scale.x, child_scale.y, child_scale.z) };
        DirectX::XMMATRIX C_R{ DirectX::XMMatrixRotationRollPitchYaw(child_rotate.x, child_rotate.y, child_rotate.z) };
        DirectX::XMMATRIX C_T{ DirectX::XMMatrixTranslation(child_trans.x, child_trans.y, child_trans.z) };

        DirectX::XMMATRIX P_W = P_S * P_R * P_T;
        DirectX::XMMATRIX C_W = C_S * C_R * C_T;
        DirectX::XMFLOAT4X4 world;
        DirectX::XMStoreFloat4x4(&world, C * C_W * P_W);

        return world;
    }

    inline auto calc_world_matrix(const DirectX::XMFLOAT4X4 parent_world_matrix,
        const DirectX::XMFLOAT3& child_scale, const DirectX::XMFLOAT3& child_rotate,
        const DirectX::XMFLOAT3& child_trans, COORDINATE_SYSTEM coordinate_system = RHS_YUP)
    {
        DirectX::XMMATRIX C{ DirectX::XMLoadFloat4x4(&conversion_coordinate_system(coordinate_system, 1.0f)) };

        DirectX::XMMATRIX C_S{ DirectX::XMMatrixScaling(child_scale.x, child_scale.y, child_scale.z) };
        DirectX::XMMATRIX C_R{ DirectX::XMMatrixRotationRollPitchYaw(child_rotate.x, child_rotate.y, child_rotate.z) };
        DirectX::XMMATRIX C_T{ DirectX::XMMatrixTranslation(child_trans.x, child_trans.y, child_trans.z) };

        DirectX::XMMATRIX P_W = XMLoadFloat4x4(&parent_world_matrix);
        DirectX::XMMATRIX C_W = C_S * C_R * C_T;
        DirectX::XMFLOAT4X4 world;
        DirectX::XMStoreFloat4x4(&world, C * C_W * P_W);

        return world;
    }

    //--------------------------------------------------------------
    //  ���[���h�s����v�Z����(�N�H�[�^�j�I��)
    //--------------------------------------------------------------
    // �߂�l�F���[���h�s��
    //--------------------------------------------------------------
    inline auto calc_world_matrix(const DirectX::XMFLOAT3& scale,
        const DirectX::XMFLOAT4& orien, const DirectX::XMFLOAT3& trans,
        COORDINATE_SYSTEM coordinate_system = RHS_YUP)
    {
        DirectX::XMMATRIX C{ DirectX::XMLoadFloat4x4(&conversion_coordinate_system(coordinate_system, 1.0f)) };
        DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) };
        DirectX::XMMATRIX R{ DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&orien)) };
        DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(trans.x, trans.y, trans.z) };
        DirectX::XMFLOAT4X4 world;
        DirectX::XMStoreFloat4x4(&world, C * S * R * T);

        return world;
    }
    //--------------------------------------------------------------
    //  ���[���h�s����v�Z����(�N�H�[�^�j�I���̐e�q�֌W)
    //--------------------------------------------------------------
    // �߂�l�F�e�q�֌W���l���������[���h�s��
    //--------------------------------------------------------------
    inline auto calc_world_matrix(const DirectX::XMFLOAT3& parent_scale,
        const DirectX::XMFLOAT4& parent_orien, const DirectX::XMFLOAT3& parent_trans,
        const DirectX::XMFLOAT3& child_scale, const DirectX::XMFLOAT4& child_orien,
        const DirectX::XMFLOAT3& child_trans, COORDINATE_SYSTEM coordinate_system = RHS_YUP)
    {
        DirectX::XMMATRIX C{ DirectX::XMLoadFloat4x4(&conversion_coordinate_system(coordinate_system, 1.0f)) };
        DirectX::XMMATRIX P_S{ DirectX::XMMatrixScaling(parent_scale.x, parent_scale.y, parent_scale.z) };
        DirectX::XMMATRIX P_R{ DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&parent_orien)) };
        DirectX::XMMATRIX P_T{ DirectX::XMMatrixTranslation(parent_trans.x, parent_trans.y, parent_trans.z) };

        DirectX::XMMATRIX C_S{ DirectX::XMMatrixScaling(child_scale.x, child_scale.y, child_scale.z) };
        DirectX::XMMATRIX C_R{ DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&child_orien)) };
        DirectX::XMMATRIX C_T{ DirectX::XMMatrixTranslation(child_trans.x, child_trans.y, child_trans.z) };

        DirectX::XMMATRIX P_W = P_S * P_R * P_T;
        DirectX::XMMATRIX C_W = C_S * C_R * C_T;
        DirectX::XMFLOAT4X4 world;
        DirectX::XMStoreFloat4x4(&world, C * C_W * P_W);

        return world;
    }

    //--------------------------------------------------------------
   //  ���[���h�s����v�Z����(�N�H�[�^�j�I���̐e�q�֌W)
   //--------------------------------------------------------------
   // �߂�l�F�e�q�֌W���l���������[���h�s��
   //--------------------------------------------------------------
    inline auto calc_world_matrix(const DirectX::XMFLOAT4X4 parent_world_matrix,
        const DirectX::XMFLOAT3& child_scale, const DirectX::XMFLOAT4& child_orien,
        const DirectX::XMFLOAT3& child_trans, COORDINATE_SYSTEM coordinate_system = RHS_YUP)
    {
        DirectX::XMMATRIX C{ DirectX::XMLoadFloat4x4(&conversion_coordinate_system(coordinate_system, 1.0f)) };
        DirectX::XMMATRIX C_S{ DirectX::XMMatrixScaling(child_scale.x, child_scale.y, child_scale.z) };
        DirectX::XMMATRIX C_R{ DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&child_orien)) };
        DirectX::XMMATRIX C_T{ DirectX::XMMatrixTranslation(child_trans.x, child_trans.y, child_trans.z) };

        DirectX::XMMATRIX P_W = XMLoadFloat4x4(&parent_world_matrix);
        DirectX::XMMATRIX C_W = C_S * C_R * C_T;
        DirectX::XMFLOAT4X4 world;
        DirectX::XMStoreFloat4x4(&world, C * C_W * P_W);

        return world;
    }
    //--------------------------------------------------------------
    //  �e�q�֌W���l������position���Z�o
    //--------------------------------------------------------------
    inline auto calc_world_position(const DirectX::XMFLOAT3& parent_pos, const DirectX::XMFLOAT3& child_pos)
    {
        using namespace DirectX;
        XMMATRIX P_T{ DirectX::XMMatrixTranslation(parent_pos.x, parent_pos.y, parent_pos.z) };
        XMMATRIX C_T{ DirectX::XMMatrixTranslation(child_pos.x, child_pos.y, child_pos.z) };
        XMFLOAT4X4 m4x4 = {};
        XMStoreFloat4x4(&m4x4, C_T * P_T);

        return XMFLOAT3(m4x4._41, m4x4._42, m4x4._43);
    }
    //--------------------------------------------------------------
    // �_(x, y)���_(cx, cy)�𒆐S�Ɋp(angle)�ŉ�]�������̍��W���Z�o
    //--------------------------------------------------------------
    inline void Rotate(float& x, float& y, float cx, float cy, float angle)
    {
        x -= cx;
        y -= cy;

        float cos{ cosf(DirectX::XMConvertToRadians(angle)) };
        float sin{ sinf(DirectX::XMConvertToRadians(angle)) };
        float tx{ x }, ty{ y };
        x = cos * tx + -sin * ty;
        y = sin * tx + cos * ty;

        x += cx;
        y += cy;
    }
    //-------------------------------------------------------
    //  ���`�⊮(float3)
    //-------------------------------------------------------
    //  �߂�l�F�⊮���ꂽ�l
    //-------------------------------------------------------
    inline const DirectX::XMFLOAT3& Lerp(const DirectX::XMFLOAT3& start,
        const DirectX::XMFLOAT3& end, float lerp_rate)
    {
        using namespace DirectX;
        XMVECTOR start_vec = XMLoadFloat3(&start);
        XMVECTOR end_vec = XMLoadFloat3(&end);
        XMFLOAT3 lerp;
        XMStoreFloat3(&lerp, XMVectorLerp(start_vec, end_vec, lerp_rate));
        return lerp;
    }
    //-------------------------------------------------------
    //  ���`�⊮(float2)
    //-------------------------------------------------------
    //  �߂�l�F�⊮���ꂽ�l
    //-------------------------------------------------------
    inline const DirectX::XMFLOAT2& Lerp(const DirectX::XMFLOAT2& start,
        const DirectX::XMFLOAT2& end, float lerp_rate)
    {
        using namespace DirectX;
        XMVECTOR start_vec = XMLoadFloat2(&start);
        XMVECTOR end_vec = XMLoadFloat2(&end);
        XMFLOAT2 lerp;
        XMStoreFloat2(&lerp, XMVectorLerp(start_vec, end_vec, lerp_rate));
        return lerp;
    }
    //-------------------------------------------------------
    //  ���`�⊮(float)
    //-------------------------------------------------------
    //  �߂�l�F�ۊǂ��ꂽ�l
    //-------------------------------------------------------
    inline float Lerp(const float start, const float end, float lerp_rate)
    {
        using namespace DirectX;
        XMVECTOR start_vec = XMLoadFloat(&start);
        XMVECTOR end_vec = XMLoadFloat(&end);
        float lerp;
        XMStoreFloat(&lerp, XMVectorLerp(start_vec, end_vec, lerp_rate));
        return lerp;
    }
    //-------------------------------------------------------
    //  ��_����x�N�g�����Z�o����
    //-------------------------------------------------------
    //  �߂�l�Fa����b�Ɍ������x�N�g��
    //-------------------------------------------------------
    inline const DirectX::XMVECTOR& calc_vector_AtoB_vec(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
    {
        using namespace DirectX;
        XMVECTOR a_vec = DirectX::XMLoadFloat3(&a);
        XMVECTOR b_vec = DirectX::XMLoadFloat3(&b);
        XMVECTOR vec = b_vec - a_vec;
        return vec;
    }
    inline const DirectX::XMFLOAT3& calc_vector_AtoB(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
    {
        using namespace DirectX;
        XMVECTOR a_vec = DirectX::XMLoadFloat3(&a);
        XMVECTOR b_vec = DirectX::XMLoadFloat3(&b);
        XMVECTOR Vec = b_vec - a_vec;
        XMFLOAT3 vec;
        XMStoreFloat3(&vec, Vec);
        return vec;
    }
    //-------------------------------------------------------
    //  ��_����x�N�g�����Z�o����
    //-------------------------------------------------------
    //  �߂�l�Fa����b�Ɍ������x�N�g��
    //-------------------------------------------------------
    inline const DirectX::XMVECTOR& calc_vector_AtoB(const DirectX::XMFLOAT2& a, const DirectX::XMFLOAT2& b)
    {
        using namespace DirectX;
        XMVECTOR a_vec = DirectX::XMLoadFloat2(&a);
        XMVECTOR b_vec = DirectX::XMLoadFloat2(&b);
        XMVECTOR vec = b_vec - a_vec;
        return vec;
    }
    //--------------------------------------------------------------
    //  ��_����x�N�g�����Z�o����(���K��)
    //--------------------------------------------------------------
    //  �߂�l�Fa����b�Ɍ��������K�����ꂽ�x�N�g��
    //--------------------------------------------------------------
    inline const DirectX::XMVECTOR& calc_vector_AtoB_normalize_vec(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
    {
        return DirectX::XMVector3Normalize(calc_vector_AtoB_vec(a, b));
    }

    inline const DirectX::XMFLOAT3& calc_vector_AtoB_normalize(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
    {
        DirectX::XMFLOAT3 v;
        DirectX::XMStoreFloat3(&v, DirectX::XMVector3Normalize(calc_vector_AtoB_vec(a, b)));
        return v;
    }
    //--------------------------------------------------------------
    //  ��_����x�N�g�����Z�o����(���K��)
    //--------------------------------------------------------------
    //  �߂�l�Fa����b�Ɍ��������K�����ꂽ�x�N�g��
    //--------------------------------------------------------------
    inline const DirectX::XMVECTOR& calc_vector_AtoB_normalize(const DirectX::XMFLOAT2& a, const DirectX::XMFLOAT2& b)
    {
        return DirectX::XMVector2Normalize(calc_vector_AtoB(a, b));
    }
    //--------------------------------------------------------------
    //  ��_����x�N�g�����Z�o���A���̒������v�Z����(2��)
    //--------------------------------------------------------------
    //  �߂�l�Fa����b�Ɍ������x�N�g���̒���
    //--------------------------------------------------------------
    inline float calc_vector_AtoB_length_sq(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
    {
        using namespace DirectX;
        XMVECTOR length_sq_vec = XMVector3LengthSq(calc_vector_AtoB_vec(a, b));
        float length_sq;
        XMStoreFloat(&length_sq, length_sq_vec);
        return length_sq;
    }
    //--------------------------------------------------------------
    //  ��_����x�N�g�����Z�o���A���̒������v�Z����(2��)
    //--------------------------------------------------------------
    //  �߂�l�Fa����b�Ɍ������x�N�g���̒���
    //--------------------------------------------------------------
    inline float calc_vector_AtoB_length_sq(const DirectX::XMFLOAT2& a, const DirectX::XMFLOAT2& b)
    {
        using namespace DirectX;
        XMVECTOR length_sq_vec = XMVector2LengthSq(calc_vector_AtoB(a, b));
        float length_sq;
        XMStoreFloat(&length_sq, length_sq_vec);
        return length_sq;
    }
    //--------------------------------------------------------------
    //  ��_����x�N�g�����Z�o���A���̒������v�Z����
    //--------------------------------------------------------------
    //  �߂�l�Fa����b�Ɍ������x�N�g���̒���
    //--------------------------------------------------------------
    inline float calc_vector_AtoB_length(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
    {
        using namespace DirectX;
        XMVECTOR length_vec = XMVector3Length(calc_vector_AtoB_vec(a, b));
        float length;
        XMStoreFloat(&length, length_vec);
        return length;
    }
    //--------------------------------------------------------------
    //  ��_����x�N�g�����Z�o���A���̒������v�Z����
    //--------------------------------------------------------------
    //  �߂�l�Fa����b�Ɍ������x�N�g���̒���
    //--------------------------------------------------------------
    inline float calc_vector_AtoB_length(const DirectX::XMFLOAT2& a, const DirectX::XMFLOAT2& b)
    {
        using namespace DirectX;
        XMVECTOR length_vec = XMVector2Length(calc_vector_AtoB(a, b));
        float length;
        XMStoreFloat(&length, length_vec);
        return length;
    }
    //--------------------------------------------------------------
    //  �w������Ɏw�苗���i�񂾓_���Z�o����
    //--------------------------------------------------------------
    //  �߂�l�Fstart����direction������length���i�񂾒n�_
    //--------------------------------------------------------------
    inline const DirectX::XMFLOAT3& calc_designated_point(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& direction, float length)
    {
        return { start.x + direction.x * length, start.y + direction.y * length, start.z + direction.z * length };
    }
    //--------------------------------------------------------------
    //  �w������Ɏw�苗���i�񂾓_���Z�o����
    //--------------------------------------------------------------
    //  �߂�l�Fstart����direction������length���i�񂾒n�_
    //--------------------------------------------------------------
    inline const DirectX::XMFLOAT2& calc_designated_point(const DirectX::XMFLOAT2& start,
        const DirectX::XMFLOAT2& direction, float length)
    {
        return { start.x + direction.x * length, start.y + direction.y * length };
    }
    //--------------------------------------------------------------
    //  ���� start end �Ɠ_p0�̍ŋߓ_���Z�o����
    //--------------------------------------------------------------
    //  �߂�l�F���� start end �Ɠ_p0�̍ŋߓ_
    //--------------------------------------------------------------
    inline const DirectX::XMFLOAT3& calc_closest_point(const DirectX::XMFLOAT3& start,
        const DirectX::XMFLOAT3& end, const DirectX::XMFLOAT3& p0)
    {
        using namespace DirectX;
        XMVECTOR v0_vec = XMLoadFloat3(&end) - XMLoadFloat3(&start);
        XMVECTOR v1_vec = XMLoadFloat3(&p0) - XMLoadFloat3(&start);
        XMVECTOR projection_length_vec = XMVector3Dot(v1_vec, XMVector3Normalize(v0_vec));
        float projection_length;
        XMStoreFloat(&projection_length, projection_length_vec);
        XMFLOAT3 v0_norm;
        XMStoreFloat3(&v0_norm, XMVector3Normalize(v0_vec));
        return calc_designated_point(start, v0_norm, projection_length);
    }

    //--------------------------------------------------------------
    //  �x�N�g���𐳋K������
    //--------------------------------------------------------------
    //  �߂�l�F�������P�̃x�N�g��
    //--------------------------------------------------------------
    inline DirectX::XMFLOAT3 Normalize(DirectX::XMFLOAT3 V_)
    {
        auto V = DirectX::XMLoadFloat3(&V_);
        V = DirectX::XMVector3Normalize(V);
        DirectX::XMFLOAT3 ret{};
        DirectX::XMStoreFloat3(&ret, V);
        return ret;
    }
    inline DirectX::XMFLOAT2 Normalize(DirectX::XMFLOAT2 V_)
    {
        auto V = DirectX::XMLoadFloat2(&V_);
        V = DirectX::XMVector2Normalize(V);
        DirectX::XMFLOAT2 ret{};
        DirectX::XMStoreFloat2(&ret, V);
        return ret;
    }
    //--------------------------------------------------------------
    //  �O��
    //--------------------------------------------------------------
    //
    //  �����F�x�N�g���A�x�N�g���A���K�����邩�ǂ���
    //
    //--------------------------------------------------------------
    //  �߂�l�F�x�N�g��
    //--------------------------------------------------------------
    inline DirectX::XMFLOAT3 Cross(DirectX::XMFLOAT3 A_, DirectX::XMFLOAT3 B_, bool IsNormalize_ = true)
    {
        const auto VA = DirectX::XMLoadFloat3(&A_);
        const auto VB = DirectX::XMLoadFloat3(&B_);
        auto Cross = DirectX::XMVector3Cross(VA, VB);
        if (IsNormalize_)
        {
            Cross = DirectX::XMVector3Normalize(Cross);
        }
        DirectX::XMFLOAT3 cross{};
        DirectX::XMStoreFloat3(&cross, Cross);
        return cross;
    }

    //--------------------------------------------------------------
    //  �t�s��
    //--------------------------------------------------------------
    inline DirectX::XMFLOAT4X4 get_inv_mat(DirectX::XMFLOAT4X4 m)
    {
        auto M = DirectX::XMLoadFloat4x4(&m);

        auto inv = DirectX::XMMatrixInverse(nullptr, M);

        DirectX::XMFLOAT4X4 ans;
        DirectX::XMStoreFloat4x4(&ans, inv);

        return ans;
    }
    //--------------------------------------------------------------
    //  ����
    //--------------------------------------------------------------
    //
    //  �����F�x�N�g���A�x�N�g��
    //
    //--------------------------------------------------------------
    //  �߂�l�F�p�x�i���W�A���j
    //--------------------------------------------------------------
    inline float Dot(const DirectX::XMFLOAT3 A, const DirectX::XMFLOAT3 B)
    {
        const DirectX::XMVECTOR V0 = DirectX::XMLoadFloat3(&A);
        const DirectX::XMVECTOR V1 = DirectX::XMLoadFloat3(&B);
        const DirectX::XMVECTOR Ans = DirectX::XMVector3Dot(V0, V1);
        float ans{};
        DirectX::XMStoreFloat(&ans, Ans);
        return ans;
    }
    
    inline float Dot(const DirectX::XMFLOAT2 A, const DirectX::XMFLOAT2 B)
    {
        const DirectX::XMVECTOR V0 = DirectX::XMLoadFloat2(&A);
        const DirectX::XMVECTOR V1 = DirectX::XMLoadFloat2(&B);
        const DirectX::XMVECTOR Ans = DirectX::XMVector2Dot(V0, V1);
        float ans{};
        DirectX::XMStoreFloat(&ans, Ans);
        return ans;
    }
    //--------------------------------------------------------------
    //  `�x�N�g���̒������v�Z����
    //--------------------------------------------------------------
    //
    //  �����F�x�N�g��
    //
    //--------------------------------------------------------------
    //  �߂�l�F����
    //--------------------------------------------------------------
    inline float Length(DirectX::XMFLOAT3 V_)
    {
        auto VL = DirectX::XMLoadFloat3(&V_);
        VL = DirectX::XMVector3Length(VL);
        float ret;
        DirectX::XMStoreFloat(&ret, VL);
        return ret;
    }
    inline float Length(DirectX::XMFLOAT2 V_)
    {
        auto VL = DirectX::XMLoadFloat2(&V_);
        VL = DirectX::XMVector2Length(VL);
        float ret;
        DirectX::XMStoreFloat(&ret, VL);
        return ret;
    }
    //--------------------------------------------------------------
    //  `�x�N�g�����X�P�[���{����
    //--------------------------------------------------------------
    //
    //  �����F�x�N�g��
    //
    //--------------------------------------------------------------
    //  �߂�l�F�X�P�[���{�����x�N�g��
    //--------------------------------------------------------------
    inline DirectX::XMFLOAT3 VectorScale(DirectX::XMFLOAT3 V_, float scale)
    {
        auto VL = DirectX::XMLoadFloat3(&V_);
        VL = DirectX::XMVectorScale(VL, scale);
        DirectX::XMFLOAT3 ret;
        DirectX::XMStoreFloat3(&ret, VL);
        return ret;
    }

    inline DirectX::XMFLOAT3 VectorScale(DirectX::XMVECTOR V_, float scale)
    {
        V_ = DirectX::XMVectorScale(V_, scale);
        DirectX::XMFLOAT3 ret;
        DirectX::XMStoreFloat3(&ret, V_);
        return ret;
    }


    //--------------------------------------------------------------
    //  �N�H�[�^�j�I����]
    //--------------------------------------------------------------
    //
    //  �����F���݂̉�]�p���A���A��]�p
    //
    //--------------------------------------------------------------
    //  �߂�l�F��]��̎p��
    //--------------------------------------------------------------
    inline DirectX::XMFLOAT4 rot_quaternion(DirectX::XMFLOAT4 Orientation_, DirectX::XMFLOAT3 Axis_, float Radian_)
    {

        if (fabs(Radian_) > 1e-8f)
        {
            // �ϊ�
            const auto Axis = DirectX::XMLoadFloat3(&Axis_);
            auto oriV = DirectX::XMLoadFloat4(&Orientation_);

            // ��]�N�H�[�^�j�I�����Z�o
            const auto rotQua = DirectX::XMQuaternionRotationAxis(Axis, Radian_);
            oriV = DirectX::XMQuaternionMultiply(oriV, rotQua);

            DirectX::XMFLOAT4 ret{};
            DirectX::XMStoreFloat4(&ret, oriV);
            return ret;
        }
        return Orientation_;
    }
    //--------------------------------------------------------------
   //
   //  �����F���݂̉�]�p���A���A��]�p,�o�ߎ��ԁA���[�g
   //
   //--------------------------------------------------------------
   //  �߂�l�F��]��̎p��
   //--------------------------------------------------------------
    inline DirectX::XMFLOAT4 rot_quaternion(DirectX::XMFLOAT4 Orientation_, DirectX::XMFLOAT3 Axis_, float Radian_,float elapsed_time,float rate = 10)
    {
        if (fabs(Radian_) > 1e-8f)
        {
            // �ϊ�
            const auto Axis = DirectX::XMLoadFloat3(&Axis_);
            auto oriV = DirectX::XMLoadFloat4(&Orientation_);

            // ��]�N�H�[�^�j�I�����Z�o
            const auto rotQua = DirectX::XMQuaternionRotationAxis(Axis, Radian_);
            DirectX::XMVECTOR End = DirectX::XMQuaternionMultiply(oriV, rotQua);
            oriV = DirectX::XMQuaternionSlerp(oriV, End, rate * elapsed_time);

            DirectX::XMFLOAT4 ret{};
            DirectX::XMStoreFloat4(&ret, oriV);
            return ret;
        }
        return Orientation_;
    }
    //--------------------------------------------------------------
   //
   //  �����F���݂̉�]�p���A���A����
   //
   //--------------------------------------------------------------
   //  �߂�l�F�w������ւ̉�]��̎p��
   //--------------------------------------------------------------
    inline DirectX::XMFLOAT4 rot_quaternion_dir(DirectX::XMFLOAT4 Orientation_, DirectX::XMFLOAT3 OriAxis_, DirectX::XMFLOAT3 DirVec_)
    {
        //�@���̃x�N�g��
        DirectX::XMVECTOR ori_axis = DirectX::XMLoadFloat3(&OriAxis_);
        DirectX::XMVECTOR Normal = DirectX::XMLoadFloat3(&DirVec_);
        Normal = DirectX::XMVector3Normalize(Normal);
        //���x�N�g���Z�o
        DirectX::XMVECTOR axis;	//��]��
        float angle;			//��]�p
        axis = DirectX::XMVector3Cross(ori_axis, Normal);
        DirectX::XMVECTOR Ang = DirectX::XMVector3Dot(ori_axis, Normal);
        DirectX::XMStoreFloat(&angle, Ang);
        angle = acosf(angle);
        if (fabs(angle) > 0.001f)
        {
            // �ϊ�
            auto oriV = DirectX::XMLoadFloat4(&Orientation_);

            // ��]�N�H�[�^�j�I�����Z�o
            DirectX::XMVECTOR rotQua;
            rotQua = DirectX::XMQuaternionRotationAxis(axis, angle);//���̕����ɓ����N�I�[�^�j�I��
            oriV = DirectX::XMQuaternionMultiply(oriV, rotQua);

            DirectX::XMFLOAT4 ret{};
            DirectX::XMStoreFloat4(&ret, oriV);
            return ret;
        }
        return Orientation_;
    }
    //--------------------------------------------------------------
   //
   //  �����F�ϊ���̉�]�s��A�ϊ����̃N�H�[�^�j�I��
   //
   //--------------------------------------------------------------
   //  �N�H�[�^�j�I�������]�s��ɕϊ�
   //--------------------------------------------------------------
    inline void transform_quaternion_to_rotatemat(DirectX::XMFLOAT4X4& m,
        DirectX::XMFLOAT4 q)
    {
        m._11 = 1.0f - 2.0f * q.y * q.y - 2.0f * q.z * q.z;
        m._12 = 2.0f * q.x * q.y + 2.0f * q.w * q.z;
        m._13 = 2.0f * q.x * q.z - 2.0f * q.w * q.y;
        
        m._21 = 2.0f * q.x * q.y - 2.0f * q.w * q.z;
        m._22 = 1.0f - 2.0f * q.x * q.x - 2.0f * q.z * q.z;
        m._23 = 2.0f * q.y * q.z + 2.0f * q.w * q.x;
        
        m._31 = 2.0f * q.x * q.z + 2.0f * q.w * q.y;
        m._32 = 2.0f * q.y * q.z - 2.0f * q.w * q.x;
        m._33 = 1.0f - 2.0f * q.x * q.x - 2.0f * q.y * q.y;
        m._44 = 1.0f;
    }

    //--------------------------------------------------------------
    //  ��]�s�񂩂�N�H�[�^�j�I���ɕϊ�
    //--------------------------------------------------------------
   //
   //  �����F�ϊ���̃N�H�[�^�j�I���A�ϊ����̉�]�s��
   //
   //--------------------------------------------------------------
    inline bool transform_rotatemat_to_quaternion(DirectX::XMFLOAT4& q,
        DirectX::XMFLOAT4X4 m)
    {
        // �ő听��������
        float elem[4]{}; // 0:x, 1:y, 2:z, 3:w
        elem[0] = m._11 - m._22 - m._33 + 1.0f;
        elem[1] = -m._11 + m._22 - m._33 + 1.0f;
        elem[2] = -m._11 - m._22 + m._33 + 1.0f;
        elem[3] = m._11 + m._22 + m._33 + 1.0f;

        unsigned biggestIndex = 0;
        for (int i = 1; i < 4; i++) {
            if (elem[i] > elem[biggestIndex])
                biggestIndex = i;
        }

        if (elem[biggestIndex] < 0.0f)
            return false; // �����̍s��ɊԈႢ����I

        // �ő�v�f�̒l���Z�o
        float* Q[4] = { &q.x, &q.y, &q.z, &q.w };
        float v = sqrtf(elem[biggestIndex]) * 0.5f;
        *Q[biggestIndex] = v;
        float mult = 0.25f / v;

        switch (biggestIndex) {
        case 0: // x
            *Q[1] = (m._12 + m._21) * mult;
            *Q[2] = (m._31 + m._13) * mult;
            *Q[3] = (m._23 - m._32) * mult;
            break;
        case 1: // y
            *Q[0] = (m._12 + m._21) * mult;
            *Q[2] = (m._23 + m._32) * mult;
            *Q[3] = (m._31 - m._13) * mult;
            break;
        case 2: // z
            *Q[0] = (m._31 + m._13) * mult;
            *Q[1] = (m._23 + m._32) * mult;
            *Q[3] = (m._12 - m._21) * mult;
            break;
        case 3: // w
            *Q[0] = (m._23 - m._32) * mult;
            *Q[1] = (m._31 - m._13) * mult;
            *Q[2] = (m._12 - m._21) * mult;
            break;
        }

        return true;
    }
    //--------------------------------------------------------------
    //  �x�N�g���擾
    //--------------------------------------------------------------
    //�E�x�N�g���擾
    inline DirectX::XMVECTOR get_posture_right_vec(DirectX::XMFLOAT4 orientation)
    {
        DirectX::XMVECTOR orientationVec = DirectX::XMLoadFloat4(&orientation);
        DirectX::XMVECTOR right;
        DirectX::XMMATRIX m = DirectX::XMMatrixRotationQuaternion(orientationVec);
        DirectX::XMFLOAT4X4 m4x4 = {};
        DirectX::XMStoreFloat4x4(&m4x4, m);

        right = { m4x4._11, m4x4._12, m4x4._13 };
        right = DirectX::XMVector3Normalize(right);
        return right;
    }

    //��x�N�g���擾
    inline DirectX::XMVECTOR get_posture_up_vec(DirectX::XMFLOAT4 orientation)
    {
        DirectX::XMVECTOR orientationVec = DirectX::XMLoadFloat4(&orientation);
        DirectX::XMVECTOR up;
        DirectX::XMMATRIX m = DirectX::XMMatrixRotationQuaternion(orientationVec);
        DirectX::XMFLOAT4X4 m4x4 = {};
        DirectX::XMStoreFloat4x4(&m4x4, m);

        up = { m4x4._21, m4x4._22, m4x4._23 };
        up = DirectX::XMVector3Normalize(up);
        return up;
    }

    //�O�x�N�g���擾
    inline DirectX::XMVECTOR get_posture_forward_vec(DirectX::XMFLOAT4 orientation)
    {
        DirectX::XMVECTOR orientationVec = DirectX::XMLoadFloat4(&orientation);
        DirectX::XMVECTOR forward;
        DirectX::XMMATRIX m = DirectX::XMMatrixRotationQuaternion(orientationVec);
        DirectX::XMFLOAT4X4 m4x4 = {};
        DirectX::XMStoreFloat4x4(&m4x4, m);
        forward = { m4x4._31, m4x4._32, m4x4._33 };
        forward = DirectX::XMVector3Normalize(forward);
        return forward;
    }

    //�E�x�N�g���擾(�N�H�[�^�j�I��)
    inline DirectX::XMFLOAT3 get_posture_right(DirectX::XMFLOAT4 orientation)
    {
        DirectX::XMVECTOR orientationVec = DirectX::XMLoadFloat4(&orientation);
        DirectX::XMVECTOR right;
        DirectX::XMMATRIX m = DirectX::XMMatrixRotationQuaternion(orientationVec);
        DirectX::XMFLOAT4X4 m4x4 = {};
        DirectX::XMStoreFloat4x4(&m4x4, m);

        right = { m4x4._11, m4x4._12, m4x4._13 };
        right = DirectX::XMVector3Normalize(right);
        DirectX::XMFLOAT3 v;
        DirectX::XMStoreFloat3(&v, right);
        return v;
    }

    //�E�x�N�g���擾(��]�s��)
    inline DirectX::XMFLOAT3 get_posture_right(DirectX::XMFLOAT4X4 transform)
    {
        DirectX::XMFLOAT4X4 w = transform;

        DirectX::XMFLOAT3 pos = { w._41,w._42,w._43 };
        DirectX::XMFLOAT3 scale = { Math::Length({w._11,w._12,w._13}),  Math::Length({w._21,w._22,w._23}),  Math::Length({w._31,w._32,w._33}) };

        DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) };
        DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z) };
        DirectX::XMMATRIX R = DirectX::XMLoadFloat4x4(&w) * DirectX::XMMatrixInverse(nullptr, S) * DirectX::XMMatrixInverse(nullptr, T);

        DirectX::XMFLOAT4X4 r;
        DirectX::XMStoreFloat4x4(&r, R);

        return { r._11,r._12, r._13 };
    }

    //��x�N�g���擾
    inline DirectX::XMFLOAT3 get_posture_up(DirectX::XMFLOAT4 orientation)
    {
        DirectX::XMVECTOR orientationVec = DirectX::XMLoadFloat4(&orientation);
        DirectX::XMVECTOR up;
        DirectX::XMMATRIX m = DirectX::XMMatrixRotationQuaternion(orientationVec);
        DirectX::XMFLOAT4X4 m4x4 = {};
        DirectX::XMStoreFloat4x4(&m4x4, m);

        up = { m4x4._21, m4x4._22, m4x4._23 };
        up = DirectX::XMVector3Normalize(up);
        DirectX::XMFLOAT3 v;
        DirectX::XMStoreFloat3(&v, up);
        return v;
    }

    //��x�N�g���擾(��]�s��)
    inline DirectX::XMFLOAT3 get_posture_up(DirectX::XMFLOAT4X4 transform)
    {
        DirectX::XMFLOAT4X4 w = transform;

        DirectX::XMFLOAT3 pos = { w._41,w._42,w._43 };
        DirectX::XMFLOAT3 scale = { Math::Length({w._11,w._12,w._13}),  Math::Length({w._21,w._22,w._23}),  Math::Length({w._31,w._32,w._33}) };

        DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) };
        DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z) };
        DirectX::XMMATRIX R = DirectX::XMLoadFloat4x4(&w) * DirectX::XMMatrixInverse(nullptr, S) * DirectX::XMMatrixInverse(nullptr, T);

        DirectX::XMFLOAT4X4 r;
        DirectX::XMStoreFloat4x4(&r, R);

        return { r._21,r._22, r._23 };
    }

    //�O�x�N�g���擾
    inline DirectX::XMFLOAT3 get_posture_forward(DirectX::XMFLOAT4 orientation)
    {
        DirectX::XMVECTOR orientationVec = DirectX::XMLoadFloat4(&orientation);
        DirectX::XMVECTOR forward;
        DirectX::XMMATRIX m = DirectX::XMMatrixRotationQuaternion(orientationVec);
        DirectX::XMFLOAT4X4 m4x4 = {};
        DirectX::XMStoreFloat4x4(&m4x4, m);
        forward = { m4x4._31, m4x4._32, m4x4._33 };
        forward = DirectX::XMVector3Normalize(forward);
        DirectX::XMFLOAT3 v;
        DirectX::XMStoreFloat3(&v, forward);
        return v;
    }

    //�O�x�N�g���擾(��]�s��)
    inline DirectX::XMFLOAT3 get_posture_forward(DirectX::XMFLOAT4X4 transform)
    {
        DirectX::XMFLOAT4X4 w = transform;

        DirectX::XMFLOAT3 pos = { w._41,w._42,w._43 };
        DirectX::XMFLOAT3 scale = { Math::Length({w._11,w._12,w._13}),  Math::Length({w._21,w._22,w._23}),  Math::Length({w._31,w._32,w._33}) };

        DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) };
        DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z) };
        DirectX::XMMATRIX R = DirectX::XMLoadFloat4x4(&w) * DirectX::XMMatrixInverse(nullptr, S) * DirectX::XMMatrixInverse(nullptr, T);

        DirectX::XMFLOAT4X4 r;
        DirectX::XMStoreFloat4x4(&r, R);

        return { r._31,r._32, r._33 };
    }

    //�N�H�[�^�j�I���𐢊E���ɍ��킹��
    inline DirectX::XMFLOAT4 OrientationReset()
    {
        DirectX::XMFLOAT4 ori;
        DirectX::XMFLOAT4X4 standard = { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
        DirectX::XMMATRIX R = DirectX::XMLoadFloat4x4(&standard);
        DirectX::XMVECTOR O = DirectX::XMQuaternionRotationMatrix(R);
        DirectX::XMStoreFloat4(&ori, O);
        return ori;
    }


    inline DirectX::XMFLOAT3 HermiteFloat3(std::vector<DirectX::XMFLOAT3>& controllPoints, float ratio)
    {
        using namespace DirectX;
        const size_t size = controllPoints.size();

        if (size == 0)assert("Not ControllPoint");
        if (size == 1)return controllPoints.at(0);

        //-----< ���ǂ��̋�Ԃɂ��邩 >-----//
        std::vector<float> sectionLength;
        float length = 0.0f;
        //�e��Ԃ̒��������߂�
        for (int i = 0; i < size - 1; i++)
        {
            const DirectX::XMFLOAT3 v = {
        controllPoints.at(i + 1).x - controllPoints.at(i).x,
        controllPoints.at(i + 1).y - controllPoints.at(i).y,
        controllPoints.at(i + 1).z - controllPoints.at(i).z };
            const float l = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);

            sectionLength.emplace_back(l);
            length += l;
        }
        std::vector<float> sections;
        //�e��Ԃ̊��������߂�
        for (int i = 0; i < size - 1; i++)
        {
            sections.emplace_back(sectionLength.at(i) / length);
        }

        int sectionNum = 0;
        float r = 0.0f;
        for (sectionNum; sectionNum < sections.size(); sectionNum++)
        {
            r += sections.at(sectionNum);
            if ((ratio - r) <= 0.00001f)
            {
                break;
            }
        }

        XMFLOAT3 dummy = {
            2.0f * controllPoints.at(0).x - controllPoints.at(1).x,
            2.0f * controllPoints.at(0).y - controllPoints.at(1).y,
            2.0f * controllPoints.at(0).z - controllPoints.at(1).z,
        };
        const auto it = controllPoints.begin();
        controllPoints.insert(it, dummy);
        dummy = {
        2.0f * controllPoints.at(size - 1).x - controllPoints.at(size - 2).x,
        2.0f * controllPoints.at(size - 1).y - controllPoints.at(size - 2).y,
        2.0f * controllPoints.at(size - 1).z - controllPoints.at(size - 2).z,
        };
        controllPoints.emplace_back(dummy);

        sectionNum++;
        float sectionRatio = (ratio - (r - sections.at(sectionNum - 1))) / sections.at(sectionNum - 1);

        XMVECTOR Out;

        const float power = 1.0f; // Usually power is 0.5f
        XMVECTOR P0 = XMLoadFloat3(&controllPoints.at(sectionNum - 1));
        XMVECTOR P1 = XMLoadFloat3(&controllPoints.at(sectionNum ));
        XMVECTOR P2 = XMLoadFloat3(&controllPoints.at(sectionNum + 1));
        XMVECTOR P3 = XMLoadFloat3(&controllPoints.at(sectionNum + 2));
        XMVECTOR V0 = XMVectorScale(XMVectorSubtract(P2, P0), power);
        XMVECTOR V1 = XMVectorScale(XMVectorSubtract(P3, P1), power);

        Out = powf(sectionRatio, 3.0f) * (2.0f * P1 - 2.0f * P2 + V0 + V1);
        Out += powf(sectionRatio, 2.0f) * (-3.0f * P1 + 3.0f * P2 - 2.0f * V0 - V1);
        Out += sectionRatio * V0 + P1;


        XMFLOAT3 out{};
        XMStoreFloat3(&out, Out);

        return out;
    }




    //--------------------------------------------------------------
    //  �t�x�N�g����Ԃ�
    //--------------------------------------------------------------
    inline DirectX::XMFLOAT3 rev_vec(DirectX::XMVECTOR vec)
    {
        DirectX::XMFLOAT3 fin_vec;

        DirectX::XMVECTOR Rev = { -1,-1,-1 };

        DirectX::XMStoreFloat3(&fin_vec, DirectX::XMVectorMultiply(vec, Rev));
        return fin_vec;
    }

    inline DirectX::XMVECTOR rev_vec_v(DirectX::XMVECTOR vec)
    {
        DirectX::XMVECTOR Rev = { -1,-1,-1 };
        return     DirectX::XMVectorMultiply(vec, Rev);
    }

    inline float random_range(float min, float max)
    {
        // 0.0�`1.0�̊Ԃ܂ł̃����_���l
        float value = static_cast<float>(rand()) / RAND_MAX;
       
        // min�`max�܂ł̃����_���l�ɕϊ�
        return min + (max - min) * value;
    }

    //--------------------------------------------------------------
    //  �N�H�[�^�j�I�����m�̕ۊ�
    //--------------------------------------------------------------
    //  �����F�ۊǂ�������2��quaternion�A�⊮�x
    // �@�߂�l�F�ʒu
    //--------------------------------------------------------------
    inline DirectX::XMFLOAT4 QuaternionLerp(DirectX::XMFLOAT4 q1, DirectX::XMFLOAT4 q2, float rate)
    {
        DirectX::XMVECTOR Q1 = DirectX::XMLoadFloat4(&q1);
        DirectX::XMVECTOR Q2 = DirectX::XMLoadFloat4(&q2);

        Q1 = DirectX::XMQuaternionSlerp(Q1, Q2, rate);
        DirectX::XMFLOAT4 result_q;
        DirectX::XMStoreFloat4(&result_q, Q1);
        return result_q;

    }
    //--------------------------------------------------------------
    //  �I�u�W�F�N�g���~��ɔz�u
    //--------------------------------------------------------------
    //  �����Fcenter ���S�@radius�F���a�@index�F�ԍ��@divitions�F���� clockwise : ���я� add_ang : �����p�x
    // �@�߂�l�F�ʒu
    //--------------------------------------------------------------
    inline DirectX::XMFLOAT2 CircumferentialPlacement(DirectX::XMFLOAT2 center, float radius, int index, int divisions, bool clockwise = true, float add_angle = 0)
    {
        DirectX::XMFLOAT2 pos{};
        int clockwise_rot = clockwise ? 1 : -1;
        pos.x = center.x + radius * clockwise_rot * cosf(DirectX::XMConvertToRadians(add_angle + index * (360.0f / divisions)));
        pos.y = center.y + radius * clockwise_rot * sinf(DirectX::XMConvertToRadians(add_angle + index * (360.0f / divisions)));
        return pos;
    }

    inline DirectX::XMFLOAT3 CircumferentialPlacement(DirectX::XMFLOAT3 center, float radius, int index, int divisions, bool clockwise = true, float add_angle = 0)
    {
        DirectX::XMFLOAT3 pos{};
        int clockwise_rot = clockwise ? 1 : -1;
        pos.x = center.x + radius * clockwise_rot * cosf(DirectX::XMConvertToRadians(add_angle + index * (360.0f / divisions)));
        pos.y = center.y + radius * clockwise_rot * sinf(DirectX::XMConvertToRadians(add_angle + index * (360.0f / divisions)));
        pos.z = center.z + radius * clockwise_rot * sinf(DirectX::XMConvertToRadians(add_angle + index * (360.0f / divisions)));
        return pos;
    }

}
//--------------------------------------------------------------
//  strBit16    ������2�i���i16bit�j��string�ɕϊ�����
//--------------------------------------------------------------
//  �@�����Fconst int n     �ϊ����鐮��
//  �߂�l�Fstd::string     ���l��2�i���ɕϊ���������(string)
//--------------------------------------------------------------
inline std::string strBit16(const int n)
{
    std::stringstream ss;
    ss << static_cast<std::bitset<16>>(n);
    return ss.str();
}


 //imgui Menu

inline void imguiMenuBar(std::string menu_label, std::string menu_item_label, bool& selected)
{
#ifdef USE_IMGUI
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu(menu_label.c_str()))
        {
            ImGui::MenuItem(menu_item_label.c_str(), NULL, &selected);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
#endif // USE_IMGUI
}

inline void ImguiMenuAndSubBar(std::string menu_label, std::string menu_item_label, std::string sub_item_label, bool& selected)
{
#ifdef USE_IMGUI
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu(menu_label.c_str()))
        {
            if (ImGui::BeginMenu(menu_item_label.c_str()))
            {
                ImGui::MenuItem(sub_item_label.c_str(), NULL, &selected);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
#endif // USE_IMGUI
}



template <typename T>
inline void safe_delete(T*& p)
{
    if (p != nullptr)
    {
        delete(p);
        (p) = nullptr;
    }
}

template<typename T>
inline void safe_delete_array(T*& p)
{
    if (p != nullptr)
    {
        delete[](p);
        (p) = nullptr;
    }
}

template <typename T>
inline void safe_release(T*& p)
{
    if (p != nullptr)
    {
        (p)->Release();
        (p) = nullptr;
    }
}


class TimerComponent final
{
public:
    void StartTimer(float LimitTimer_)
    {
        mStackTimer = 0.0f;
        mLimitTime = LimitTimer_;
    }
    void fUpdate(float elapsedTime_)
    {
        mStackTimer += elapsedTime_;
    }
    [[nodiscard]] bool fGetOver()const
    {
        return mStackTimer > mLimitTime;
    }
private:
    float mStackTimer{};
    float mLimitTime{};
};

#pragma once
#include <DirectXMath.h>
#include <sstream>

// �q�b�g����
struct HitResult
{
    DirectX::XMFLOAT3 position = { 0,0,0 };   // ���C�ƃ|���S���̌�_
    DirectX::XMFLOAT3 normal = { 0,0,0 };   // �Փ˂����|���S���̖@���x�N�g��
    float distance = 0.0f;
    //int material_index = -1;
    std::string meshName;
    std::string materialName;
};

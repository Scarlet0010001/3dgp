#pragma once
#include <DirectXMath.h>
#include <sstream>

// ヒット結果
struct HitResult
{
    DirectX::XMFLOAT3 position = { 0,0,0 };   // レイとポリゴンの交点
    DirectX::XMFLOAT3 normal = { 0,0,0 };   // 衝突したポリゴンの法線ベクトル
    float distance = 0.0f;
    //int material_index = -1;
    std::string meshName;
    std::string materialName;
};

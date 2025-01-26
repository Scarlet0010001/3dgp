#pragma once
#include "sprite_batch.h"
#include "graphics.h"
class UI
{
public:
    //--------<constructor/destructor>--------//
    UI() {}
    virtual ~UI() {}
    //--------< ŠÖ” >--------//
    virtual void Update(float elapsed_time) = 0;
    virtual void Render(ID3D11DeviceContext* dc) = 0;
    //--------< \‘¢‘Ì >--------//
    struct Element
    {
        DirectX::XMFLOAT2 position{};
        DirectX::XMFLOAT2 scale{ 1, 1 };
        DirectX::XMFLOAT2 pivot{};
        DirectX::XMFLOAT4 color{ 1,1,1,1 };
        float angle{};
        DirectX::XMFLOAT2 texpos{};
        DirectX::XMFLOAT2 texsize{};
    };
};
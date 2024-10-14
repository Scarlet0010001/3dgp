#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <cstdint>
#include <DirectXMath.h>

#include "misc.h"

enum class CB_FLAG : uint8_t
{
    PS = 0x01,
    VS = 0x02,
    CS = 0x03,
    HS = 0x04,
    DS = 0x05,
    GS = 0x06,
    PS_VS = PS | VS,
    PS_CS = PS | CS,
    PS_GS = PS | GS,
    PS_HS = PS | HS,
    PS_DS = PS | DS,
    VS_CS = VS | CS,
    VS_GS = VS | GS,
    VS_HS = VS | HS,
    VS_DS = VS | DS,
    CS_HS = CS | HS,
    CS_DS = CS | DS,
    CS_GS = CS | GS,
    HS_DS = HS | DS,
    HS_GS = HS | GS,
    DS_GS = DS | GS,
    PS_VS_CS = PS | VS | CS,
    PS_VS_HS = PS | VS | HS,
    PS_VS_DS = PS | VS | DS,
    PS_VS_GS = PS | VS | GS,
    PS_CS_HS = PS | CS | HS,
    PS_CS_DS = PS | CS | DS,
    PS_CS_GS = PS | CS | GS,
    PS_HS_DS = PS | HS | DS,
    PS_HS_GS = PS | HS | GS,
    PS_DS_GS = PS | DS | GS,
    VS_CS_GS = VS | CS | GS,
    PS_VS_CS_HS = PS | VS | CS | HS,
    PS_VS_CS_DS = PS | VS | CS | DS,
    PS_VS_CS_GS = PS | VS | CS | GS,
    PS_VS_HS_DS = PS | VS | HS | DS,
    PS_VS_HS_GS = PS | VS | HS | GS,
    PS_VS_DS_GS = PS | VS | DS | GS,
    PS_CS_HS_DS = PS | CS | HS | DS,
    PS_CS_DS_GS = PS | CS | DS | GS,
    PS_VS_CS_HS_DS = PS | VS | CS | HS | DS,
    PS_VS_CS_HS_GS = PS | VS | CS | HS | GS,
    PS_CS_HS_DS_GS = PS | CS | HS | DS | GS,
    ALL = PS | VS | CS | HS | DS | GS,
};

inline bool operator&(CB_FLAG lhs, CB_FLAG rhs)
{
    return static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs);
}

template <class T>
class Constants
{
public:
    //--------<constructor/destructor>--------//
    Constants(ID3D11Device* device)
    {
        _ASSERT_EXPR(sizeof(T) % 16 == 0, L"constant buffer's need to be 16 byte aligned");
        HRESULT hr{ S_OK };
        D3D11_BUFFER_DESC buffer_desc{};
        buffer_desc.ByteWidth = sizeof(T);
        buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        buffer_desc.CPUAccessFlags = 0;
        buffer_desc.MiscFlags = 0;
        buffer_desc.StructureByteStride = 0;
        hr = device->CreateBuffer(&buffer_desc, nullptr, constantBuffer.ReleaseAndGetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }
    ~Constants() {}
    //--------< 関数 >--------//
    //定数バッファのセット
    void Bind(ID3D11DeviceContext* dc, UINT slot, CB_FLAG flags = CB_FLAG::PS)
    {
        dc->UpdateSubresource(constantBuffer.Get(), 0, 0, &data, 0, 0);
        if (flags & CB_FLAG::PS) dc->PSSetConstantBuffers(slot, 1, constantBuffer.GetAddressOf());
        if (flags & CB_FLAG::VS) dc->VSSetConstantBuffers(slot, 1, constantBuffer.GetAddressOf());
        if (flags & CB_FLAG::CS) dc->CSSetConstantBuffers(slot, 1, constantBuffer.GetAddressOf());
        if (flags & CB_FLAG::HS) dc->HSSetConstantBuffers(slot, 1, constantBuffer.GetAddressOf());
        if (flags & CB_FLAG::DS) dc->DSSetConstantBuffers(slot, 1, constantBuffer.GetAddressOf());
        if (flags & CB_FLAG::GS) dc->GSSetConstantBuffers(slot, 1, constantBuffer.GetAddressOf());
    }

    //外部でデータを作って代入したいときに使う
    void DataSet(T add_data)
    {
        data = add_data;
    }
    //--------< 変数 >--------//
    T data;
private:
    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;
};
#include "include/Renderer/DirectX11/DX11IndexBuffer.h"

namespace JonsEngine
{
    DX11IndexBuffer::DX11IndexBuffer(ID3D11DevicePtr device, ID3D11DeviceContextPtr context, const std::vector<uint16_t>& indexData)
    {
        D3D11_BUFFER_DESC bufferDescription;
        ZeroMemory(&bufferDescription, sizeof(D3D11_BUFFER_DESC));
        bufferDescription.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDescription.ByteWidth = indexData.size() * sizeof(uint16_t);
        bufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA initData;
        ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
        initData.pSysMem = &indexData.at(0);
        DXCALL(device->CreateBuffer(&bufferDescription, &initData, &mIndexBuffer));
    }

    DX11IndexBuffer::~DX11IndexBuffer()
    {
    }


    void DX11IndexBuffer::Bind()
    {

    }
}
#include "include/Renderer/DirectX11/DX11FullscreenTrianglePass.h"

#include "include/Renderer/DirectX11/Shaders/Compiled/FullscreenTriangleVertex.h"

namespace JonsEngine
{
    DX11FullscreenTrianglePass::DX11FullscreenTrianglePass(ID3D11DevicePtr device, ID3D11DeviceContextPtr context) :
        mContext(context), mVertexShader(nullptr)
    {
        DXCALL(device->CreateVertexShader(gFullscreenTriangleVertexShader, sizeof(gFullscreenTriangleVertexShader), nullptr, &mVertexShader));
    }

    DX11FullscreenTrianglePass::~DX11FullscreenTrianglePass()
    {
    }


    void DX11FullscreenTrianglePass::Render()
    {
        mContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        mContext->IASetInputLayout(nullptr);

        mContext->VSSetShader(mVertexShader, nullptr, 0);

        mContext->Draw(3, 0);
    }
}
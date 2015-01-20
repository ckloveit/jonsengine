#include "include/Renderer/DirectX11/DX11AABBPass.h"

#include "include/Renderer/DirectX11/DX11Utils.h"
#include "include/Renderer/DirectX11/DX11VertexTransformPass.h"
#include "include/Renderer/DirectX11/Shaders/Compiled/SimpleColorPixel.h"

namespace JonsEngine
{
    DX11AABBPass::DX11AABBPass(ID3D11DevicePtr device, ID3D11DeviceContextPtr context, DX11VertexTransformPass& vertexTransformPass) :
        mContext(context), mPixelShader(nullptr), mVertexTransformPass(vertexTransformPass)
    {
        DXCALL(device->CreatePixelShader(gSimpleColorPixelShader, sizeof(gSimpleColorPixelShader), NULL, &mPixelShader));
    }
    
    DX11AABBPass::~DX11AABBPass()
    {
    }


    void DX11AABBPass::Render(const RenderQueue& renderQueue, const Mat4& viewProjectionMatrix)
    {
        mContext->PSSetShader(mPixelShader, NULL, NULL);
        mVertexTransformPass.BindForTransformPass(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

        mVertexTransformPass.RenderAABBs(renderQueue, viewProjectionMatrix);
    }
}
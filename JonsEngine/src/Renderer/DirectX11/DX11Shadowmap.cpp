#include "include/Renderer/DirectX11/DX11Shadowmap.h"

#include "include/Renderer/DirectX11/DX11Texture.h"
#include "include/Renderer/DirectX11/Shaders/Compiled/TransformVertex.h"

namespace JonsEngine
{
    DX11Shadowmap::DX11Shadowmap(ID3D11DevicePtr device, ID3D11DeviceContextPtr context, const EngineSettings::ShadowResolution shadowmapRes, const uint32_t numTextures, const bool isCubeTexture) :
        mContext(context), mShadowmapTexture(nullptr), mInputLayout(nullptr), mShadowmapSRV(nullptr)
    {
        const uint32_t shadowmapResVal = EngineSettingsToVal(shadowmapRes);

        mShadowmapViews.resize(numTextures);

        D3D11_INPUT_ELEMENT_DESC inputDescription;
        ZeroMemory(&inputDescription, sizeof(D3D11_INPUT_ELEMENT_DESC));
        inputDescription.SemanticName = "POSITION";
        inputDescription.SemanticIndex = 0;
        inputDescription.Format = DXGI_FORMAT_R32G32B32_FLOAT;
        inputDescription.InputSlot = 0;
        inputDescription.AlignedByteOffset = 0;
        inputDescription.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        inputDescription.InstanceDataStepRate = 0;
        DXCALL(device->CreateInputLayout(&inputDescription, 1, gTransformVertexShader, sizeof(gTransformVertexShader), &mInputLayout));

        // create shadowmap texture/view/srv
        D3D11_TEXTURE2D_DESC depthBufferDesc;
        ZeroMemory(&depthBufferDesc, sizeof(D3D11_TEXTURE2D_DESC));
        depthBufferDesc.ArraySize = numTextures;
        depthBufferDesc.Format = DXGI_FORMAT_R32_TYPELESS;
        depthBufferDesc.Width = shadowmapResVal;
        depthBufferDesc.Height = shadowmapResVal;
        depthBufferDesc.MipLevels = 1;
        depthBufferDesc.SampleDesc.Count = 1;
        depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
        depthBufferDesc.MiscFlags = isCubeTexture ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;
        DXCALL(device->CreateTexture2D(&depthBufferDesc, nullptr, &mShadowmapTexture));

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        ZeroMemory(&dsvDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
        dsvDesc.Texture2DArray.ArraySize = 1;
        for (uint32_t face = 0; face < numTextures; face++)
        {
            dsvDesc.Texture2DArray.FirstArraySlice = face;
            DXCALL(device->CreateDepthStencilView(mShadowmapTexture, &dsvDesc, &mShadowmapViews.at(face)));
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;

        if (isCubeTexture)
        {
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
            srvDesc.TextureCube.MipLevels = 1;
            DXCALL(device->CreateShaderResourceView(mShadowmapTexture, &srvDesc, &mShadowmapSRV));
        }
        else
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
            ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
            srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
            srvDesc.Texture2DArray.ArraySize = numTextures;
            srvDesc.Texture2DArray.MipLevels = 1;
            DXCALL(device->CreateShaderResourceView(mShadowmapTexture, &srvDesc, &mShadowmapSRV));
        }

        // viewport used during shadow pass
        ZeroMemory(&mShadowPassViewport, sizeof(D3D11_VIEWPORT));
        mShadowPassViewport.TopLeftX = 0;
        mShadowPassViewport.TopLeftY = 0;
        mShadowPassViewport.Width = static_cast<float>(shadowmapResVal);
        mShadowPassViewport.Height = static_cast<float>(shadowmapResVal);
        mShadowPassViewport.MinDepth = 0.0f;
        mShadowPassViewport.MaxDepth = 1.0f;
    }

    DX11Shadowmap::~DX11Shadowmap()
    {
    }


    void DX11Shadowmap::BindForDrawing()
    {
        mContext->RSSetViewports(1, &mShadowPassViewport);

        // unbind shadowmap as shader resource
        mContext->PSSetShaderResources(DX11Texture::SHADER_TEXTURE_SLOT_EXTRA, 1, &gNullSRV.p);

        // defaults to depth rendering/testing
        mContext->OMSetDepthStencilState(nullptr, 0);
    }

    void DX11Shadowmap::BindDepthView(const uint32_t depthViewIndex)
    {
        mContext->OMSetRenderTargets(0, nullptr, mShadowmapViews.at(depthViewIndex));
        mContext->ClearDepthStencilView(mShadowmapViews.at(depthViewIndex), D3D11_CLEAR_DEPTH, 1.0f, 0);
    }

    void DX11Shadowmap::BindForReading()
    {
        mContext->PSSetShaderResources(DX11Texture::SHADER_TEXTURE_SLOT_EXTRA, 1, &mShadowmapSRV.p);
    }


    uint32_t DX11Shadowmap::GetTextureSize() const
    {
        D3D11_TEXTURE2D_DESC depthBufferDesc;
        ZeroMemory(&depthBufferDesc, sizeof(D3D11_TEXTURE2D_DESC));

        mShadowmapTexture->GetDesc(&depthBufferDesc);

        // same size for height/width
        return depthBufferDesc.Width;
    }
}
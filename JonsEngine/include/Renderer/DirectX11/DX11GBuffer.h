#pragma once

#include "include/Renderer/DirectX11/DX11ConstantBuffer.hpp"
#include "include/Renderer/DirectX11/DX11Utils.h"
#include "include/Core/Types.h"

#include <d3d11.h>
#include <array>

namespace JonsEngine
{
    class DX11GBuffer
    {
    public:
        enum GBUFFER_RENDERTARGET_INDEX
        {
            GBUFFER_RENDERTARGET_INDEX_DIFFUSE,
            GBUFFER_RENDERTARGET_INDEX_NORMAL,
            GBUFFER_NUM_RENDERTARGETS,
        };


        DX11GBuffer(ID3D11DevicePtr device, ID3D11DepthStencilViewPtr lightAccumDSV, uint32_t textureWidth, uint32_t textureHeight);
        ~DX11GBuffer();

        void SetConstantData(ID3D11DeviceContextPtr context, const Mat4& wvpMatrix, const Mat4& worldMatrix, const float textureTilingFactor, const bool hasDiffuseTexture, const bool hasNormalTexture);
        void BindForGeometryStage(ID3D11DeviceContextPtr context);
        void BindGeometryTextures(ID3D11DeviceContextPtr context);


    private:
        struct GBufferCBuffer
        {
            Mat4 mWVPMatrix;
            Mat4 mWorldMatrix;
            float mTextureTilingFactor;
            uint32_t mHasDiffuseTexture;
            uint32_t mHasNormalTexture;
            float __padding;

            GBufferCBuffer(const Mat4& wvpMatrix, const Mat4& worldMatrix, const float textureTilingFactor, const bool hasDiffuse, const bool hasNormal) :
                mWVPMatrix(wvpMatrix), mWorldMatrix(worldMatrix), mTextureTilingFactor(textureTilingFactor), mHasDiffuseTexture(hasDiffuse), mHasNormalTexture(hasNormal)
            {
            }
        };


        std::array<ID3D11Texture2DPtr, DX11GBuffer::GBUFFER_NUM_RENDERTARGETS> mTextures;
        std::array<ID3D11RenderTargetViewPtr, DX11GBuffer::GBUFFER_NUM_RENDERTARGETS> mRenderTargets;
        std::array<ID3D11ShaderResourceViewPtr, DX11GBuffer::GBUFFER_NUM_RENDERTARGETS> mShaderResourceViews;
        ID3D11InputLayoutPtr mInputLayout;
        ID3D11VertexShaderPtr mVertexShader;
        ID3D11PixelShaderPtr mPixelShader;
        DX11ConstantBuffer<GBufferCBuffer> mConstantBuffer;
        ID3D11DepthStencilViewPtr mDSV;
    };
}

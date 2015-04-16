#pragma once

#include "include/Renderer/DirectX11/DX11ConstantBuffer.hpp"
#include "include/Renderer/DirectX11/DX11Utils.h"
#include "include/Renderer/DirectX11/DX11Mesh.h"
#include "include/Renderer/DirectX11/DX11Shadowmap.h"
#include "include/Renderer/DirectX11/DX11RenderTarget2D.h"
#include "include/Core/Types.h"

#include <d3d11.h>
#include <array>
#include <vector>

namespace JonsEngine
{
    class DX11FullscreenTrianglePass;
    class DX11VertexTransformPass;
	struct RenderableDirLight;

    class DX11DirectionalLightPass
    {
    public:
        const static uint32_t NUM_SHADOWMAP_CASCADES = 4;

        DX11DirectionalLightPass(ID3D11DevicePtr device, ID3D11DeviceContextPtr context, DX11FullscreenTrianglePass& fullscreenPass, DX11VertexTransformPass& transformPass, const uint32_t shadowmapSize,
            const uint32_t windowWidth, const uint32_t windowHeight);
        ~DX11DirectionalLightPass();

        void Render(const RenderableDirLight& directionalLight, const float degreesFOV, const float aspectRatio, const Mat4& cameraViewMatrix, const Mat4& invCameraProjMatrix, const Vec2& windowSize, const bool drawFrustrums);


    private:
        struct DirectionalLightCBuffer
        {
            std::array<Mat4, NUM_SHADOWMAP_CASCADES> mSplitVPMatrices;
            Mat4 mInvProjMatrix;
            std::array<float, NUM_SHADOWMAP_CASCADES> mSplitDistances;
            Vec4 mLightColor;
            Vec4 mLightDirection;
            Vec2 mWindowSize;
            float mShadowmapSize;
            float __padding;


            DirectionalLightCBuffer(const std::array<Mat4, NUM_SHADOWMAP_CASCADES>& splitMatrices, const Mat4& invProjMatrix, const std::array<float, NUM_SHADOWMAP_CASCADES>& splitDistances, const Vec4& lightColor, const Vec4& lightDir, const Vec2& windowSize, const float shadowmapSize) :
                mSplitVPMatrices(splitMatrices), mInvProjMatrix(invProjMatrix), mSplitDistances(splitDistances), mLightColor(lightColor), mLightDirection(lightDir), mWindowSize(windowSize), mShadowmapSize(shadowmapSize)
            {
            }
        };

        Vec2 ReduceDepth();

        ID3D11DeviceContextPtr mContext;
        ID3D11PixelShaderPtr mPixelShader;
        ID3D11ComputeShaderPtr mDepthReductionInitialShader;
        ID3D11ComputeShaderPtr mDepthReductionFinalShader;
        ID3D11RasterizerStatePtr mRSDepthClamp;
        std::vector<DX11RenderTarget2D> mDepthReductionRTVs;

        DX11FullscreenTrianglePass& mFullscreenPass;
        DX11VertexTransformPass& mVertexTransformPass;
        DX11Shadowmap mShadowmap;
        DX11ConstantBuffer<DirectionalLightCBuffer> mDirLightCBuffer;
    };
}

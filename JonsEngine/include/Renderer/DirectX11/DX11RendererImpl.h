#pragma once

#include "include/Renderer/RenderCommands.h"
#include "include/Renderer/DirectX11/DX11Mesh.h"
#include "include/Renderer/DirectX11/DX11Texture.h"
#include "include/Renderer/DirectX11/DX11Context.h"
#include "include/Renderer/DirectX11/DX11Pipeline.h"
#include "include/Renderer/DirectX11/DX11GBuffer.h"
#include "include/Renderer/DirectX11/DX11FullscreenTrianglePass.h"
#include "include/Renderer/DirectX11/DX11VertexTransformPass.h"
#include "include/Renderer/DirectX11/DX11AmbientPass.h"
#include "include/Renderer/DirectX11/DX11DirectionalLightPass.h"
#include "include/Renderer/DirectX11/DX11PointLightPass.h"
#include "include/Renderer/DirectX11/DX11PostProcessor.h"
#include "include/Renderer/DirectX11/DX11AABBPass.h"
#include "include/Renderer/DirectX11/DX11Sampler.h"
#include "include/Renderer/DirectX11/DX11Utils.h"
#include "include/Core/Types.h"
#include "include/Core/EngineSettings.h"
#include "include/Core/Memory/HeapAllocator.h"
#include "include/Core/DebugOptions.h"

#include <windows.h>
#include <d3d11.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>


namespace JonsEngine
{
    class Logger;

    class DX11RendererImpl : protected DX11Context
    {
    public:
        DX11RendererImpl(const EngineSettings& settings, Logger& logger, IMemoryAllocatorPtr memoryAllocator);
        ~DX11RendererImpl();

        MeshID CreateMesh(const std::vector<float>& vertexData, const std::vector<float>& normalData, const std::vector<float>& texCoords, const std::vector<float>& tangents,
            const std::vector<float>& bitangents, const std::vector<uint16_t>& indexData, const Vec3& minBounds, const Vec3& maxBounds);
        TextureID CreateTexture(TextureType textureType, const std::vector<uint8_t>& textureData, uint32_t textureWidth, uint32_t textureHeight);
        void Render(const RenderQueue& renderQueue, const RenderableLighting& lighting, const DebugOptions::RenderingFlags debugFlags);

        EngineSettings::Anisotropic GetAnisotropicFiltering() const;
        void SetAnisotropicFiltering(const EngineSettings::Anisotropic anisotropic);

        EngineSettings::AntiAliasing GetAntiAliasing() const;
        void SetAntiAliasing(const EngineSettings::AntiAliasing aa);

        bool IsSSAOEnabled() const;
        void SetSSAO(const bool useSSAO);

        float GetZNear() const;
        float GetZFar() const;
        EngineSettings::ShadowQuality GetShadowQuality() const;


    private:
        static LRESULT CALLBACK DX11RendererImpl::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
        void SetupContext(const uint32_t viewportWidth, const uint32_t viewportHeight);
        void GeometryStage(const RenderQueue& renderQueue, const Mat4& viewMatrix);
        void LightingStage(const RenderQueue& renderQueue, const RenderableLighting& lighting, const DebugOptions::RenderingFlags debugFlags);
        void PostProcessingStage(const RenderQueue& renderQueue, const RenderableLighting& lighting, const DebugOptions::RenderingFlags debugFlags);

        Logger& mLogger;
        IMemoryAllocatorPtr mMemoryAllocator;
        EngineSettings::ShadowQuality mShadowQuality;
        EngineSettings::AntiAliasing mAntiAliasing;

        DX11VertexTransformPass mVertexTransformPass;
        DX11FullscreenTrianglePass mFullscreenTrianglePass;
        //DX11LightAccumulationbuffer mLightingAccBuffer;
        //DX11Backbuffer mBackbuffer;
        DX11Pipeline mPipeline;
        DX11GBuffer mGBuffer;
        DX11AmbientPass mAmbientPass;
        DX11DirectionalLightPass mDirectionalLightPass;
        DX11PointLightPass mPointLightPass;
        DX11PostProcessor mPostProcessor;
        DX11AABBPass mAABBPass;
        ID3D11RasterizerStatePtr mDefaultRasterizerState;
        ID3D11BlendStatePtr mAdditiveBlending;
        std::unique_ptr<DX11Sampler, std::function<void(DX11Sampler*)>> mModelSampler;
        DX11Sampler mShadowmapSampler;
        DX11Sampler mShadowmapNoCompareSampler;
        bool mSSAOEnabled;

        std::vector<DX11MeshPtr> mMeshes;
        std::vector<DX11TexturePtr> mTextures;
    };
}
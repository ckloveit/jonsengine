#pragma once

#include "Renderer/DirectX11/DX11Utils.h"
#include "Core/Types.h"
#include "Core/EngineSettings.h"
#include "Core/Platform.h"

#include <vector>

namespace JonsEngine
{
    class DX11Shadowmap
    {
    public:
        DX11Shadowmap(ID3D11DevicePtr device, ID3D11DeviceContextPtr context, const RenderSettings::ShadowResolution shadowmapRes, const uint32_t numTextures, const bool isCubeTexture);
        ~DX11Shadowmap();

        void BindForDrawing();
        void BindDepthView(const uint32_t depthViewIndex);
        void BindForReading();

        uint32_t GetTextureSize() const;


    private:
        ID3D11DeviceContextPtr mContext;
        ID3D11Texture2DPtr mShadowmapTexture;
        ID3D11ShaderResourceViewPtr mShadowmapSRV;

        std::vector<ID3D11DepthStencilViewPtr> mShadowmapViews;
        D3D11_VIEWPORT mShadowPassViewport;
    };
}
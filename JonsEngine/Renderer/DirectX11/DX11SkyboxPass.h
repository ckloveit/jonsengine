#pragma once

#include "Renderer/DirectX11/DX11Utils.h"
#include "Core/Types.h"
#include "Core/Platform.h"

namespace JonsEngine
{
    class DX11Texture;

    class DX11SkyboxPass
    {
    public:
        DX11SkyboxPass(ID3D11DevicePtr device, ID3D11DeviceContextPtr context);
        ~DX11SkyboxPass();

        void Render(DX11Texture& skyboxTexture);


    private:
        ID3D11DeviceContextPtr mContext;
        ID3D11VertexShaderPtr mVertexShader;
        ID3D11PixelShaderPtr mPixelShader;
    };
}

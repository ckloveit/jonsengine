#pragma once

#include "include/Renderer/DirectX11/Shaders/Constants.h"
#include "include/Core/Logging/Logger.h"
#include "include/Core/EngineSettings.h"

#include "boost/current_function.hpp"
#include <d3d11.h>
#include <atlbase.h>
#include <sstream>
#include <exception>
#include <array>

namespace JonsEngine
{
    // TODO: make dynamic?
    const float Z_NEAR = 0.1f;
    const float Z_FAR = 100.0f;

    typedef std::array<float, 4> DX11Color;

    enum SHADER_TEXTURE_SLOT
    {
        SHADER_TEXTURE_SLOT_DIFFUSE = TEXTURE_SLOT_DIFFUSE,
        SHADER_TEXTURE_SLOT_NORMAL = TEXTURE_SLOT_NORMAL,
        SHADER_TEXTURE_SLOT_DEPTH = TEXTURE_SLOT_DEPTH,
        SHADER_TEXTURE_SLOT_EXTRA = TEXTURE_SLOT_EXTRA,
        NUM_SHADER_TEXTURE_SLOTS
    };

    uint32_t EngineSettingsToVal(const EngineSettings::Anisotropic anisotropicEnum);
    uint32_t EngineSettingsToVal(const EngineSettings::ShadowReadbackLatency shadowReadbackLatencyEnum);
    uint32_t EngineSettingsToVal(const EngineSettings::ShadowResolution shadowResolutionEnum);

    #ifdef _DEBUG
    #define DXCALL(function)                                                                                                                            \
    {                                                                                                                                                   \
        HRESULT result = function;                                                                                                                      \
        if (FAILED(result))                                                                                                                             \
        {                                                                                                                                               \
            std::stringstream errorSS;                                                                                                                  \
            errorSS << "D3D11 Error: (" << __FILE__ << ":" << __LINE__ << ") in " << BOOST_CURRENT_FUNCTION << ": code " << result;                     \
            Logger::GetRendererLogger().Log(LogManager::LogLevel::Error, errorSS.str());                                                                    \
            throw new std::runtime_error(errorSS.str());                                                                                                \
        }                                                                                                                                               \
    }
    #else
    #define DXCALL(function) function;
    #endif

    // COM smart pointers
    typedef CComPtr<ID3D11Buffer> ID3D11BufferPtr;
    typedef CComPtr<ID3D11VertexShader> ID3D11VertexShaderPtr;
    typedef CComPtr<ID3D11PixelShader> ID3D11PixelShaderPtr;
    typedef CComPtr<ID3D11ComputeShader> ID3D11ComputeShaderPtr;
    typedef CComPtr<ID3D11RasterizerState> ID3D11RasterizerStatePtr;
    typedef CComPtr<ID3D11BlendState> ID3D11BlendStatePtr;
    typedef CComPtr<ID3D11SamplerState> ID3D11SamplerStatePtr;
    typedef CComPtr<ID3D11RenderTargetView> ID3D11RenderTargetViewPtr;
    typedef CComPtr<ID3D11DepthStencilState> ID3D11DepthStencilStatePtr;
    typedef CComPtr<ID3D11DepthStencilView> ID3D11DepthStencilViewPtr;
    typedef CComPtr<IDXGISwapChain> IDXGISwapChainPtr;
    typedef CComPtr<ID3D11Device> ID3D11DevicePtr;
    typedef CComPtr<ID3D11DeviceContext> ID3D11DeviceContextPtr;
    typedef CComPtr<ID3D11Texture2D> ID3D11Texture2DPtr;
    typedef CComPtr<ID3D11ShaderResourceView> ID3D11ShaderResourceViewPtr;
    typedef CComPtr<ID3D11UnorderedAccessView> ID3D11UnorderedAccessViewPtr;
    typedef CComPtr<ID3D11InputLayout> ID3D11InputLayoutPtr;

    static ID3D11ShaderResourceViewPtr gNullSRV = nullptr;
	static ID3D11UnorderedAccessViewPtr gNullUAV = nullptr;
}

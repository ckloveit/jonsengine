#include "Renderer/DirectX11/DX11TerrainPass.h"

#include "Renderer/DirectX11/DX11Texture.h"
#include "Renderer/DirectX11/DX11VertexTransformPass.h"
#include "Renderer/RenderSettings.h"
#include "Compiled/TerrainVertex.h"
#include "Compiled/TerrainHull.h"
#include "Compiled/TerrainComputeCoplanarity16.h"
#include "Compiled/TerrainComputeCoplanarity32.h"
#include "Compiled/TerrainComputeCoplanarity64.h"
#include "Compiled/TerrainDomain.h"
#include "Compiled/TerrainPixel.h"
#include "Compiled/TerrainPixelDebug.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/AABB.h"

namespace JonsEngine
{
	/*const std::vector<float> gQuadVertices{
		-1.0f, 0.0f, -1.0f,
		1.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 1.0f,
		-1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 3.0f,
		-1.0f, 0.0f, 3.0f,
		-1.0f, 0.0f, -3.0f,
		1.0f, 0.0f, -3.0f,
		3.0f, 0.0f, -1.0f,
		3.0f, 0.0f, 1.0f,
		-3.0f, 0.0f, 1.0f,
		-3.0f, 0.0f, -1.0f,
	};*/

	const std::vector<float> gQuadVertices{
		-1.0f, 0.0f, -1.0f,
		1.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 1.0f,
		-1.0f, 0.0f, 1.0f,
		-1.0f, 0.0f, -3.0f,
		1.0f, 0.0f, -3.0f,
		3.0f, 0.0f, -1.0f,
		3.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 3.0f,
		-1.0f, 0.0f, 3.0f,
		-3.0f, 0.0f, 1.0f,
		-3.0f, 0.0f, -1.0f,
	};

	const std::vector<uint16_t> gQuadIndices{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

	enum VSInputLayout
	{
		POSITION = 0,
		NUM_INPUT_LAYOUTS
	};

    void GetCoplanarityTextureOutputDimensions( uint32_t& width, uint32_t& height, const DX11Texture& heightTexture, RenderSettings::TerrainPatchSize patchSize )
    {
        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
        heightTexture.GetDesc( desc );

        uint32_t patchSizeVal = RenderSettingsToVal( patchSize );
        assert( patchSizeVal > 0 );

        width = desc.Width / patchSizeVal;
        height = desc.Height / patchSizeVal;
    }


	DX11TerrainPass::DX11TerrainPass(ID3D11DevicePtr device, ID3D11DeviceContextPtr context, DX11VertexTransformPass& vertexTransformer, const IDMap<DX11Texture>& textureMap, RenderSettings::TerrainPatchSize patchSize) :
		mContext(context),
        mDevice(device),

		mCBuffer(device, context, mCBuffer.CONSTANT_BUFFER_SLOT_DOMAIN),
		mPerTerrainCBuffer(device, context, mPerTerrainCBuffer.CONSTANT_BUFFER_SLOT_EXTRA),
		mTransformsBuffer(device, context),
		mQuadMesh(device, context, gQuadVertices, gQuadIndices, AABB::gUnitQuadAABB.Min(), AABB::gUnitQuadAABB.Max()),
		mVertexTransformer(vertexTransformer),
		mTextureMap(textureMap),

		mPatchSize(patchSize)
	{
		// input layout
		// TODO: generalize into one class
		D3D11_INPUT_ELEMENT_DESC inputDescription[NUM_INPUT_LAYOUTS];
		ZeroMemory(&inputDescription, sizeof(D3D11_INPUT_ELEMENT_DESC) * NUM_INPUT_LAYOUTS);

		inputDescription[VSInputLayout::POSITION].SemanticName = "POSITION";
		inputDescription[VSInputLayout::POSITION].SemanticIndex = 0;
		inputDescription[VSInputLayout::POSITION].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		inputDescription[VSInputLayout::POSITION].InputSlot = DX11Mesh::VERTEX_BUFFER_SLOT_POSITIONS;
		inputDescription[VSInputLayout::POSITION].AlignedByteOffset = 0;
		inputDescription[VSInputLayout::POSITION].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputDescription[VSInputLayout::POSITION].InstanceDataStepRate = 0;
		DXCALL(device->CreateInputLayout(inputDescription, VSInputLayout::NUM_INPUT_LAYOUTS, gTerrainVertex, sizeof(gTerrainVertex), &mLayout));

		// shaders
		DXCALL(device->CreateVertexShader(gTerrainVertex, sizeof(gTerrainVertex), nullptr, &mVertexShader));
		DXCALL(device->CreateHullShader(gTerrainHull, sizeof(gTerrainHull), nullptr, &mHullShader));
        DXCALL(device->CreateComputeShader(gTerrainComputeCoplanarity16, sizeof(gTerrainComputeCoplanarity16), nullptr, &mCoplanarityComputeShader16));
        DXCALL(device->CreateComputeShader(gTerrainComputeCoplanarity32, sizeof(gTerrainComputeCoplanarity32), nullptr, &mCoplanarityComputeShader32));
        DXCALL(device->CreateComputeShader(gTerrainComputeCoplanarity64, sizeof(gTerrainComputeCoplanarity64), nullptr, &mCoplanarityComputeShader64));
		DXCALL(device->CreateDomainShader(gTerrainDomain, sizeof(gTerrainDomain), nullptr, &mDomainShader));
		DXCALL(device->CreatePixelShader(gTerrainPixel, sizeof(gTerrainPixel), nullptr, &mPixelShader));
		DXCALL( device->CreatePixelShader( gTerrainPixelDebug, sizeof( gTerrainPixelDebug ), nullptr, &mPixelDebugShader ) );

		D3D11_RASTERIZER_DESC rasterizerDesc;
		ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
		rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
		rasterizerDesc.CullMode = D3D11_CULL_BACK;
		rasterizerDesc.FrontCounterClockwise = true;
		rasterizerDesc.DepthClipEnable = true;
		rasterizerDesc.ScissorEnable = false;
		rasterizerDesc.MultisampleEnable = false;
		rasterizerDesc.AntialiasedLineEnable = false;
		DXCALL(device->CreateRasterizerState(&rasterizerDesc, &mDebugRasterizer));
	}

	void DX11TerrainPass::Render( const RenderableTerrains& terrains )
	{
		if ( !terrains.GetNumTerrains() )
			return;

		BindForRendering();
		RenderInternal( terrains );
		UnbindRendering();
	}

	void DX11TerrainPass::RenderDebug( const RenderableTerrains& terrains )
	{
		if ( !terrains.GetNumTerrains() )
			return;

		BindForRendering();

		mContext->PSSetShader( mPixelDebugShader, nullptr, 0 );
		ID3D11RasterizerStatePtr prevRasterizer = nullptr;
		mContext->RSGetState( &prevRasterizer );
		mContext->RSSetState( mDebugRasterizer );

		RenderInternal( terrains );

		mContext->RSSetState( prevRasterizer );

		UnbindRendering();
	}

    void DX11TerrainPass::CreateCoplanarityMap( DX11TextureID heightmapID )
    {
        uint32_t width, height;
        GetCoplanarityTextureOutputDimensions( width, height, mTextureMap.GetItem( heightmapID ), mPatchSize );

        mTerrainCoplanarityMap.erase( heightmapID );
        mTerrainCoplanarityMap.emplace( std::piecewise_construct, std::forward_as_tuple( heightmapID ), std::forward_as_tuple( mDevice, DXGI_FORMAT_R16_UNORM, width, height, true ) );

        // coplanarity value between [0, 1]
        /*D3D11_TEXTURE2D_DESC coplanarityTextureDesc;
        ZeroMemory(&coplanarityTextureDesc, sizeof(D3D11_TEXTURE2D_DESC));
        coplanarityTextureDesc.ArraySize = 1;
        coplanarityTextureDesc.Format = DXGI_FORMAT_R16_UNORM;
        coplanarityTextureDesc.Width = width;
        coplanarityTextureDesc.Height = height;
        coplanarityTextureDesc.MipLevels = 1;
        coplanarityTextureDesc.SampleDesc.Count = 1;
        coplanarityTextureDesc.Usage = D3D11_USAGE_DEFAULT;
        coplanarityTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
        DXCALL(mDevice->CreateTexture2D(&coplanarityTextureDesc, nullptr, &coplanarityTexture));

        mTerrainCoplanarityMap[ heightmapID ] = coplanarityTexture;*/
    }

    void DX11TerrainPass::UpdatePatchSize( RenderSettings::TerrainPatchSize patchSize )
    {
        mPatchSize = patchSize;

        std::vector<DX11TextureID> heightTextures;
        for ( const auto& pair : mTerrainCoplanarityMap )
            heightTextures.emplace_back( pair.first );

        mTerrainCoplanarityMap.clear();
        for ( DX11TextureID ID : heightTextures )
            CreateCoplanarityMap( ID );
    }

	void DX11TerrainPass::RenderInternal( const RenderableTerrains& terrains )
	{
		mTransformsBuffer.SetData( terrains.mTransforms );
		mTransformsBuffer.Bind( DX11CPUDynamicBuffer::Shaderslot::Vertex, SBUFFER_SLOT_EXTRA );

		uint32_t beginIndex = 0;
		for ( const RenderableTerrainData& terrainData : terrains.mTerrainData )
		{
			const DX11Texture& heightmap = mTextureMap.GetItem( terrainData.mHeightMap );

			heightmap.BindAsShaderResource( SHADER_TEXTURE_SLOT::SHADER_TEXTURE_SLOT_EXTRA );

			mPerTerrainCBuffer.SetData( { terrainData.mHeightScale, terrainData.mVariationScale, terrainData.mWorldMin, terrainData.mWorldMax } );
			mPerTerrainCBuffer.Bind();

			uint32_t endIndex = terrainData.mEndIndex;
			assert( endIndex > beginIndex );

			uint32_t numTransforms = endIndex - beginIndex;
			mQuadMesh.DrawInstanced( numTransforms );

			beginIndex = endIndex;
		}
	}

    void DX11TerrainPass::UpdateCoplanarityTexture( DX11TextureID heightmapID )
    {
        const DX11DynamicTexture& texture = mTerrainCoplanarityMap.at( heightmapID );

        uint32_t width, height;
        GetCoplanarityTextureOutputDimensions( width, height, mTextureMap.GetItem( heightmapID ), mPatchSize );

        BindComputeShader();

        mContext->CSSetUnorderedAccessViews( UAV_SLOT, 1, &texture.mUAV.p, nullptr ); 
        mContext->Dispatch(1, 1, 1);
        mContext->CSSetUnorderedAccessViews( UAV_SLOT, 1, &gNullUAV.p, nullptr );
    }

    void DX11TerrainPass::BindComputeShader()
    {
        switch ( mPatchSize )
        {
            case RenderSettings::TerrainPatchSize::X16:
            {
                mContext->CSSetShader(mCoplanarityComputeShader16, nullptr, 0);
                break;
            }

            case RenderSettings::TerrainPatchSize::X32:
            {
                mContext->CSSetShader(mCoplanarityComputeShader32, nullptr, 0);
                break;
            }

            case RenderSettings::TerrainPatchSize::X64:
            {
                mContext->CSSetShader(mCoplanarityComputeShader64, nullptr, 0);
                break;
            }
        }
    }

	void DX11TerrainPass::BindForRendering()
	{
		mContext->VSSetShader(mVertexShader, nullptr, 0);
		mContext->DSSetShader(mDomainShader, nullptr, 0);
		mContext->PSSetShader(mPixelShader, nullptr, 0);
		mContext->HSSetShader(mHullShader, nullptr, 0);
		mContext->IASetInputLayout(mLayout);
		mContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST);
		mCBuffer.SetData({ mTessData.mMinDistance, mTessData.mMaxDistance, mTessData.mMinFactor, mTessData.mMaxFactor });
		mCBuffer.Bind();
	}

	void DX11TerrainPass::UnbindRendering()
	{
		mContext->VSSetShader( nullptr, nullptr, 0 );
		mContext->DSSetShader( nullptr, nullptr, 0 );
		mContext->HSSetShader( nullptr, nullptr, 0 );
		mContext->PSSetShader( nullptr, nullptr, 0 );
	}
}
#pragma once

#include "include/Renderer/DirectX11/DX11Mesh.h"
#include "include/Renderer/DirectX11/DX11ConstantBuffer.hpp"
#include "include/Renderer/DirectX11/DX11Utils.h"
#include "include/Renderer/DirectX11/DX11CPUDynamicBuffer.h"
#include "include/RenderQueue/RenderQueue.h"
#include "include/Core/Types.h"
#include "include/Core/Containers/IDMap.hpp"
#include "include/Core/Platform/Directx11.h"

#include <vector>
#include <array>

namespace JonsEngine
{
	struct AABBRenderData;

    class DX11VertexTransformPass
    {
    public:
        DX11VertexTransformPass(ID3D11DevicePtr device, ID3D11DeviceContextPtr context, IDMap<DX11Mesh>& meshMap);
        ~DX11VertexTransformPass();

        void RenderStaticMesh(DX11Mesh& mesh, const Mat4& wvpMatrix, D3D_PRIMITIVE_TOPOLOGY topology);
		void RenderStaticMeshInstanced(DX11Mesh& mesh, const Mat4& viewProjectionMatrix, const std::vector<Mat4>& worldTransforms, D3D_PRIMITIVE_TOPOLOGY topology);
		void RenderStaticMeshes(const RenderableMeshContainer& renderData, MeshIndex start, MeshIndex stop, const Mat4& viewProjectionMatrix, D3D_PRIMITIVE_TOPOLOGY topology);
		void RenderAnimatedMeshes(const RenderableMeshContainer& renderData, MeshIndex start, MeshIndex stop, const Mat4& viewProjectionMatrix, D3D_PRIMITIVE_TOPOLOGY topology);
		void RenderMeshes(const RenderQueue::RenderData& renderData, const RenderableCollection& renderables, const Mat4& viewProjectionMatrix, D3D_PRIMITIVE_TOPOLOGY topology);
		void RenderAABBs(const AABBRenderData& aabbRenderData);


    private:
        struct TransformCBuffer
        {
            Mat4 mWVPMatrix;
			uint32_t mBoneIndexOffset;


            TransformCBuffer(const Mat4& wvpMatrix, const uint32_t boneIndexOffset) :
				mWVPMatrix(wvpMatrix),
				mBoneIndexOffset(boneIndexOffset)
            {
            }
        };

		enum class StaticRenderMode
		{
			Instanced,
			NonInstanced
		};

		void BindForStaticRendering(StaticRenderMode mode, D3D_PRIMITIVE_TOPOLOGY topology);
		void BindForAnimatedRendering(D3D_PRIMITIVE_TOPOLOGY topology);
        void RenderMeshesAux(const RenderableMesh::ContainerType& meshContainer, const RenderableMesh::Index beginIndex, const RenderableMesh::Index endIndex, const Mat4& viewProjectionMatrix);


		DX11CPUDynamicBuffer mInstancedDataBuffer;
		DX11ConstantBuffer<TransformCBuffer> mTransformCBuffer;
		IDMap<DX11Mesh>& mMeshMap;

        ID3D11DeviceContextPtr mContext;
        ID3D11VertexShaderPtr mStaticShader;
		ID3D11VertexShaderPtr mStaticInstancedShader;
		ID3D11VertexShaderPtr mAnimatedShader;
        ID3D11InputLayoutPtr mLayoutStatic;
		ID3D11InputLayoutPtr mLayoutAnimated;
    };
}

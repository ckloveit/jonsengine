#pragma once

#include "Renderer/DirectX11/DX11Utils.h"
#include "Renderer/DirectX11/DX11VertexBufferSet.hpp"
#include "Core/Types.h"
#include "Core/Platform.h"
#include "Resources/Bone.h"

#include <vector>
#include <memory>

namespace JonsEngine
{
    class DX11Mesh
    {
    public:
        enum VertexBufferSlot
        {
            VERTEX_BUFFER_SLOT_POSITIONS = 0,
            VERTEX_BUFFER_SLOT_NORMALS,
            VERTEX_BUFFER_SLOT_TEXCOORDS,
            VERTEX_BUFFER_SLOT_TANGENTS,
			VERTEX_BUFFER_SLOT_BONE_WEIGHTS,
            NUM_VERTEX_BUFFER_SLOTS
        };


		// TODO: make template to accept generic data container rather than just vectors
		DX11Mesh() = default;
		DX11Mesh(ID3D11DevicePtr device, ID3D11DeviceContextPtr context, const std::vector<float>& vertexData, const std::vector<uint16_t>& indexData, const Vec3& minBounds, const Vec3& maxBounds);
        DX11Mesh(ID3D11DevicePtr device, ID3D11DeviceContextPtr context, const std::vector<float>& vertexData, const std::vector<float>& normalData, const std::vector<float>& texCoords,
            const std::vector<float>& tangentData, const std::vector<uint16_t>& indexData, const Vec3& minBounds, const Vec3& maxBounds);
        DX11Mesh(ID3D11DevicePtr device, ID3D11DeviceContextPtr context, const std::vector<float>& vertexData, const std::vector<float>& normalData, const std::vector<float>& texCoords,
            const std::vector<float>& tangentData, const std::vector<BoneWeight>& boneWeights, const std::vector<uint16_t>& indexData, const Vec3& minBounds, const Vec3& maxBounds);
        ~DX11Mesh();

        void Draw();
		void Draw( uint32_t offset, uint32_t numIndices );
		void DrawInstanced(uint32_t numInstances);
        void DrawPositions();
		void DrawPositionsInstanced(uint32_t numInstances);
        void DrawAABB();


    private:
		void BindAllBuffers();

        ID3D11DeviceContextPtr mContext;
        ID3D11BufferPtr mVertexBuffer;
        ID3D11BufferPtr mNormalBuffer;
        ID3D11BufferPtr mTangentBuffer;
        ID3D11BufferPtr mTexcoordBuffer;
        ID3D11BufferPtr mBoneWeightBuffer;
        ID3D11BufferPtr mIndexBuffer;

		DX11MeshID mMeshID;
        uint32_t mNumVertices;
        uint32_t mNumIndices;
        bool mHasBones;
    };
}

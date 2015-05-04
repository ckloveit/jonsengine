#pragma once

#include "include/Renderer/DirectX11/DX11Utils.h"
#include "include/Core/EngineDefs.h"
#include "include/Core/Types.h"

#include <d3d11.h>
#include <vector>
#include <memory>

namespace JonsEngine
{
    class DX11Mesh;
    typedef std::shared_ptr<DX11Mesh> DX11MeshPtr;

    class DX11Mesh
    {
    public:
        enum VertexBufferSlot
        {
            VERTEX_BUFFER_SLOT_VERTICES = 0,
            VERTEX_BUFFER_SLOT_NORMALS,
            VERTEX_BUFFER_SLOT_TANGENTS,
            VERTEX_BUFFER_SLOT_BITANGENTS,
            VERTEX_BUFFER_SLOT_TEXCOORDS,
            NUM_VERTEX_BUFFER_SLOTS
        };


        DX11Mesh(ID3D11DevicePtr device, ID3D11DeviceContextPtr context, const std::vector<float>& vertexData, const std::vector<float>& normalData, const std::vector<float>& texCoords,
            const std::vector<float>& tangents, const std::vector<float>& bitangents, const std::vector<uint16_t>& indexData, const Vec3& minBounds, const Vec3& maxBounds);
        ~DX11Mesh();

        void Draw();
        void DrawAABB();

        MeshID GetMeshID() const;


    private:
        ID3D11DeviceContextPtr mContext;
        ID3D11BufferPtr mVertexBuffer;
        ID3D11BufferPtr mNormalBuffer;
        ID3D11BufferPtr mTangentBuffer;
        ID3D11BufferPtr mBitangentBuffer;
        ID3D11BufferPtr mTexcoordBuffer;
        ID3D11BufferPtr mIndexBuffer;

        MeshID mMeshID;
        uint32_t mNumVertices;
        uint16_t mNumIndices;
    };
}
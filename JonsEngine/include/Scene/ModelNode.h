#pragma once

#include "include/Scene/Mesh.h"
#include "include/Resources/JonsPackage.h"
#include "include/Core/Types.h"
#include "include/Core/Utils/Math.h"

#include <string>
#include <vector>

namespace JonsEngine
{
	class DX11Renderer;
    struct PackageNode;

	typedef const std::function<MaterialPtr(const std::string& assetName, const JonsPackagePtr jonsPkg)>& LoadMaterialFunc;

    class ModelNode
    {
    public:
        ModelNode(const std::string& name, const Mat4& initialTransform, const Vec3& minBounds, const Vec3& maxBounds, const MeshID meshID, MaterialPtr material);
        ModelNode(DX11Renderer& renderer, const JonsPackagePtr jonsPkg, const PackageNode& node, const Mat4& parentTransform, LoadMaterialFunc loadMaterialFunction);
        ~ModelNode();

		std::vector<Mesh>& GetMeshes();
		std::vector<ModelNode>& GetChildNodes();


		const std::string mName;
        const Mat4 mTransform;

        const AABB mAABB;


    private:

		std::vector<Mesh> mMeshes;
        std::vector<ModelNode> mChildNodes;
    };
}
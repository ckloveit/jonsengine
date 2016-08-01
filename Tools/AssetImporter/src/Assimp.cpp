#include "include/Assimp.h"

#include "include/Core/Math/Math.h"
#include "include/Core/Math/AABB.h"
#include "include/Resources/Animation.h"
#include "include/FreeImage.h"

#include <limits>

using namespace JonsEngine;

namespace JonsAssetImporter
{
    void AddStaticAABB(PackageModel& model);
    void CheckForInvalidAABB(PackageAABB& aabb);

    bool OnlyOneNodePerMesh(const aiScene* scene);
    Mat4 GetMeshNodeTransform(const aiScene* scene, const aiNode* node, const uint32_t meshIndex, const Mat4& parentTransform);
    uint32_t CountMeshOccurances(const aiNode* node, const uint32_t aiMeshIndex, const bool recursive);
    uint32_t GetNodeIndex(const PackageModel& model, const std::string& nodeName);
    PackageBone::BoneIndex GetBoneIndex(const PackageModel& model, const std::string& boneName);
    uint32_t CountChildren(const aiNode* node);
    bool UsedLessThanMaxNumBones(const std::vector<float>& boneWeights, const uint32_t offset);
    PackageNode::NodeIndex FindSkeletonRootNode(const aiMesh* assimpMesh, const std::vector<PackageNode>& nodes);
    const aiMesh* FindAssimpMesh(const std::string& meshName, const aiScene* scene);
    void GetNodeDistanceFromRoot(const std::string& name, const std::vector<PackageNode>& nodes, uint32_t& distFromRoot, PackageNode::NodeIndex& rootIndex);

    Mat4 aiMat4ToJonsMat4(const aiMatrix4x4& aiMat);
    Quaternion aiQuatToJonsQuat(const aiQuaternion& aiQuat);
    Vec3 aiColor3DToJonsVec3(const aiColor3D& color);
    Vec3 aiVec3ToJonsVec3(const aiVector3D& vec);


    Assimp::Assimp()
    {
    }

    Assimp::~Assimp()
    {
    }


    bool Assimp::ProcessScene(const boost::filesystem::path& modelPath, const std::string& modelName, FreeImage& freeimageImporter, JonsPackagePtr pkg)
    {
        const aiScene* scene = mImporter.ReadFile(modelPath.string(), aiProcess_FlipUVs | aiProcessPreset_TargetRealtime_MaxQuality);
        if (!scene)
        {
            Log("ERROR: Assimp parsing error: ");
            Log(mImporter.GetErrorString());
            return false;
        }

        // process materials
        // map scene material indexes to actual package material indexes
        MaterialMap materialMap;
        ProcessMaterials(scene, modelPath, materialMap, freeimageImporter, pkg);

        // process model hierarchy
        if (!ProcessModel(scene, modelName, materialMap, pkg))
            return false;

        PackageModel& model = pkg->mModels.back();
        if (!ProcessAnimations(model, scene))
            return false;

        AddStaticAABB(model);

        return true;
    }

    void Assimp::ProcessMaterials(const aiScene* scene, const boost::filesystem::path& modelPath, MaterialMap& materialMap, FreeImage& freeimageImporter, JonsPackagePtr pkg)
    {
        if (!scene->HasMaterials())
            return;

        for (uint32_t i = 0; i < scene->mNumMaterials; ++i)
        {
            const aiMaterial* material = scene->mMaterials[i];
            PackageMaterial pkgMaterial;
            aiString assimpTexturePath;

            if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0 && material->GetTexture(aiTextureType_DIFFUSE, 0, &assimpTexturePath) == aiReturn_SUCCESS)
            {
                const boost::filesystem::path texturePath = std::string(assimpTexturePath.C_Str());

                std::string fullTexturePath = "";
                if (modelPath.has_parent_path())
                {
                    fullTexturePath.append(modelPath.parent_path().string());
                    fullTexturePath.append("/");
                }
                fullTexturePath.append(texturePath.string());

                freeimageImporter.ProcessTexture(pkgMaterial.mDiffuseTexture, fullTexturePath);
                pkgMaterial.mHasDiffuseTexture = true;
            }

            if ((material->GetTextureCount(aiTextureType_NORMALS) > 0 && material->GetTexture(aiTextureType_NORMALS, 0, &assimpTexturePath) == aiReturn_SUCCESS) ||
                (material->GetTextureCount(aiTextureType_HEIGHT) > 0 && material->GetTexture(aiTextureType_HEIGHT, 0, &assimpTexturePath) == aiReturn_SUCCESS))
            {
                const boost::filesystem::path texturePath = std::string(assimpTexturePath.C_Str());

                std::string fullTexturePath = "";
                if (modelPath.has_parent_path())
                {
                    fullTexturePath.append(modelPath.parent_path().string());
                    fullTexturePath.append("/");
                }
                fullTexturePath.append(texturePath.string());

                freeimageImporter.ProcessTexture(pkgMaterial.mNormalTexture, fullTexturePath);
                pkgMaterial.mHasNormalTexture = true;
            }

            aiString materialName;
            aiColor3D diffuseColor;
            aiColor3D ambientColor(0.1f);
            aiColor3D specularColor;
            aiColor3D emissiveColor;

            material->Get(AI_MATKEY_NAME, materialName);
            material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
            //if (material->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor) == aiReturn_SUCCESS)     TODO
            //    ambientColor = aiColor3D(1.0f);
            material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);
            material->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor);

            pkgMaterial.mName = materialName.C_Str();
            pkgMaterial.mDiffuseColor = aiColor3DToJonsVec3(diffuseColor);
            pkgMaterial.mAmbientColor = aiColor3DToJonsVec3(ambientColor);
            pkgMaterial.mSpecularColor = aiColor3DToJonsVec3(specularColor);
            pkgMaterial.mEmissiveColor = aiColor3DToJonsVec3(emissiveColor);

            std::vector<PackageMaterial>::iterator iter = pkg->mMaterials.insert(pkg->mMaterials.end(), pkgMaterial);

            materialMap.insert(MaterialPair(i, std::distance(pkg->mMaterials.begin(), iter)));
        }
    }

    bool Assimp::ProcessModel(const aiScene* scene, const std::string& modelName, const MaterialMap& materialMap, JonsPackagePtr pkg)
    {
        pkg->mModels.emplace_back(modelName);
        PackageModel& model = pkg->mModels.back();

        // reserve storage
        const uint32_t numNodes = CountChildren(scene->mRootNode) + 1;
        model.mNodes.reserve(numNodes);
        model.mMeshes.reserve(scene->mNumMeshes);
        model.mAnimations.reserve(scene->mNumAnimations);

        // we assume only one node per mesh - means we can pre-transform vertices for bind pose, makes rendering static actors faster
        if (!OnlyOneNodePerMesh(scene))
        {
            Log("ERROR: Assimp parser assumes only one node per mesh");
            return false;
        }

        if (!ProcessMeshes(model.mMeshes, model.mSkeleton, scene, materialMap))
            return false;

        // recursively go through assimp node tree
        const auto rootParentIndex = PackageNode::INVALID_NODE_INDEX;
        if (!ProcessNode(model.mNodes, model.mMeshes, scene, scene->mRootNode, rootParentIndex))
            return false;
            
        if (!ProcessBones(model.mSkeleton, model.mMeshes, model.mNodes, scene))
            return false;

        return true;
    }

    bool Assimp::ProcessNode(std::vector<JonsEngine::PackageNode>& nodeContainer, const std::vector<JonsEngine::PackageMesh>& meshContainer, const aiScene* scene, const aiNode* assimpNode, const JonsEngine::PackageNode::NodeIndex parentNodeIndex)
    {
        const uint32_t nodeIndex = nodeContainer.size();
        nodeContainer.emplace_back(assimpNode->mName.C_Str(), nodeIndex, parentNodeIndex);
        PackageNode& jonsNode = nodeContainer.back();

        // jonsPkg uses same mesh indices as aiScene
        for (uint32_t meshIndex = 0; meshIndex < assimpNode->mNumMeshes; ++meshIndex)
        {
            jonsNode.mMeshes.emplace_back(assimpNode->mMeshes[meshIndex]);

            const PackageMesh& mesh = meshContainer.at(meshIndex);
            jonsNode.mAABB.mMinBounds = MinVal(jonsNode.mAABB.mMinBounds, mesh.mAABB.mMinBounds);
            jonsNode.mAABB.mMaxBounds = MaxVal(jonsNode.mAABB.mMaxBounds, mesh.mAABB.mMaxBounds);
        }

        const uint32_t firstChildIndex = nodeContainer.size();
        for (uint32_t childIndex = 0; childIndex < assimpNode->mNumChildren; ++childIndex)
        {
            const aiNode* child = assimpNode->mChildren[childIndex];

            if (!ProcessNode(nodeContainer, meshContainer, scene, child, nodeIndex))
                return false;
        }
        const uint32_t lastChildIndex = nodeContainer.size() - 1;

        // update node AABB
        for (uint32_t childIndex = firstChildIndex; childIndex <= lastChildIndex; ++childIndex)
        {
            const PackageNode& child = nodeContainer.at(childIndex);
            jonsNode.mAABB.mMinBounds = MinVal(jonsNode.mAABB.mMinBounds, child.mAABB.mMinBounds);
            jonsNode.mAABB.mMaxBounds = MaxVal(jonsNode.mAABB.mMaxBounds, child.mAABB.mMaxBounds);
        }

        // if node has no legit AABB from either its own mesh or a childrens mesh, zero length it
        // is done after updating parent aabb bounds since the result might otherwise invalidate the parents aabb when comparing it against its other childrens
        CheckForInvalidAABB(jonsNode.mAABB);

        return true;
    }

    bool Assimp::ProcessMeshes(std::vector<JonsEngine::PackageMesh>& meshContainer, std::vector<JonsEngine::PackageBone>& skeleton, const aiScene* scene, const MaterialMap& materialMap)
    {
        uint32_t boneStartIndex = 0, boneEndIndex = 0;
        for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
        {
            aiMesh* assimpMesh = scene->mMeshes[meshIndex];
            boneEndIndex = boneStartIndex + assimpMesh->mNumBones;
            meshContainer.emplace_back(assimpMesh->mName.C_Str(), boneStartIndex, boneEndIndex);
            PackageMesh& jonsMesh = meshContainer.back();

            if (!ProcessMeshGeometricData(jonsMesh, assimpMesh, scene, meshIndex))
                return false;

            // bone weights
            if (!ProcessVertexBoneWeights(jonsMesh.mBoneIndices, jonsMesh.mBoneWeights, assimpMesh))
                return false;

            if (materialMap.find(assimpMesh->mMaterialIndex) == materialMap.end())
            {
                Log("ERROR: Unable to find mesh material in materialMap");
                return false;
            }

            jonsMesh.mMaterialIndex = materialMap.at(assimpMesh->mMaterialIndex);

            boneStartIndex = boneEndIndex;
        }

        return true;
    }

    bool Assimp::ProcessMeshGeometricData(PackageMesh& jonsMesh, const aiMesh* assimpMesh, const aiScene* scene, const uint32_t meshIndex)
    {
        const uint32_t numFloatsPerTriangle = 3;
        const uint32_t numFloatsPerTexcoord = 2;

        // reserve storage
        jonsMesh.mVertexData.reserve(assimpMesh->mNumVertices * numFloatsPerTriangle);
        jonsMesh.mNormalData.reserve(assimpMesh->mNumVertices * numFloatsPerTriangle);
        jonsMesh.mTexCoordsData.reserve(assimpMesh->mNumVertices * numFloatsPerTexcoord);
        // store both tangents and bitangents in same buffer
        jonsMesh.mTangentData.reserve(assimpMesh->mNumVertices * numFloatsPerTriangle * 2);
        jonsMesh.mIndiceData.reserve(assimpMesh->mNumFaces * numFloatsPerTriangle);

        // get the transform of the node associated with this mesh to pre-multiply all the vertices
        const Mat4 nodeTransform = GetMeshNodeTransform(scene, scene->mRootNode, meshIndex, gIdentityMatrix);

        // vertice, normal, texcoord, tangents and bitangents data
        for (uint32_t j = 0; j < assimpMesh->mNumVertices; ++j)
        {
            const Vec3 transformedVertices = Vec3(nodeTransform * Vec4(aiVec3ToJonsVec3(assimpMesh->mVertices[j]), 1.0f));
            jonsMesh.mVertexData.push_back(transformedVertices.x);
            jonsMesh.mVertexData.push_back(transformedVertices.y);
            jonsMesh.mVertexData.push_back(transformedVertices.z);

            if (assimpMesh->HasNormals())
            {
                const Vec3 transformedNormals = Vec3(nodeTransform * Vec4(aiVec3ToJonsVec3(assimpMesh->mNormals[j]), 0.0f));
                jonsMesh.mNormalData.push_back(transformedNormals.x);
                jonsMesh.mNormalData.push_back(transformedNormals.y);
                jonsMesh.mNormalData.push_back(transformedNormals.z);
            }

            // multiple texture coordinates only used in special scenarios so only use first row by default
            if (assimpMesh->HasTextureCoords(0))
            {
                jonsMesh.mTexCoordsData.push_back(assimpMesh->mTextureCoords[0][j].x);
                jonsMesh.mTexCoordsData.push_back(assimpMesh->mTextureCoords[0][j].y);
            }

            if (assimpMesh->HasTangentsAndBitangents())
            {
                const Vec3 transformedTangents = Vec3(nodeTransform * Vec4(aiVec3ToJonsVec3(assimpMesh->mTangents[j]), 0.0f));
                jonsMesh.mTangentData.push_back(transformedTangents.x);
                jonsMesh.mTangentData.push_back(transformedTangents.y);
                jonsMesh.mTangentData.push_back(transformedTangents.z);

                const Vec3 transformedBitangents = Vec3(nodeTransform * Vec4(aiVec3ToJonsVec3(assimpMesh->mBitangents[j]), 0.0f));
                jonsMesh.mTangentData.push_back(transformedBitangents.x);
                jonsMesh.mTangentData.push_back(transformedBitangents.y);
                jonsMesh.mTangentData.push_back(transformedBitangents.z);
            }

            // mesh AABB
            jonsMesh.mAABB.mMinBounds = MinVal(jonsMesh.mAABB.mMinBounds, transformedVertices);
            jonsMesh.mAABB.mMaxBounds = MaxVal(jonsMesh.mAABB.mMaxBounds, transformedVertices);
        }

        // index data
        for (uint32_t j = 0; j < assimpMesh->mNumFaces; ++j)
        {
            // only dem triangles
            assert(assimpMesh->mFaces[j].mNumIndices == numFloatsPerTriangle);
            for (uint32_t index = 0; index < numFloatsPerTriangle; index++)
            {
                assert(assimpMesh->mFaces[j].mIndices[index] <= UINT16_MAX);
                jonsMesh.mIndiceData.push_back(assimpMesh->mFaces[j].mIndices[index]);
            }
        }

        return true;
    }

    bool Assimp::ProcessBones(std::vector<PackageBone>& bones, std::vector<PackageMesh>& meshes, const std::vector<PackageNode>& nodes, const aiScene* scene)
    {
        for (auto& pkgMesh : meshes)
        {
            const aiMesh* assimpMesh = FindAssimpMesh(pkgMesh.mName, scene);
            assert(assimpMesh);
        
            const bool hasSkeleton = assimpMesh->mNumBones > 0;
            if (!hasSkeleton)
                continue;
            
            const PackageNode::NodeIndex rootNodeIndex = FindSkeletonRootNode(assimpMesh, nodes);
            assert(rootNodeIndex != PackageNode::INVALID_NODE_INDEX);
			const PackageNode& rootNode = nodes.at(rootNodeIndex);

			pkgMesh.mStartBoneIndex = bones.size();
			bones.emplace_back(rootNode.mName, gIdentityMatrix);
			for (uint32_t boneNum = 0; boneNum < assimpMesh->mNumBones; ++boneNum)
			{
				const auto bone = assimpMesh->mBones[boneNum];
				bones.emplace_back(bone->mName.C_Str(), aiMat4ToJonsMat4(bone->mOffsetMatrix));
			}
			pkgMesh.mEndBoneIndex = bones.size();
        }
    
        return true;
    }

    bool Assimp::ProcessVertexBoneWeights(std::vector<uint8_t>& boneIndices, std::vector<float>& boneWeights, const aiMesh* assimpMesh)
    {
        // make sure containers are large enough as we will access indices directly when iterating the bones
        const uint32_t numVertices = assimpMesh->mNumVertices;
        const uint32_t maxContainerSize = numVertices * Animation::MAX_BONES_PER_VERTEX;
		// TODO: this maybe overallocates memory?
        boneIndices.resize(maxContainerSize);
        boneWeights.resize(maxContainerSize);

        const uint32_t numBones = assimpMesh->mNumBones;
        for (uint8_t boneIndex = 0; boneIndex < numBones; ++boneIndex)
        {
            const auto assimpBone = assimpMesh->mBones[boneIndex];
            
            const uint32_t numWeights = assimpBone->mNumWeights;
            for (uint32_t weightIndex = 0; weightIndex < numWeights; ++weightIndex)
            {
                const auto assimpWeight = assimpBone->mWeights[weightIndex];
                const uint32_t vertexStartIndex = assimpWeight.mVertexId * Animation::MAX_BONES_PER_VERTEX;

                // make sure we havn't reached bone weight cap per bone
                const bool notExceededNumBones = UsedLessThanMaxNumBones(boneWeights, vertexStartIndex);
                if (!notExceededNumBones)
                {
                    Log("ERROR: More bone weights used than capacity for");
                    return false;
                }
                
                // get the first unused bone index
                uint32_t firstFreeIndex = vertexStartIndex;
                while (!IsEqual(boneWeights.at(firstFreeIndex), 0.0f))
                    ++firstFreeIndex;

                boneWeights.at(firstFreeIndex) = assimpWeight.mWeight;
                boneIndices.at(firstFreeIndex) = boneIndex;
            }
        }

        return true;
    }

    bool Assimp::ProcessAnimations(PackageModel& model, const aiScene* scene)
    {
        if (!scene->HasAnimations())
            return true;

        const Mat4 rootNodeTransform = aiMat4ToJonsMat4(scene->mRootNode->mTransformation);
        const Mat4 invRootNodeTransform = glm::inverse(rootNodeTransform);

        for (uint32_t animationIndex = 0; animationIndex < scene->mNumAnimations; ++animationIndex)
        {
            aiAnimation* animation = *(scene->mAnimations + animationIndex);

            // what is this anyway...
            // shouldn't be present
            assert(animation->mNumMeshChannels == 0);

            const uint32_t durationMillisec = static_cast<uint32_t>((animation->mDuration / animation->mTicksPerSecond) * 1000);
            model.mAnimations.emplace_back(animation->mName.C_Str(), invRootNodeTransform, durationMillisec);
            PackageAnimation& pkgAnimation = model.mAnimations.back();

            const uint32_t noAnimationKeysNum = 1;
            for (uint32_t nodeAnimIndex = 0; nodeAnimIndex < animation->mNumChannels; ++nodeAnimIndex)
            {
                aiNodeAnim* nodeAnimation = *(animation->mChannels + nodeAnimIndex);

                // cant do scaling animations
                assert(nodeAnimation->mNumScalingKeys == noAnimationKeysNum);

                // if no rotation and translation, continue
                if (nodeAnimation->mNumPositionKeys == nodeAnimation->mNumRotationKeys == noAnimationKeysNum)
                    continue;

                const auto boneIndex = GetBoneIndex(model, nodeAnimation->mNodeName.C_Str());
                pkgAnimation.mBoneAnimations.emplace_back(boneIndex);
                BoneAnimation& boneAnimation = pkgAnimation.mBoneAnimations.back();

                const uint32_t numPosKeys = nodeAnimation->mNumPositionKeys;
                const uint32_t numRotkeys = nodeAnimation->mNumRotationKeys;
                const uint32_t maxNumKeys = glm::max(nodeAnimation->mNumPositionKeys, nodeAnimation->mNumRotationKeys);
                for (uint32_t key = 0; key < maxNumKeys; ++key)
                {
                    // same pos/rot might be used for several pos/rot transforms

                    // NOTE: use mTransformation after last key encountered????

                    // position
                    const uint32_t posKey = key < numPosKeys ? key : numPosKeys - 1;
                    aiVectorKey* aiPos = nodeAnimation->mPositionKeys + posKey;
                    const Vec3 translateVector = aiVec3ToJonsVec3(aiPos->mValue);
                    //Mat4 transform = glm::translate(posVec);

                    // rotation
                    const uint32_t rotKey = key < numRotkeys ? key : numRotkeys - 1;
                    aiQuatKey* aiRot = nodeAnimation->mRotationKeys + rotKey;
                    const Quaternion rotationQuat = aiQuatToJonsQuat(aiRot->mValue);
                    //transform *= glm::toMat4(rotQuat);

                    const double maxKeyTimeSeconds = glm::max(aiPos->mTime, aiRot->mTime) / animation->mTicksPerSecond;
                    const uint32_t timestampMillisec = static_cast<uint32_t>(maxKeyTimeSeconds * 1000);

                    boneAnimation.mKeyframes.emplace_back(timestampMillisec, translateVector, rotationQuat);
                }
            }
        }

        return true;
    }

    void AddStaticAABB(PackageModel& model)
    {
        // two cases: animated vs static model
        const PackageAABB& rootNodeAABB = model.mNodes.front().mAABB;
        model.mStaticAABB = rootNodeAABB;

        // TODO
        // static model: use root node AABB as its overall AABB
       /* if (model.mAnimations.empty())
        {
            model.mStaticAABB = rootNodeAABB;
            return;
        }

        // animated model: needs to transform all nodes using all the animations to find the maximum extents
        // results in a loose, static aabb
        Vec3 minExtent(rootNodeAABB.mMinBounds), maxExtent(rootNodeAABB.mMaxBounds);
        for (const PackageAnimation& animation : model.mAnimations)
        {
            for (const PackageBoneAnimation& animNode : animation.mBoneAnimations)
            {
                for (const PackageBoneKeyframe& keyframe : animNode.mKeyframes)
                {
                    AABB aabb(node.mAABB.mMinBounds, node.mAABB.mMaxBounds);
                    aabb = aabb * keyframe.mTransform;

                    const Vec3 tempMin = aabb.Min(), tempMax = aabb.Max();
                    if (tempMin.x < minExtent.x) minExtent.x = tempMin.x;
                    if (tempMin.y < minExtent.y) minExtent.y = tempMin.y;
                    if (tempMin.z < minExtent.z) minExtent.z = tempMin.z;

                    if (tempMax.x > maxExtent.x) maxExtent.x = tempMax.x;
                    if (tempMax.y > maxExtent.y) maxExtent.y = tempMax.y;
                    if (tempMax.z > maxExtent.z) maxExtent.z = tempMax.z;
                }
            }
        }

        model.mStaticAABB.mMinBounds = minExtent;
        model.mStaticAABB.mMaxBounds = maxExtent;*/
    }


    void CheckForInvalidAABB(PackageAABB& aabb)
    {
        // resets AABB to zero length
        if (aabb.mMinBounds == Vec3(std::numeric_limits<float>::max()) && aabb.mMaxBounds == Vec3(std::numeric_limits<float>::lowest()))
        {
            aabb.mMinBounds = Vec3(0.0f);
            aabb.mMaxBounds = Vec3(0.0f);
        }
    }

    bool OnlyOneNodePerMesh(const aiScene* scene)
    {
        for (uint32_t meshNum = 0; meshNum < scene->mNumMeshes; ++meshNum)
        {
            const bool recursive = true;
            const uint32_t occurances = CountMeshOccurances(scene->mRootNode, meshNum, recursive);
            if (occurances > 1)
                return false;
        }

        return true;
    }

    // assumes aiMeshIndex actually exists in node hierarchy
    Mat4 GetMeshNodeTransform(const aiScene* scene, const aiNode* node, const uint32_t aiMeshIndex, const Mat4& parentTransform)
    {
        const bool recursive = false;
        const uint32_t occurances = CountMeshOccurances(node, aiMeshIndex, recursive);
        const Mat4 nodeTransform = parentTransform * aiMat4ToJonsMat4(node->mTransformation);
        if (occurances != 1)
        {
            for (uint32_t childNum = 0; childNum < node->mNumChildren; ++childNum)
            {
                const aiNode* child = node->mChildren[childNum];
                if (CountMeshOccurances(child, aiMeshIndex, recursive) == 1)
                    return GetMeshNodeTransform(scene, child, aiMeshIndex, nodeTransform);
            }
        }

        return nodeTransform;
    }

    uint32_t CountMeshOccurances(const aiNode* node, const uint32_t aiMeshIndex, const bool recursive)
    {
        uint32_t occurances = 0;
        for (uint32_t meshNum = 0; meshNum < node->mNumMeshes; ++meshNum)
        {
            if (*(node->mMeshes + meshNum) == aiMeshIndex)
                ++occurances;
        }

        if (recursive)
        {
            for (uint32_t childNum = 0; childNum < node->mNumChildren; ++childNum)
            {
                const aiNode* child = node->mChildren[childNum];
                occurances += CountMeshOccurances(child, aiMeshIndex, recursive);
            }
        }

        return occurances;
    }

    uint32_t GetNodeIndex(const PackageModel& model, const std::string& nodeName)
    {
        uint32_t ret = model.mNodes.size();
        for (const PackageNode& node : model.mNodes)
        {
            if (node.mName == nodeName)
            {
                ret = node.mNodeIndex;
                break;
            }
        }

        assert(ret != model.mNodes.size());

        return ret;
    }
    
    PackageBone::BoneIndex GetBoneIndex(const PackageModel& model, const std::string& boneName)
    {
        for (const PackageMesh& mesh : model.mMeshes)
        {
            const uint32_t numBones = model.mSkeleton.size();
            for (PackageBone::BoneIndex boneIndex = 0; boneIndex < numBones; ++boneIndex)
            {
                const PackageBone& bone = model.mSkeleton.at(boneIndex);
                if (bone.mName == boneName)
                    return boneIndex;
            }
        }
        
        return PackageBone::INVALID_BONE_INDEX;
    }

    uint32_t CountChildren(const aiNode* node)
    {
        uint32_t ret = 0;

        for (uint32_t index = 0; index < node->mNumChildren; ++index)
        {
            ++ret;

            ret += CountChildren(node->mChildren[index]);
        }

        return ret;
    }

    bool UsedLessThanMaxNumBones(const std::vector<float>& boneWeights, const uint32_t offset)
    {
        for (uint32_t index = offset; index < offset + Animation::MAX_BONES_PER_VERTEX; ++index)
        {
            // weight zero means unused index
            if (IsEqual(boneWeights.at(index), 0.0f))
                return true;
        }

        // all indices used
        return false;
    }
    
    PackageNode::NodeIndex FindSkeletonRootNode(const aiMesh* assimpMesh, const std::vector<PackageNode>& nodes)
    {
        assert(assimpMesh);
        
        const uint32_t largestNodeDistance = std::numeric_limits<uint32_t>::max();
        
        PackageNode::NodeIndex ret = PackageNode::INVALID_NODE_INDEX;
        uint32_t minDistance = largestNodeDistance;
        for (uint32_t boneNum = 0; boneNum < assimpMesh->mNumBones; ++boneNum)
        {
            const aiBone* bone = assimpMesh->mBones[boneNum];
            uint32_t distFromRoot = largestNodeDistance;
            PackageNode::NodeIndex nodeIndex = PackageNode::INVALID_NODE_INDEX;
            GetNodeDistanceFromRoot(bone->mName.C_Str(), nodes, distFromRoot, nodeIndex);
            if (distFromRoot < minDistance)
            {
                minDistance = distFromRoot;
                ret = nodeIndex;
            }
        }
        
        return ret;
    }
    
    const aiMesh* FindAssimpMesh(const std::string& meshName, const aiScene* scene)
    {
        for (uint32_t meshNum = 0; meshNum < scene->mNumMeshes; ++meshNum)
        {
            const aiMesh* mesh = *scene->mMeshes + meshNum;
            if (mesh->mName.C_Str() == meshName)
                return mesh;
        }
        
        return nullptr;
    }
    
    void GetNodeDistanceFromRoot(const std::string& name, const std::vector<PackageNode>& nodes, uint32_t& distFromRoot, PackageNode::NodeIndex& rootIndex)
    {
        // finds the PackageNode with the right name
        PackageNode::NodeIndex nodeIndex = PackageNode::INVALID_NODE_INDEX;
        for (const auto& pkgNode : nodes)
        {
            if (pkgNode.mName == name)
				nodeIndex = pkgNode.mNodeIndex;
        }
        assert(nodeIndex != PackageNode::INVALID_NODE_INDEX);
        
        // backtracks to the root node and count steps
        uint32_t steps = 0;
        while (nodeIndex != PackageNode::INVALID_NODE_INDEX)
        {
            const auto& node = nodes.at(nodeIndex);
			nodeIndex = node.mParentNodeIndex;
            
            ++steps;
        };
        
        distFromRoot = steps;
		rootIndex = nodeIndex;
    }


    Mat4 aiMat4ToJonsMat4(const aiMatrix4x4& aiMat)
    {
        Mat4 jMat;

        jMat[0][0] = aiMat.a1; jMat[0][1] = aiMat.b1; jMat[0][2] = aiMat.c1; jMat[0][3] = aiMat.d1;
        jMat[1][0] = aiMat.a2; jMat[1][1] = aiMat.b2; jMat[1][2] = aiMat.c2; jMat[1][3] = aiMat.d2;
        jMat[2][0] = aiMat.a3; jMat[2][1] = aiMat.b3; jMat[2][2] = aiMat.c3; jMat[2][3] = aiMat.d3;
        jMat[3][0] = aiMat.a4; jMat[3][1] = aiMat.b4; jMat[3][2] = aiMat.c4; jMat[3][3] = aiMat.d4;

        return jMat;
    }

    Quaternion aiQuatToJonsQuat(const aiQuaternion& aiQuat)
    {
        return Quaternion(aiQuat.x, aiQuat.y, aiQuat.z, aiQuat.w);
    }

    Vec3 aiColor3DToJonsVec3(const aiColor3D& color)
    {
        return Vec3(color.r, color.g, color.b);
    }

    Vec3 aiVec3ToJonsVec3(const aiVector3D& vec)
    {
        return Vec3(vec.x, vec.y, vec.z);
    }
}
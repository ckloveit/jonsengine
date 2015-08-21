#include "include/Assimp.h"

#include "include/Core/Math/Math.h"
#include "include/FreeImage.h"

#include <limits>

using namespace JonsEngine;

namespace JonsAssetImporter
{
    void CheckForInvalidAABB(JonsEngine::PackageAABB& aabb);

    JonsEngine::Mat4 aiMat4ToJonsMat4(const aiMatrix4x4& aiMat);
    JonsEngine::Quaternion aiQuatToJonsQuat(const aiQuaternion& aiQuat);
    JonsEngine::Vec3 aiColor3DToJonsVec3(const aiColor3D& color);
    JonsEngine::Vec3 aiVec3ToJonsVec3(const aiVector3D& vec);


    Assimp::Assimp()
    {
    }

    Assimp::~Assimp()
    {
    }


    bool Assimp::ProcessScene(const boost::filesystem::path& modelPath, const std::string& modelName, FreeImage& freeimageImporter, JonsEngine::JonsPackagePtr pkg)
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
        ProcessAssimpMaterials(scene, modelPath, materialMap, freeimageImporter, pkg);

        // process model hierarchy
        pkg->mModels.emplace_back(modelName);
        auto& model = pkg->mModels.back();
        if (!ProcessAssimpNode(model.mRootNode, scene, scene->mRootNode, materialMap, model.mRootNode.mAABB.mMinBounds, model.mRootNode.mAABB.mMaxBounds))
            return false;

        if (!ProcessAssimpAnimations(model, scene))
            return false;

        return true;
    }

    void Assimp::ProcessAssimpMaterials(const aiScene* scene, const boost::filesystem::path& modelPath, MaterialMap& materialMap, FreeImage& freeimageImporter, JonsEngine::JonsPackagePtr pkg)
    {
        if (!scene->HasMaterials())
            return;

        for (uint32_t i = 0; i < scene->mNumMaterials; i++)
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

    bool Assimp::ProcessAssimpNode(JonsEngine::PackageNode& pkgNode, const aiScene* scene, const aiNode* node, const MaterialMap& materialMap, JonsEngine::Vec3& parentMinBounds, JonsEngine::Vec3& parentMaxBounds)
    {
        pkgNode.mName = node->mName.C_Str();
        pkgNode.mTransform = aiMat4ToJonsMat4(node->mTransformation);

        const uint32_t numMeshes = node->mNumMeshes;
        for (uint32_t i = 0; i < numMeshes; i++)
        {
            pkgNode.mMeshes.emplace_back();
            if (!ProcessAssimpMesh(pkgNode.mMeshes.back(), scene->mMeshes[node->mMeshes[i]], materialMap, pkgNode.mAABB.mMinBounds, pkgNode.mAABB.mMaxBounds))
                return false;
        }

        const uint32_t numChildren = node->mNumChildren;
        for (uint32_t i = 0; i < numChildren; i++)
        {
            pkgNode.mChildNodes.emplace_back();
            if (!ProcessAssimpNode(pkgNode.mChildNodes.back(), scene, node->mChildren[i], materialMap, pkgNode.mAABB.mMinBounds, pkgNode.mAABB.mMaxBounds))
                return false;
        }

        parentMinBounds = MinVal(parentMinBounds, pkgNode.mAABB.mMinBounds);
        parentMaxBounds = MaxVal(parentMaxBounds, pkgNode.mAABB.mMaxBounds);

        // if node has no legit AABB from either its own mesh or a childrens mesh, zero length it
        // is done after updating parent aabb bounds since the result might otherwise invalidate the parents aabb when comparing it against its other childrens
        CheckForInvalidAABB(pkgNode.mAABB);

        return true;
    }

    bool Assimp::ProcessAssimpMesh(JonsEngine::PackageMesh& pkgMesh, const aiMesh* m, const MaterialMap& materialMap, JonsEngine::Vec3& nodeMinBounds, JonsEngine::Vec3& nodeMaxBounds)
    {
        pkgMesh.mName = m->mName.C_Str();

        const uint32_t numFloatsPerTriangle = 3;
        const uint32_t numFloatsPerTexcoord = 2;
        pkgMesh.mVertexData.reserve(m->mNumVertices * numFloatsPerTriangle);
        pkgMesh.mNormalData.reserve(m->mNumVertices * numFloatsPerTriangle);
        pkgMesh.mTexCoordsData.reserve(m->mNumVertices * numFloatsPerTexcoord);
        // store both tangents and bitangents in same buffer
        pkgMesh.mTangentData.reserve(m->mNumVertices * numFloatsPerTriangle * 2);;

        // vertice, normal, texcoord, tangents and bitangents data
        for (unsigned int j = 0; j < m->mNumVertices; j++)
        {
            pkgMesh.mVertexData.push_back(m->mVertices[j].x);
            pkgMesh.mVertexData.push_back(m->mVertices[j].y);
            pkgMesh.mVertexData.push_back(m->mVertices[j].z);

            if (m->HasNormals())
            {
                pkgMesh.mNormalData.push_back(m->mNormals[j].x);
                pkgMesh.mNormalData.push_back(m->mNormals[j].y);
                pkgMesh.mNormalData.push_back(m->mNormals[j].z);
            }

            if (m->HasTextureCoords(0))
            {
                pkgMesh.mTexCoordsData.push_back(m->mTextureCoords[0][j].x);
                pkgMesh.mTexCoordsData.push_back(m->mTextureCoords[0][j].y);
            }

            if (m->HasTangentsAndBitangents())
            {
                pkgMesh.mTangentData.push_back(m->mTangents[j].x);
                pkgMesh.mTangentData.push_back(m->mTangents[j].y);
                pkgMesh.mTangentData.push_back(m->mTangents[j].z);

                pkgMesh.mTangentData.push_back(m->mBitangents[j].x);
                pkgMesh.mTangentData.push_back(m->mBitangents[j].y);
                pkgMesh.mTangentData.push_back(m->mBitangents[j].z);
            }

            // mesh AABB
            const Vec3 vertex = aiVec3ToJonsVec3(m->mVertices[j]);
            pkgMesh.mAABB.mMinBounds = MinVal(pkgMesh.mAABB.mMinBounds, vertex);
            pkgMesh.mAABB.mMaxBounds = MaxVal(pkgMesh.mAABB.mMaxBounds, vertex);
        }

        pkgMesh.mIndiceData.reserve(m->mNumFaces * numFloatsPerTriangle);

        // index data
        for (uint32_t j = 0; j < m->mNumFaces; j++)
        {
            // only dem triangles
            assert(m->mFaces[j].mNumIndices == numFloatsPerTriangle);
            for (uint32_t index = 0; index < numFloatsPerTriangle; index++)
            {
                assert(m->mFaces[j].mIndices[index] <= UINT16_MAX);
                pkgMesh.mIndiceData.push_back(m->mFaces[j].mIndices[index]);
            }
        }

        if (materialMap.find(m->mMaterialIndex) == materialMap.end())
        {
            Log("ERROR: Unable to find mesh material in materialMap");
            return false;
        }

        pkgMesh.mMaterialIndex = materialMap.at(m->mMaterialIndex);
        pkgMesh.mHasMaterial = true;

        // node AABB
        nodeMinBounds = MinVal(nodeMinBounds, pkgMesh.mAABB.mMinBounds);
        nodeMaxBounds = MaxVal(nodeMaxBounds, pkgMesh.mAABB.mMaxBounds);

        return true;
    }

    bool Assimp::ProcessAssimpAnimations(JonsEngine::PackageModel& model, const aiScene* scene)
    {
        if (!scene->HasAnimations())
            return true;

        for (uint32_t animationIndex = 0; animationIndex < scene->mNumAnimations; ++animationIndex)
        {
            aiAnimation* animation = *(scene->mAnimations + animationIndex);

            // what is this anyway...
            assert(animation->mNumMeshChannels == 0);

            const double duration = animation->mDuration / animation->mTicksPerSecond;
            model.mAnimations.emplace_back(animation->mName.C_Str(), duration);
            PackageAnimation& pkgAnimation = model.mAnimations.back();

            aiNode* rootNode = scene->mRootNode;
            const uint32_t noAnimationKeysNum = 1;
            for (uint32_t nodeKey = 0; nodeKey < animation->mNumChannels; ++nodeKey)
            {
                aiNodeAnim* nodeAnimation = *(animation->mChannels + nodeKey);
                aiNode* node = rootNode->FindNode(nodeAnimation->mNodeName);
                assert(node != NULL);

                // cant do scaling animations
                assert(nodeAnimation->mNumScalingKeys == noAnimationKeysNum);

                // if no rotation/translation, continue
                if (nodeAnimation->mNumPositionKeys == nodeAnimation->mNumRotationKeys == noAnimationKeysNum)
                    continue;

                pkgAnimation.mAnimatedNodes.emplace_back(nodeAnimation->mNodeName.C_Str());
                PackageAnimatedNode& pkgAnimatedNode = pkgAnimation.mAnimatedNodes.back();

                const uint32_t numPosKeys = nodeAnimation->mNumPositionKeys;
                const uint32_t numRotkeys = nodeAnimation->mNumRotationKeys;
                const uint32_t maxNumKeys = glm::max(nodeAnimation->mNumPositionKeys, nodeAnimation->mNumRotationKeys);
                for (uint32_t key = 0; key < maxNumKeys; ++key)
                {
                    // position
                    const uint32_t posKey = key < numPosKeys ? key : numPosKeys - 1;
                    aiVectorKey* aiPos = nodeAnimation->mPositionKeys + posKey;
                    const Vec3 posVec = aiVec3ToJonsVec3(aiPos->mValue);
                    Mat4 transform = glm::translate(posVec);
                    float keyTime = aiPos->mTime;

                    // rotation
                    const uint32_t rotKey = key < numRotkeys ? key : numRotkeys - 1;
                    aiQuatKey* aiRot = nodeAnimation->mRotationKeys + rotKey;
                    const Quaternion rotQuat = aiQuatToJonsQuat(aiRot->mValue);
                    transform *= glm::toMat4(rotQuat);
                    keyTime = aiRot->mTime;

                    pkgAnimatedNode.mAnimationTransforms.emplace_back(keyTime, transform);
                }
            }
        }

        return true;
    }


    void CheckForInvalidAABB(JonsEngine::PackageAABB& aabb)
    {
        // resets AABB to zero length
        if (aabb.mMinBounds == Vec3(std::numeric_limits<float>::max()) && aabb.mMaxBounds == Vec3(std::numeric_limits<float>::lowest()))
        {
            aabb.mMinBounds = Vec3(0.0f);
            aabb.mMaxBounds = Vec3(0.0f);
        }
    }


    JonsEngine::Mat4 aiMat4ToJonsMat4(const aiMatrix4x4& aiMat)
    {
        Mat4 jMat;

        jMat[0][0] = aiMat.a1; jMat[0][1] = aiMat.b1; jMat[0][2] = aiMat.c1; jMat[0][3] = aiMat.d1;
        jMat[1][0] = aiMat.a2; jMat[1][1] = aiMat.b2; jMat[1][2] = aiMat.c2; jMat[1][3] = aiMat.d2;
        jMat[2][0] = aiMat.a3; jMat[2][1] = aiMat.b3; jMat[2][2] = aiMat.c3; jMat[2][3] = aiMat.d3;
        jMat[3][0] = aiMat.a4; jMat[3][1] = aiMat.b4; jMat[3][2] = aiMat.c4; jMat[3][3] = aiMat.d4;

        return jMat;
    }

    JonsEngine::Quaternion aiQuatToJonsQuat(const aiQuaternion& aiQuat)
    {
        return Quaternion(aiQuat.x, aiQuat.y, aiQuat.z, aiQuat.w);
    }

    JonsEngine::Vec3 aiColor3DToJonsVec3(const aiColor3D& color)
    {
        return Vec3(color.r, color.g, color.b);
    }

    JonsEngine::Vec3 aiVec3ToJonsVec3(const aiVector3D& vec)
    {
        return Vec3(vec.x, vec.y, vec.z);
    }
}
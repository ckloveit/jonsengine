#include "include/Scene/Scene.h"
#include "include/Core/Memory/HeapAllocator.h"
#include "include/Core/EngineDefs.h"
#include "include/Core/Utils/Math.h"
#include "include/Resources/ResourceManifest.h"

#include "boost/functional/hash.hpp"
#include <algorithm>

namespace JonsEngine
{
    const float gPointLightMinZ = 0.1f;

    const Vec3 gCubemapDirVectors[RenderablePointLight::POINT_LIGHT_DIR_COUNT] = { Vec3(1.0f, 0.0f, 0.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f),
                                                                                   Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, 0.0f, 1.0f) };

    const Vec3 gCubemapUpVectors[RenderablePointLight::POINT_LIGHT_DIR_COUNT] = { Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f),
                                                                                  Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f) };


    void AddMesh(std::vector<RenderableModel>& resultMeshes, const Mesh& mesh, const Mat4& worldMatrix)
    {
		resultMeshes.emplace_back(mesh.mMeshID, worldMatrix, mesh.GetMaterial()->mDiffuseTexture, mesh.GetMaterial()->mNormalTexture, mesh.GetMaterial()->mSpecularFactor, mesh.GetTextureTilingFactor());
    }

    void AddMesh(std::vector<RenderableMesh>& resultMeshes, const Mesh& mesh, const Mat4& worldMatrix)
    {
        resultMeshes.emplace_back(mesh.mMeshID, worldMatrix);
    }

    void AddMesh(std::vector<uint32_t>& resultMeshes, const ModelMesh& mesh, const Mat4& worldMatrix)
    {
        resultMeshes.push_back(mesh.mMeshID);
    }

    template <typename T>
    void AddAllMeshes(std::vector<T>& resultMeshes, ModelNode& node, const Mat4& worldMatrix)
    {
        const Mat4 localWorldMatrix = worldMatrix * node.mTransform;

		for (const Mesh& mesh : node.GetMeshes())
            AddMesh(resultMeshes, mesh, localWorldMatrix);

		for (ModelNode& node : node.GetChildNodes())
            AddAllMeshes(resultMeshes, node, localWorldMatrix);
    }

    void AddAllMeshes2(const Model& model, const ModelNode& node)
    {

    }

    void Scene::CullFrustrum(std::vector<MeshID>& visibleMeshes, ModelNode& node, const Mat4& wvpMatrix)
    {
        const AABB& aabb = mAABBs.at(node.mAABBIndex);

        const Mat4 localWVPMatrix = wvpMatrix * mTransforms.at(node.mTransformIndex);

        AABBIntersection nodeAABBIntersection = IsAABBInFrustum(aabb, localWVPMatrix);
        switch (nodeAABBIntersection)
        {
            // if partially inside, recursively test all meshes and child nodes
            case AABBIntersection::AABB_INTERSECTION_PARTIAL:
            {
                AABBIntersection meshAABBIntersection(AABBIntersection::AABB_INTERSECTION_INSIDE);

                for (uint32_t meshIndex = node.mMeshStartIndex; meshIndex < node.mMeshEndIndex; meshIndex++)
                {
                    const ModelMesh& mesh = mMeshes.at(meshIndex);
                    const AABB& aabbMesh = mAABBs.at(mesh.mAABBIndex);

                    meshAABBIntersection = IsAABBInFrustum(aabbMesh, localWVPMatrix);
                    if (meshAABBIntersection == AABBIntersection::AABB_INTERSECTION_OUTSIDE)
                        continue;

                    if (meshAABBIntersection == AABBIntersection::AABB_INTERSECTION_INSIDE || meshAABBIntersection == AABBIntersection::AABB_INTERSECTION_PARTIAL)
                        visibleMeshes.push_back(mesh.mMeshID);
                }

                for (uint32_t nodeIndex = node.mNodeStartIndex; nodeIndex < node.mNodeEndIndex; nodeIndex++)
                    CullFrustrum(visibleMeshes, mNodes.at(nodeIndex), localWVPMatrix);

                break;
            }

            case AABBIntersection::AABB_INTERSECTION_INSIDE:
            {
                for (uint32_t meshIndex = node.mMeshStartIndex; meshIndex < node.mMeshEndIndex; meshIndex++)
                    visibleMeshes.push_back(mMeshes.at(meshIndex).mMeshID);

                break;
            }

            case AABBIntersection::AABB_INTERSECTION_OUTSIDE:
            default:
                break;
        }
    }

    /*void CullMeshesSphere(std::vector<T>& resultMeshes, ModelNode& node, const Mat4& worldMatrix, const Vec3& sphereCentre, const float sphereRadius)
    {
        const Mat4 localWVPMatrix = wvpMatrix * node.mTransform;

        // test node frustrum
        AABBIntersection nodeAABBIntersection = IsAABBInFrustum(node.mAABBCenter, node.mAABBExtent, localWVPMatrix);
        switch (nodeAABBIntersection)
        {
            // if partially inside, recursively test all meshes and child nodes
        case AABBIntersection::AABB_INTERSECTION_PARTIAL:
        {
            AABBIntersection meshAABBIntersection(AABBIntersection::AABB_INTERSECTION_INSIDE);

            for (const Mesh& mesh : node.GetMeshes())
            {
                meshAABBIntersection = IsAABBInFrustum(mesh.mAABBCenter, mesh.mAABBExtent, localWVPMatrix);
                if (meshAABBIntersection == AABBIntersection::AABB_INTERSECTION_OUTSIDE)
                    continue;

                if (meshAABBIntersection == AABBIntersection::AABB_INTERSECTION_INSIDE || meshAABBIntersection == AABBIntersection::AABB_INTERSECTION_PARTIAL)
                    AddMesh(resultMeshes, mesh, invViewProjMatrix * localWVPMatrix);
            }

            for (ModelNode& node : node.GetChildNodes())
                CullMeshesFrustrum(resultMeshes, node, localWVPMatrix, invViewProjMatrix, worldMatrix);

            break;
        }

        case AABBIntersection::AABB_INTERSECTION_INSIDE:
        {
            AddAllMeshes(resultMeshes, node, invViewProjMatrix * localWVPMatrix);
            break;
        }

        case AABBIntersection::AABB_INTERSECTION_OUTSIDE:
        default:
            break;
        }
    }*/

    Scene::Scene(const std::string& sceneName) : mName(sceneName), mHashedID(boost::hash_value(sceneName)), mMemoryAllocator(HeapAllocator::GetDefaultHeapAllocator()), mRootNode("Root"), mAmbientLight(0.2f)
    {
    }

    Scene::~Scene()
    {
    }


    bool Scene::operator==(const Scene& s1)
    {
        return mHashedID == s1.mHashedID;
    }

    bool Scene::operator==(const std::string& sceneName)
    {
        return mHashedID == boost::hash_value(sceneName);
    }


    const RenderQueue& Scene::GetRenderQueue(const uint32_t windowWidth, const uint32_t windowHeight, const float zNear, const float zFar)
    {
        mRenderQueue.Clear();

        const Camera& sceneCamera = GetSceneCamera();

        // camera
        mRenderQueue.mCamera.mFOV = sceneCamera.GetFOV();
        mRenderQueue.mCamera.mCameraPosition = sceneCamera.Position();
        mRenderQueue.mCamera.mCameraProjectionMatrix = PerspectiveMatrixFov(mRenderQueue.mCamera.mFOV, windowWidth / static_cast<float>(windowHeight), zNear, zFar);
        mRenderQueue.mCamera.mCameraViewMatrix = sceneCamera.GetCameraTransform();
        
        const Mat4 cameraViewProjMatrix = mRenderQueue.mCamera.mCameraProjectionMatrix * mRenderQueue.mCamera.mCameraViewMatrix;
        const Mat4 invCamViewProjMatrix = glm::inverse(cameraViewProjMatrix);

		for (const ActorPtr& actor : mActors)
		{
			if (!actor->mSceneNode)
				continue;

			const Mat4& worldMatrix = actor->mSceneNode->GetNodeTransform();
            const Mat4 wvpMatrix = cameraViewProjMatrix * worldMatrix;

            CullFrustrum(mRenderQueue.mCamera.mModels, mNodes.at(actor->mModel->mRootNodeIndex), wvpMatrix);
		}

        // point lights
        /*for (const PointLightPtr& pointLight : mPointLights)
        {
            const Vec3& pointLightPosition = pointLight->mSceneNode->Position();
            const Mat4 worldMatrix = pointLight->mSceneNode->GetNodeTransform();
            const Vec3 camViewLightPosition = Vec3(mRenderQueue.mCamera.mCameraViewMatrix * Vec4(pointLightPosition, 1.0));

            // scaled WVP is used for stencil op
            const Mat4 scaledWorldMatrix = glm::scale(worldMatrix, Vec3(pointLight->mMaxDistance));
            const Mat4 scaledWVPMatrix = cameraViewProjMatrix * scaledWorldMatrix;

            mRenderQueue.mPointLights.emplace_back(scaledWVPMatrix, pointLight->mLightColor, camViewLightPosition, pointLight->mLightIntensity, pointLight->mMaxDistance);
            RenderablePointLight& renderablePointLight = mRenderQueue.mPointLights.back();

            // for each cubemap face (6) of the point light, get meshes in view
            for (uint32_t dirIndex = 0; dirIndex < RenderablePointLight::POINT_LIGHT_DIR_COUNT; dirIndex++)
            {
                const Mat4 faceViewMatrix = glm::lookAt(camViewLightPosition, camViewLightPosition + gCubemapDirVectors[dirIndex], gCubemapUpVectors[dirIndex]);
                const Mat4 faceProjmatrix = PerspectiveMatrixFov(90.0f, 1.0f, gPointLightMinZ, pointLight->mMaxDistance);

                // face wvp matrix
                renderablePointLight.mFaceWVPMatrices.at(dirIndex) = faceProjmatrix * faceViewMatrix * mRenderQueue.mCamera.mCameraViewMatrix;

                //  cull meshes for each face
                for (const ActorPtr& actor : mActors)
                {
                    if (!actor->mSceneNode)
                        continue;

                    const Mat4& worldMatrix = actor->mSceneNode->GetNodeTransform();
					CullMeshes<RenderableMesh>(renderablePointLight.mMeshes.at(dirIndex), actor->mModel->GetRootNode(), renderablePointLight.mFaceWVPMatrices.at(dirIndex) * worldMatrix, worldMatrix);
                }
            }
        }*/

        // dir lights
        for (const DirectionalLightPtr& dirLight : mDirectionalLights)
        {
            // TODO
            mRenderQueue.mDirectionalLights.emplace_back(dirLight->mLightColor, dirLight->mLightDirection);
        }

        // misc
        mRenderQueue.mAmbientLight = mAmbientLight;
        
        return mRenderQueue;
    }


    Actor* Scene::CreateActor(const std::string& actorName, const ModelPtr model, const SceneNodePtr node)
    {
        mActors.emplace_back(mMemoryAllocator.AllocateObject<Actor>(actorName, model, node), std::bind(&HeapAllocator::DeallocateObject<Actor>, &mMemoryAllocator, std::placeholders::_1));

        return mActors.back().get();
    }

    void Scene::DeleteActor(Actor* actor)
    {
        auto iter = std::find_if(mActors.begin(), mActors.end(), [actor](const ActorPtr& storedActor) { return storedActor.get() == actor; });

        if (iter != mActors.end())
            mActors.erase(iter);
    }

    Actor* Scene::GetActor(const std::string& actorName)
    {
        auto iter = std::find_if(mActors.begin(), mActors.end(), [actorName](const ActorPtr& storedActor) { return *storedActor == actorName; });
        if (iter == mActors.end())
            return nullptr;

        return iter->get();
    }

	const std::vector<ActorPtr>& Scene::GetActors() const
	{
		return mActors;
	}


    PointLight* Scene::CreatePointLight(const std::string& lightName, const SceneNodePtr node)
    {
        mPointLights.emplace_back(mMemoryAllocator.AllocateObject<PointLight>(lightName, node), std::bind(&HeapAllocator::DeallocateObject<PointLight>, &mMemoryAllocator, std::placeholders::_1));

        return mPointLights.back().get();
    }
    
    void Scene::DeletePointLight(const PointLight* pointLight)
    {
        auto iter = std::find_if(mPointLights.begin(), mPointLights.end(), [pointLight](const PointLightPtr& storedLight) { return storedLight.get() == pointLight; });

        if (iter != mPointLights.end())
            mPointLights.erase(iter);
    }
    
    PointLight* Scene::GetPointLight(const std::string& lightName)
    {
		auto iter = std::find_if(mPointLights.begin(), mPointLights.end(), [lightName](const PointLightPtr& storedLight) { return *storedLight == lightName; });
        if (iter == mPointLights.end())
            return nullptr;

        return iter->get();
    }
   
    const std::vector<PointLightPtr>& Scene::GetPointLights() const
    {
        return mPointLights;
    }
    

    DirectionalLight* Scene::CreateDirectionalLight(const std::string& lightName)
    {
        mDirectionalLights.emplace_back(mMemoryAllocator.AllocateObject<DirectionalLight>(lightName), std::bind(&HeapAllocator::DeallocateObject<DirectionalLight>, &mMemoryAllocator, std::placeholders::_1));

        return mDirectionalLights.back().get();
    }
    
    void Scene::DeleteDirectionalLight(const DirectionalLight* dirLight)
    {
        auto iter = std::find_if(mDirectionalLights.begin(), mDirectionalLights.end(), [dirLight](const DirectionalLightPtr& storedLight) { return storedLight.get() == dirLight; });

        if (iter != mDirectionalLights.end())
            mDirectionalLights.erase(iter);
    }
    
    DirectionalLight* Scene::GetDirectionalLight(const std::string& lightName)
    {
        auto iter = std::find_if(mDirectionalLights.begin(), mDirectionalLights.end(), [lightName](const DirectionalLightPtr& storedLight) { return *storedLight == lightName; });
        if (iter == mDirectionalLights.end())
            return nullptr;

        return iter->get();
    }

    const std::vector<DirectionalLightPtr>& Scene::GetDirectionalLights() const
    {
        return mDirectionalLights;
    }


	void Scene::SetAmbientLight(const Vec4& ambientLight)
    {
        mAmbientLight = ambientLight;
    }

    const Vec4& Scene::GetAmbientLight() const
    {
        return mAmbientLight;
    }


	Camera& Scene::GetSceneCamera()
	{ 
		return mSceneCamera;
	}

    SceneNode& Scene::GetRootNode()
	{ 
		return mRootNode;
	}
}
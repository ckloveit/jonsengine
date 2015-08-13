#include "include/Scene.h"

#include "include/Core/Types.h"

using namespace JonsEngine;

namespace JonsGame
{
    Scene::Scene(JonsEngine::Scene& jonsScene, JonsEngine::ResourceManifest& resManifest, const std::string& jonsPkgName) :
        // base
        mScene(jonsScene),
        mJonsPackage(ReadJonsPkg("assets.jons")),

        // skybox
        mSkybox(resManifest.LoadSkybox("skybox", mJonsPackage)),

        // materials
        mMaterialCheckers(resManifest.LoadMaterial("checkers", mJonsPackage)),

        // models
        mModelSectoid(resManifest.LoadModel("sectoid", mJonsPackage)),
        mModelCube(resManifest.LoadModel("cube", mJonsPackage)),
        mModelChair(resManifest.LoadModel("chair", mJonsPackage)),
        mModelHouse(resManifest.LoadModel("house", mJonsPackage)),
        mModelWuson(resManifest.LoadModel("wuson", mJonsPackage)),
        mModelPlane(resManifest.CreateRectangle("GroundPlane", 64, 1.0, 64)),
        mModelSphere(resManifest.CreateSphere("Sphere", 1.0f, 12, 24)),
        mModelCube2(resManifest.CreateCube("Cube2", 3)),
        mModelCube3(resManifest.CreateCube("Cube3", 3)),

        // scene nodes
        mNodeSectoid(mScene.GetRootNode().CreateChildNode("nodeSectoid")),
        mNodeCube(mScene.GetRootNode().CreateChildNode("nodeCube")),
        mNodeChair(mScene.GetRootNode().CreateChildNode("nodeChair")),
        mNodeHouse(mScene.GetRootNode().CreateChildNode("nodeHouse")),
        mNodeWuson(mScene.GetRootNode().CreateChildNode("wuson")),
        mNodePointLight(mScene.GetRootNode().CreateChildNode("nodeMovingLight")),
        mNodePlane(mScene.GetRootNode().CreateChildNode("nodePlane")),
        mNodeSphere(mScene.GetRootNode().CreateChildNode("nodeSphere")),
        mNodeCube2(mScene.GetRootNode().CreateChildNode("nodeCube2")),
        mNodeCube3(mScene.GetRootNode().CreateChildNode("nodeCube3")),
        
        // lights
        mMovingPointLightID(mScene.CreatePointLight("MovingPointLight", mNodePointLight)),
        mSunDirLightID(mScene.CreateDirectionalLight("DirectionalLight", 4)),

        // actors
        mActorSectoid(mScene.CreateActor("actorSectoid", mModelSectoid, mNodeSectoid)),
        mActorCube(mScene.CreateActor("actorCube", mModelCube, mNodeCube)),
        mActorChair(mScene.CreateActor("actorChair", mModelChair, mNodeChair)),
        mActorHouse(mScene.CreateActor("actorHouse", mModelHouse, mNodeHouse)),
        mActorWuson(mScene.CreateActor("wuson", mModelWuson, mNodeWuson)),
        mActorPlane(mScene.CreateActor("actorPlane", mModelPlane, mNodePlane)),
        mActorSphere(mScene.CreateActor("actorSphere", mModelSphere, mNodeSphere)),
        mActorCube2(mScene.CreateActor("actorCube2", mModelCube2, mNodeCube2)),
        mActorCube3(mScene.CreateActor("actorCube3", mModelCube3, mNodeCube3))
    {
        // ambient light
        mScene.SetAmbientLight(Vec4(0.01f));

        // sectoid
        mNodeSectoid->TranslateNode(Vec3(0.0f, 0.5f, -4.0f));
        mNodeSectoid->RotateNode(90.0f, Vec3(1.0f, 0.0f, 0.0f));

        // cube
        mNodeCube->TranslateNode(Vec3(7.0f, 1.0f, -15.0f));

        // chair
        mNodeChair->TranslateNode(Vec3(-8.0f, 0.5f, -4.0f));
        mNodeChair->ScaleNode(2.0f);

        // house
        mNodeHouse->TranslateNode(Vec3(-7.0f, 0.5f, -15.0f));

        // animated wuson
        mNodeWuson->TranslateNode(Vec3(-10.0f, 0.5f, -11.0f));
        mNodeWuson->RotateNode(90.0f, Vec3(1.0f, 0.0f, 0.0f));

        // point light
        PointLight& movingLight = mScene.GetPointLight(mMovingPointLightID);
        movingLight.mLightRadius = 10.0f;
        movingLight.mLightIntensity = 2.0f;
        movingLight.mLightColor = Vec4(1.0f, 1.0f, 0.0f, 0.0f);
        mNodePointLight->TranslateNode(Vec3(5.0f, 3.5f, -15.0f));

        // directional light
        DirectionalLight& sun = mScene.GetDirectionalLight(mMovingPointLightID);
        sun.mLightColor = Vec4(0.55f);

        // ground plane
        Actor& actorPlane = mScene.GetActor(mActorPlane);
        actorPlane.SetMaterialTilingFactor(64.0f);

        // create a sphere
        mNodeSphere->TranslateNode(Vec3(6.0f, 5.5f, 10.0f));

        // create a  second cube
        mNodeCube2->TranslateNode(Vec3(3.0f, 2.0f, -15.0f));

        // create a third cube, far away casting shadows from the dir light
        mNodeCube3->TranslateNode(-sun.mLightDirection * 40.0f);

        // load skybox
        mScene.SetSkybox(mSkybox);

        // move up camera
        mScene.GetSceneCamera().TranslateCamera(Vec3(0.0f, 3.0f, 0.0f));
    }

    Scene::~Scene()
    {
        // destructor is called at application shutdown anyway so dont bother deleting assets
    }


    JonsEngine::Scene& Scene::GetJonsScene()
    {
        return mScene;
    }

    JonsEngine::PointLight& Scene::GetMovingPointLight()
    {
        return mScene.GetPointLight(mMovingPointLightID);
    }

    JonsEngine::DirectionalLight& Scene::GetSunDirLight()
    {
        return mScene.GetDirectionalLight(mSunDirLightID);
    }

    JonsEngine::Actor& Scene::GetAnimatedWuson()
    {
        return mScene.GetActor(mActorWuson);
    }
}
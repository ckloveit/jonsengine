#pragma once

#include "Core/EngineSettings.h"
#include "Scene/SceneDirtyFlags.h"
#include "RenderQueue/RenderQueue.h"

namespace JonsEngine
{
	struct DebugOptions;
    class ResourceManifest;
    class Scene;

    class SceneParser
    {
    public:
        SceneParser(const EngineSettings& engineSettings, const ResourceManifest& resManifest);
        ~SceneParser();

        const RenderQueue& ParseScene( const Scene& scene, DirtyFlagsSet dirtyFlags, const DebugOptions& debugOpts, Vec2 windowDimentions, float windowAspectRatio,
			float zNear, float zFar, RenderSettings::TerrainPatchMaxSize terrainMaxPatchSize );


    private:
		// TODO: support multiple cameras
        void ViewFrustumCulling(const Scene& scene, const float windowAspectRatio, const float zNear, const float zFar);
        void PointLightCulling(const Scene& scene);
        void DirectionalLightCulling(const Scene& scene);
		void TerrainParsing( const Scene& scene, float zNear, float zFar, RenderSettings::TerrainPatchMaxSize terrainMaxPatchSize, const DirtyFlagsSet dirtyFlags );
		void CopyBoneTransforms(const Scene& scene);
		void GetSkybox(const Scene& scene);
		void GetAmbientLight(const Scene& scene);
		// needs to run after ViewFrustumCulling
		void AddModelAABBDebugData(const Scene& scene);
		void AddTerrainAABBDebugData( const Scene& scene, float zNear, float zFar, bool doPatches );


        const ResourceManifest& mResourceManifest;
        const EngineSettings::CullingStrategy mCullingStrategy;
		DX11MeshID mDebugAABBMesh = INVALID_DX11_MESH_ID;
        RenderQueue mRenderQueue;
    };
}
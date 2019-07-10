#include "SceneTessellation.h"

#include "Core/Types.h"
#include "Scene/SceneManager.h"

using namespace JonsEngine;

constexpr float gTerrainHeightScale = 128.0f;
constexpr float gTerrainVariationScale = 2.0f;

namespace JonsGame
{
	SceneTessellation::SceneTessellation( JonsEngine::SceneManager& sceneMgr, JonsEngine::ResourceManifest& resManifest ) :
		mID( sceneMgr.CreateScene( "TessellationScene" ) ),
		mSceneMgr( sceneMgr ),
		mJonsPackage( ReadJonsPkg( "assets.jons" ) ),
		mSkybox( resManifest.LoadSkybox( "skybox", mJonsPackage ) ),
		mNodeTerrain( sceneMgr.GetScene( mID ).CreateSceneNode( "nodeTerrain", sceneMgr.GetScene( mID ).GetRootNodeID() ) ),
		mNodeTerrainTest( sceneMgr.GetScene( mID ).CreateSceneNode( "nodeTerrainTest", sceneMgr.GetScene( mID ).GetRootNodeID() ) ),
		mTerrainDataTest( resManifest.CreateTerrainData( "terrainDataTest", "terrain_test", mJonsPackage ) ),
		mTerrainData( resManifest.CreateTerrainData( "terraindata", "terrain_heightmap", mJonsPackage ) ),
		mTerrainTest( sceneMgr.GetScene( mID ).CreateTerrain( "terrainTest", gTerrainHeightScale, gTerrainVariationScale, mNodeTerrainTest, mTerrainDataTest ) ),
		mTerrain( sceneMgr.GetScene( mID ).CreateTerrain( "terrain", gTerrainHeightScale, gTerrainVariationScale, mNodeTerrain, mTerrainData ) ),
		mSunDirLightID( sceneMgr.GetScene( mID ).CreateDirectionalLight( "DirectionalLight", 4 ) ),
		mSun( mSceneMgr.GetScene( mID ).GetDirectionalLight( mSunDirLightID ) )
	{
		JonsEngine::Scene& scene = sceneMgr.GetScene( mID );

		scene.SetAmbientLight( Vec4( 0.1f ) );

		DirectionalLight& sun = scene.GetDirectionalLight( mSunDirLightID );
		sun.SetLightColor( Vec4( 6.0f ) );
		sun.SetLightDirection( Vec3( -0.735238075f, -0.607724974f, -0.645194054f ) );

		SceneNode& nodeTerrain = scene.GetSceneNode( mNodeTerrain );
		SceneNode& nodeTerrainTest = scene.GetSceneNode( mNodeTerrainTest );
		nodeTerrain.TranslateNode( Vec3( 0.0f, 0.0f, 0.0f ) );
		nodeTerrainTest.TranslateNode( Vec3( 0.0f, 0.0f, 0.0f ) );
		nodeTerrainTest.SetVisible( false );

		scene.SetSkybox( mSkybox );

		scene.GetSceneCamera().TranslateCamera( { 1248.0f, 350.0f, -820.0f } );
	}

	SceneTessellation::~SceneTessellation()
	{
		// TODO: cleanup...
	}


	void SceneTessellation::Update()
	{
		//mSun.Update();
	}

	void SceneTessellation::FlipTerrain()
	{
		JonsEngine::Scene& scene = mSceneMgr.GetScene( mID );

		SceneNode& nodeTerrain = scene.GetSceneNode( mNodeTerrain );
		SceneNode& nodeTerrainTest = scene.GetSceneNode( mNodeTerrainTest );

		nodeTerrain.SetVisible( !nodeTerrain.IsVisible() );
		nodeTerrainTest.SetVisible( !nodeTerrainTest.IsVisible() );

		if ( nodeTerrainTest.IsVisible() )
		{
			scene.GetSceneCamera().SetPosition( { 0.0f, 200.0f, 0.0f } );
		}
		else
		{
			scene.GetSceneCamera().SetPosition( { 390.0f, 60.0f, -576.0f } );
		}
	}

	JonsEngine::Scene& SceneTessellation::GetJonsScene()
	{
		return mSceneMgr.GetScene( mID );
	}

	JonsEngine::Terrain& SceneTessellation::GetTerrain()
	{
		return mSceneMgr.GetScene( mID ).GetTerrain( mTerrain );
	}
}
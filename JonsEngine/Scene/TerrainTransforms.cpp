#include "Scene/TerrainTransforms.h"
#include "Core/Math/MathUtils.h"

#include <algorithm>

namespace JonsEngine
{
	namespace
	{
		int32_t GetNumColumns( const uint32_t patchSize, const uint32_t width )
		{
			return width / patchSize;
		}

		int32_t GetNumRows( const uint32_t patchSize, const uint32_t height )
		{
			return height / patchSize;
		}

		void GetPatchHeight( float& minY, float& maxY, const std::vector<uint8_t>& heightmapData, float heightmapScale, uint32_t width, uint32_t textureExtentWidth, uint32_t textureExtentHeight, uint32_t rowIndex, uint32_t colIndex )
		{
			uint32_t currX = colIndex * textureExtentWidth, currY = rowIndex * textureExtentHeight;
			uint32_t endX = ( colIndex + 1 ) * textureExtentWidth, endY = ( rowIndex + 1 ) * textureExtentHeight;

			minY = std::numeric_limits<float>::max(), maxY = std::numeric_limits<float>::min();
			for ( uint32_t y = currY; y < endY; ++y )
			{
				for ( uint32_t x = currX; x < endX; ++x )
				{
					uint32_t textureIndex = ( y * width ) + x;
					float val = Normalize( heightmapData[ textureIndex ] ) * heightmapScale;

					maxY = std::max( maxY, val );
					minY = std::min( minY, val );
				}
			}
		}
	}

	TerrainTransforms::TerrainTransforms(const IDMap<Terrain>& terrainLookup, const IDMap<TerrainData>& terrainDataLookup, const IDMapTree<SceneNode>& sceneNodeLookup) :
		mTerrainLookup(terrainLookup),
		mTerrainDataLookup(terrainDataLookup),
		mSceneNodeLookup(sceneNodeLookup)
	{
	}

	void TerrainTransforms::AddDirty(TerrainID ID)
	{
		auto iter = std::find( mDirtyTransforms.cbegin(), mDirtyTransforms.cend(), ID );
		if ( iter != mDirtyTransforms.cend() )
			return;

		if ( !HasAddedDirty( ID ) )
			mDirtyTransforms.push_back( ID );
	}

	uint32_t TerrainTransforms::UpdateTransforms( uint32_t patchMinSize )
	{
		for ( const TerrainTransformData& transformData : mTerrainTransforms )
		{
			//if ( transformData.mQuadTree.GetPatchMinSize() != patchMinSize )
			//	mDirtyTransforms.emplace_back( transformData.mID );
		}

		uint32_t numUpdated = 0;
		for (TerrainID ID : mDirtyTransforms)
		{
			const Terrain& terrain = mTerrainLookup.GetItem(ID);
			const TerrainData& terrainData = mTerrainDataLookup.GetItem(terrain.GetTerrainData());
			const Mat4& worldTransform = mSceneNodeLookup.GetNode(terrain.GetSceneNode()).GetWorldTransform();

			RemoveIfAdded( ID );
			RebuildTransforms( ID, worldTransform, terrain, terrainData, patchMinSize );

			++numUpdated;
		}

		mDirtyTransforms.clear();

		return numUpdated;
	}

	uint32_t TerrainTransforms::GetNumEntries() const
	{
		return static_cast<uint32_t>( mTerrainTransforms.size() );
	}

	const TerrainQuadTree& TerrainTransforms::GetQuadTree( TerrainID ID ) const
	{
		if ( mTerrainQuadTreeMap.find( ID ) == mTerrainQuadTreeMap.cend() )
		{
			const static TerrainQuadTree gStaticEmptyTree;
			return gStaticEmptyTree;
		}

		return mTerrainQuadTreeMap.at( ID );
	}

	void TerrainTransforms::RemoveIfAdded( TerrainID ID )
	{
		auto funcIDComparison = [ ID ]( const TerrainTransformData& metadata ) { return ID == metadata.mID; };
		auto beginIter = mTerrainTransforms.begin(), endIter = mTerrainTransforms.end();
		mTerrainTransforms.erase( std::remove_if( beginIter, endIter, funcIDComparison ), endIter );
	}

	bool TerrainTransforms::HasAddedDirty( TerrainID ID ) const
	{
		auto beginIter = mDirtyTransforms.cbegin(), endIter = mDirtyTransforms.cend();
		auto iter = std::find( beginIter, endIter, ID );

		return iter != endIter;
	}

	void TerrainTransforms::RebuildTransforms( TerrainID ID, const Mat4& worldTransform, const Terrain& terrain, const TerrainData& terrainData, uint32_t patchMinSize )
	{
		uint32_t heightmapWidth = terrainData.GetWidth(), heightmapHeight = terrainData.GetHeight();
		const std::vector<uint8_t>& heightmapData = terrainData.GetHeightMapData();

		// TODO
		patchMinSize = 32;
		assert( patchMinSize && heightmapWidth && heightmapHeight );

		int32_t numWidth = GetNumColumns( patchMinSize, heightmapWidth ), numHeight = GetNumRows( patchMinSize, heightmapHeight );
		int32_t gridSize = numWidth * numHeight;

		std::vector<Mat4> transforms;
		std::vector<Mat4> AABBTransforms;
		transforms.reserve( gridSize );
		AABBTransforms.reserve( gridSize );

		for ( int32_t rowNum = -numHeight; rowNum < numHeight; rowNum += 2 )
		{
			for ( int32_t colNum = -numWidth; colNum < numWidth; colNum += 2 )
			{
				float minY, maxY;
				uint32_t textureExtentWidth = heightmapWidth / numWidth, textureExtentHeight = heightmapHeight / numHeight;
				uint32_t rowIndex = ( rowNum + numHeight ) / 2, colIndex = ( colNum + numWidth ) / 2;
				GetPatchHeight( minY, maxY, heightmapData, terrain.GetHeightScale(), heightmapWidth, textureExtentWidth, textureExtentHeight, rowIndex, colIndex );

				Mat4 patchScaleTransform = glm::scale( Vec3( patchMinSize, 1.0f, patchMinSize ) );
				transforms.emplace_back( worldTransform * patchScaleTransform * glm::translate( Vec3( colNum / 2, 0.0f, rowNum / 2 ) ) );

				float minMaxHeightDiff = maxY - minY;
				AABBTransforms.emplace_back( glm::translate( Vec3( 0.0f, minMaxHeightDiff / 2.0f, 0.0f ) ) * glm::scale( Vec3( 1.0, minMaxHeightDiff, 1.0f ) ) *  transforms.back() );
			}
		}

		mTerrainTransforms.emplace_back( ID, std::move( transforms ), std::move( AABBTransforms ) );
		// TODO patchMinSize
		mTerrainQuadTreeMap.emplace( std::piecewise_construct, std::forward_as_tuple( ID ), std::forward_as_tuple( heightmapData, heightmapWidth, heightmapHeight, 4, terrain.GetHeightScale(), worldTransform ) );
	}
}

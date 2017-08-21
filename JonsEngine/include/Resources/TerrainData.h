#pragma once

#include "include/Core/Types.h"
#include "include/Core/Math/AABB.h"
#include "include/Core/Containers/IDMap.hpp"

#include <string>

namespace JonsEngine
{
	class TerrainData
	{
	public:
		TerrainData(const std::string& name, float minElevation, float maxElevation, float width, float height, DX11TextureID heightMap);

		const std::string& GetName() const;
		const AABB& GetStaticAABB() const;
		DX11TextureID GetHeightMap() const;


	private:
		std::string mName;
		AABB mStaticAABB;
		DX11TextureID mHeightMap;
	};

	typedef IDMap<TerrainData>::ItemID TerrainDataID;
	static const TerrainDataID INVALID_TERRAIN_DATA_ID = IDMap<TerrainDataID>::INVALID_ITEM_ID;
}
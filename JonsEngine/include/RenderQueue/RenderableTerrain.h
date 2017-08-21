#pragma once

#include "include/RenderQueue/RenderableMesh.h"
#include "include/Core/Types.h"

namespace JonsEngine
{
	struct RenderableTerrain
	{
		RenderableTerrain();
		RenderableTerrain(const Mat4& transform, DX11TextureID heightmap);


		Mat4 mWorldTransform;
		DX11TextureID mHeightMap;
	};
}
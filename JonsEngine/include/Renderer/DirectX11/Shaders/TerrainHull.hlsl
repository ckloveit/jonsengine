#ifndef TERRAIN_HULL_HLSL
#define TERRAIN_HULL_HLSL

#include "TerrainCommon.hlsl"

PatchTess PatchHS(InputPatch<VertexOut, 3> patch, uint patchID : SV_PrimitiveID)
{
	PatchTess pt;

	// Average tess factors along edges, and pick an edge tess factor for 
	// the interior tessellation.  It is important to do the tess factor
	// calculation based on the edge properties so that edges shared by 
	// more than one triangle will have the same tessellation factor.  
	// Otherwise, gaps can appear.
	pt.mEdgeTess[0] = 0.5f*(patch[1].mTessFactor + patch[2].mTessFactor);
	pt.mEdgeTess[1] = 0.5f*(patch[2].mTessFactor + patch[0].mTessFactor);
	pt.mEdgeTess[2] = 0.5f*(patch[0].mTessFactor + patch[1].mTessFactor);
	pt.mInsideTess = pt.mEdgeTess[0];

	return pt;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchHS")]
HullOut hull_main(InputPatch<VertexOut, 3> p, uint i : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
	HullOut hout;

	hout.mPositionW = p[i].mPositionW;
	hout.mNormalW = p[i].mNormalW;
	hout.mTexcoordW = p[i].mTexcoordW;

	return hout;
}

#endif
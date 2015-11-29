//--------------------------------------------------------------------------------------
// File: AppendVoxelList.hlsl
//
// HLSL file for appending voxel data to a output buffer.
//--------------------------------------------------------------------------------------

#include "voxel.fx"	// Voxel format header file.

//--------------------------------------------------------------------------------------
// Thread constant.
//--------------------------------------------------------------------------------------
static const int NUM_THREAD = 64;

//--------------------------------------------------------------------------------------
// Constant Buffer.
//--------------------------------------------------------------------------------------
cbuffer data
{
	float gVoxelDim;
	float gNum;
}
//-----------------------------------------------------------------------------------------
// Textures and Buffers.
//-----------------------------------------------------------------------------------------
Texture3D gVoxelData : register(t0);
AppendStructuredBuffer< voxelViewData> gVoxelListUAV : register(u0);

//-----------------------------------------------------------------------------------------
// Determine and process voxel data, and then append data to the output buffer (gVoxelListUAV).
//-----------------------------------------------------------------------------------------
[numthreads(NUM_THREAD, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
	// Use DispatchThreadID to access an address of Texture3D.
	uint sliceNum = gVoxelDim*gVoxelDim;
	uint z = DTid.x / (sliceNum);
	uint temp = DTid.x % (sliceNum);
	uint y = temp / (uint)gVoxelDim;
	uint x = temp % (uint)gVoxelDim;

	float4 data = gVoxelData[uint3(x, y, z)];

		// Any data?
		if (any(data))
		{
			voxelViewData viewData;
			viewData.pos = uint3(x, y, z);
			viewData.normal = data.xyz;
			viewData.ID = data.w;
			// Append to RWBuffer.
			gVoxelListUAV.Append(viewData);

		}
}

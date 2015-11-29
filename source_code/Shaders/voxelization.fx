//--------------------------------------------------------------------------------------
// File: voxelization.fx
//
// FX file for voxelizing 3D models to voxels.
//--------------------------------------------------------------------------------------


#include "voxel.fx"	// Voxel format header file.

// Constant.
#define D3DX_PI 3.1415926536

//--------------------------------------------------------------------------------------
// Constant Buffers.
//--------------------------------------------------------------------------------------
cbuffer cbPerFrame : register(b0)
{
	float2 gRasterSize;
	float4x4 gView;
	int gVoxelDim;
	float3 gVoxelSize;
};

cbuffer cbPerObject : register(b1)
{
	float gObjectID;
	float4x4 gWorld;
};

//--------------------------------------------------------------------------------------
// Render States.
//--------------------------------------------------------------------------------------

// RasterizerState for disabling culling.
RasterizerState RS_CullDisabled
{
	CullMode = None;
};


// BlendState for disabling blending.
BlendState NoBlending
{
	AlphaToCoverageEnable = FALSE;
	BlendEnable[0] = FALSE;
};

// DepthStencilState for disabling depth writing.
DepthStencilState DisableDepth
{
	DepthEnable = FALSE;
	DepthWriteMask = 0;
};

//--------------------------------------------------------------------------------------
// Read&Write texture for saving voxel data.
//--------------------------------------------------------------------------------------
RWTexture3D<float4> gTargetUAV;

//--------------------------------------------------------------------------------------
// Shader input/output structure.
//--------------------------------------------------------------------------------------
struct VS_IN
{
	float3 posL    : POSITION;
	float3 norL    : NORMAL;
};

struct VS_OUT
{
	float4 posW    : TEXCOORD0;		// World-space position.
	float4 posV    : SV_Position;	// View-space position.
	float3 norW    : TEXCOORD1;
};


struct GS_OUT
{
	float4 Pos     : SV_Position;	// Position for rasterization.
	float4 posV    : TEXCOORD0;		// View-space position for voxeliztion.
	float3 norW    : TEXCOORD2;		// World-space normal.
};

//--------------------------------------------------------------------------------------
// Vertex shader to transform local-space normals and positions to world-space.
//--------------------------------------------------------------------------------------
VS_OUT VS_MAIN(VS_IN vIn)
{
	VS_OUT vOut;

	// Transform to world space.
	vOut.posW = mul(float4(vIn.posL, 1.0f), gWorld);
	vOut.norW = mul(float4(vIn.norL, 0.0f), gWorld).xyz;

	vOut.posV = mul(float4(vOut.posW.xyz, 1.0f), gView);


	return vOut;
}

//--------------------------------------------------------------------------------------
// Geometry shader to transform triangles to make sure that all triangles are ready for voxelization.
//--------------------------------------------------------------------------------------
[maxvertexcount(3)]
void GS_MAIN(triangle VS_OUT input[3], inout TriangleStream<GS_OUT> triStream)
{
	// Initial data.
	float3 newPos[3];
	float3 centerPos = float3(0, 0, 0);
	float3 normal = float3(0, 0, 0);

	float4x4 gRot = { 1.f, 0.f, 0.f, 0.f,0.f, 1.f, 0.f, 0.f,0.f, 0.f, 1.f, 0.f,
					0.f, 0.f, 0.f, 1.f };

	// Find the center of triangle.
	for (int j = 0; j < 3; j++)
	{
		normal += input[j].norW;
		centerPos += input[j].posW.xyz;

	}
	centerPos /= 3;
	normal /= 3;

	// Rasterizing triangles may clip some parts of a triangle or even an entire triangle.
	// To make sure that every triangle is complete, the shader transform triangles.
	// Rotate triangle to be opposite with view direction to avoid clipping problem.

	// Use float3(0,0,1) as view direction,
	// find the orthogonal direction of this triangle,
	float eye_angle = dot(normal, float3(0, 0, 1));
	float3 v = cross(normal, float3(0, 0, 1));
	v = normalize(v);

	// Make sure v is not float3(0,0,0),
	if (dot(v, v) > 0) {

		// Create a rotation matrix to rotate the triangle to the opposite with view direction.
		float cost = eye_angle, sint = pow(1 - eye_angle*eye_angle, 0.5f), one_sub_cost = 1 - cost;
		float4x4 RotToCam = { v.x * v.x * one_sub_cost + cost, v.x * v.y * one_sub_cost + v.z * sint, v.x * v.z * one_sub_cost - v.y * sint, 0, \
			v.x * v.y * one_sub_cost - v.z * sint, v.y * v.y * one_sub_cost + cost, v.y * v.z * one_sub_cost + v.x * sint, 0, \
			v.x * v.z * one_sub_cost + v.y * sint, v.y * v.z * one_sub_cost - v.x * sint, v.z * v.z * one_sub_cost + cost, 0, \
			0, 0, 0, 1 };
		gRot = RotToCam;
	}

	// Apply rotation.
	for (int k = 0; k < 3; k++)
	{
		newPos[k] = input[k].posW.xyz - centerPos;
		newPos[k] = mul(float4(newPos[k], 1.0f), gRot).xyz;
	}

	// Build a bounding box of this triangle in order to control the density of pixels that a triangle can produce.
	float minX = min(min(newPos[0].x, newPos[1].x), newPos[2].x);
	float maxX = max(max(newPos[0].x, newPos[1].x), newPos[2].x);
	float minY = min(min(newPos[0].y, newPos[1].y), newPos[2].y);
	float maxY = max(max(newPos[0].y, newPos[1].y), newPos[2].y);

	float2 RasterSize = (2 / float2(maxX - minX, maxY - minY));

	// Apply orthogonal projection.
	for (int i = 0; i < 3; i++)
	{
		GS_OUT output;
		// Transform x,y to [-1,1].
		newPos[i].xy = (newPos[i].xy - float2(minX, minY))  * RasterSize.xy - 1;
		output.Pos = float4(newPos[i].xy, 1, 1);
		output.posV = input[i].posV;	// Assign view-space voxel positions.
		output.norW = input[i].norW;

		triStream.Append(output);
	}


	triStream.RestartStrip();



}

//--------------------------------------------------------------------------------------
// Pixel shader for saving data to a shader resource (Texture3D).
//--------------------------------------------------------------------------------------
float4 PS_MAIN(GS_OUT pIn) : SV_Target
{
	// Use view-space position to access an address of Texture3D.
	float x = pIn.posV.x / gVoxelSize.x;
	float y = pIn.posV.y / gVoxelSize.y;
	float z = pIn.posV.z / gVoxelSize.z;
	x = x + gVoxelDim / 2;
	y = y + gVoxelDim / 2;

	// Store voxels which are inside voxel-space boundary.
	if (x >= 0 && x < gVoxelDim && y >= 0 && y < gVoxelDim && z >= 0 && z < gVoxelDim)
	{
		float3 normal = (pIn.norW + 1.f)*0.5f;	// Transform normal data from -1~1 to 0~1 for DXGI_FORMAT_R8G8B8A8_UNORM format.

		// Compress 3 components to 2 components.
		half scale = 1.7777;
		half2 enc = normal.xy / (normal.z + 1);
		enc /= scale;
		enc = enc*0.5 + 0.5;
		normal.xyz = float3(enc.xy, 0);

		gTargetUAV[uint3(x, y, z)] = float4(normal, gObjectID);

	}

	// A trivial return-value because I don't set any render targets.
	return float4(0, 0, 0, 0);
}

// A technique uses RWTexture to save voxels.
technique11 RWTexVoxelTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_MAIN()));
		SetGeometryShader(CompileShader(gs_5_0, GS_MAIN()));
		SetPixelShader(CompileShader(ps_5_0, PS_MAIN()));
		SetDepthStencilState(DisableDepth, 0);
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(RS_CullDisabled);
	}
}

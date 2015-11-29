//--------------------------------------------------------------------------------------
// File: voxelViewer.fx
//
// FX file for previewing voxel data
//--------------------------------------------------------------------------------------

#include "voxel.fx"

Texture2D gEdge;
Texture3D gVoxelList;
Texture1D gColorTable;

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
cbuffer cbPerFrame : register(b0)
{
	float4x4 gViewProj;
	float3 gLightDir;
	float3 gLightIntense;
	float gVoxelScale;		// voxel scale
};

//--------------------------------------------------------------------------------------
// Render States
//--------------------------------------------------------------------------------------

// Using ordinary depth comparsion
DepthStencilState LessEqualDSS
{
	DepthEnable = TRUE;
	DepthFunc = LESS_EQUAL;
};

// Render States for culling back
RasterizerState RS_Cull
{
	CullMode = Back;
};


//-----------------------------------------------------------------------------------------
// Sampler
//-----------------------------------------------------------------------------------------
SamplerState gAnisotropicSam
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 16;
};

//-----------------------------------------------------------------------------------------
// Arrays for generating cube in the geometry shader
//-----------------------------------------------------------------------------------------

const float3 boxOffset[24] =
{
	1, -1, 1,
	1, 1, 1,
	-1, -1, 1,
	-1, 1, 1,

	-1, 1, 1,
	-1, 1, -1,
	-1, -1, 1,
	-1, -1, -1,

	1, 1, -1,
	1, 1, 1,
	1, -1, -1,
	1, -1, 1,

	-1, 1, -1,
	1, 1, -1,
	-1, -1, -1,
	1, -1, -1,

	1, 1, 1,
	1, 1, -1,
	-1, 1, 1,
	-1, 1, -1,

	-1, -1, -1,
	1, -1, -1,
	-1, -1, 1,
	1, -1, 1
};

const float2 boxTexArray[4] =
{
	0, 0,
	0, 1,
	1, 0,
	1, 1
};

const float3 boxNormalArray[6] =
{
	0, 0, 1,
	-1, 0, 0,
	1, 0, 0,
	-1, 0, 0,
	0, 1, 0,
	0, -1, 0
};

//--------------------------------------------------------------------------------------
// Shader input/output structure
//--------------------------------------------------------------------------------------

struct VS_IN
{

	uint index : SV_VertexID;	//using index to access voxel data


};
struct VS_OUT
{
	float3 posL		 : POSITION;
	float3 normal    : NORMAL;
	float id : ID;	//object ID
};

struct GS_OUT
{
	float4 Pos				: SV_Position;
	float3 worldNormal		: Position;		//normal for lighting cubes
	float3 Normal			: NORMAL;		//normal of voxel data
	float2 texcoord			: TEXCOORD;
	float id : ID;
};

//--------------------------------------------------------------------------------------
// Vertex shader to load voxel data
//--------------------------------------------------------------------------------------
VS_OUT VS(VS_IN vIn)
{
	// Access a voxel of Texture3D by vertex index
	float w, h, d;
	gVoxelList.GetDimensions(w, h, d);
	uint VoxelDim = w;
	uint sliceNum = VoxelDim*VoxelDim;
	uint z = vIn.index / (sliceNum);
	uint temp = vIn.index % (sliceNum);
	uint y = temp / (uint)VoxelDim;
	uint x = temp % (uint)VoxelDim;
	uint3 pos = uint3(x, y, z);

	// Output voxel data to geometry shader
	VS_OUT output;
	output.posL = pos;
	output.normal = gVoxelList[pos].xyz;
	output.id = gVoxelList[pos].w;

	// Decompress normal from 2 components to 3 components
	half scale = 1.7777;
	half3 nn =
	output.normal*half3(2 * scale, 2 * scale, 0) +
	half3(-scale, -scale, 1);
	half g = 2.0 / dot(nn.xyz, nn.xyz);
	half3 n;
	output.normal.xy = g*nn.xy;
	output.normal.z = g - 1;

	return output;
}


//--------------------------------------------------------------------------------------
// Geometry shader to generate cubes in order to visualize voxel data
//--------------------------------------------------------------------------------------
[maxvertexcount(24)]
void GS(point VS_OUT input[1], inout TriangleStream<GS_OUT> triStream)
{
		// Only process vertex with voxel data
		if (dot(float3(1, 1, 1), input[0].normal.xyz) > 0.01f)
		{
			// Generate vertexes for six faces
			for (int i = 0; i < 6; i++)
			{
				// Generate four vertexes for a face
				for (int j = 0; j < 4; j++)
				{
					GS_OUT outGS;
					// Create cube vertexes with boxOffset array
					float3 vertex = input[0].posL.xyz + boxOffset[i * 4 + j] * 0.5f;

					// Output both geometry data for rendering a cube and voxel data for visulization
					outGS.Pos = mul(float4(vertex*gVoxelScale, 1), gViewProj);
					outGS.worldNormal = boxNormalArray[i];
					outGS.texcoord = boxTexArray[j];
					outGS.Normal = input[0].normal;
					outGS.id = input[0].id;

					triStream.Append(outGS);

				}
				triStream.RestartStrip();
			}
		}

}

//--------------------------------------------------------------------------------------
// Pixel shader to show different voxel data and how voxel should look like
//--------------------------------------------------------------------------------------

// Show normal data
float4 ShoNormal_PS(GS_OUT pIn) : SV_Target
{
	
	float4 normal = float4(pIn.Normal, 1);

	float3 output = gEdge.Sample(gAnisotropicSam, pIn.texcoord);
	normal.rgb *= output;

	return normal;
}

// Show binary data
float4 Voxel_PS(GS_OUT pIn) : SV_Target
{
	float3 LightPos = gLightDir;
	// Clamp light 0.5 to 1
	float nDotL = saturate((1.5f + dot((LightPos), (pIn.worldNormal))) / 2);
	float3 output = gEdge.Sample(gAnisotropicSam, pIn.texcoord);

	output = output*nDotL;

	return float4(output, 1);
}

// Show object ID data
float4 ShowID_PS(GS_OUT pIn) : SV_Target
{
	float3 LightPos = gLightDir;
	// Clamp light 0.5 to 1
	float nDotL = saturate((1.5f + dot((LightPos), (pIn.worldNormal))) / 2);

	float3 output = gEdge.Sample(gAnisotropicSam, pIn.texcoord);
	output = gColorTable.Sample(gAnisotropicSam, pIn.id)*output*nDotL;	//load a color from color table to visualize different objects

	return float4(output, 1);

}

// 3 passes for visualizing different information of voxel data
technique11 ShowVoxelRenderTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(CompileShader(gs_5_0, GS()));
		SetPixelShader(CompileShader(ps_5_0, ShoNormal_PS()));
		SetRasterizerState(RS_Cull);
		SetDepthStencilState(LessEqualDSS, 0);
	}
	pass P1
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(CompileShader(gs_5_0, GS()));
		SetPixelShader(CompileShader(ps_5_0, Voxel_PS()));
		SetRasterizerState(RS_Cull);
		SetDepthStencilState(LessEqualDSS, 0);
	}
	pass P2
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(CompileShader(gs_5_0, GS()));
		SetPixelShader(CompileShader(ps_5_0, ShowID_PS()));
		SetRasterizerState(RS_Cull);
		SetDepthStencilState(LessEqualDSS, 0);
	}
}





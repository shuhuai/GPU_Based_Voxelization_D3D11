//--------------------------------------------------------------------------------------
// File: ShowRawData.fx
//
// FX file for visualizing raw Texture3D data.
//--------------------------------------------------------------------------------------

float index;	// Display this slice index.

Texture3D gTarget;	// Voxel data.

SamplerState gPointSam
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Clamp;
	AddressV = Clamp;
	AddressW = Clamp;
};


//--------------------------------------------------------------------------------------
// Shader input/output structure.
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float4 Position : POSITION0;
	float2 texCoord: TEXCOORD0;

};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float2 texCoord : TEXCOORD0;

};

//--------------------------------------------------------------------------------------
// Vertex shader to draw a full-screen quad,
// and transform vertex positions from -1~+1 to 0~1.
//--------------------------------------------------------------------------------------
VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output;

	Output.Position = float4(Input.Position.xy, 0, 1);

	Output.texCoord.x = 0.5 * (1 + Input.Position.x );
	Output.texCoord.y = 1 - 0.5 * (1 - Input.Position.y);

	return(Output);

}


//--------------------------------------------------------------------------------------
// Pixel shader to draw a specific slice in Texture3D.
//--------------------------------------------------------------------------------------
float4 ps_main(VS_OUTPUT Input) : SV_Target
{

	float4 color = gTarget.SampleLevel(gPointSam, float3(Input.texCoord, index), 0);

	return color;

}

technique11 RTT
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, vs_main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, ps_main()));

	}
}
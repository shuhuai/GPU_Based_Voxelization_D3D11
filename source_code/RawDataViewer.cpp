//--------------------------------------------------------------------------------------
// File: RawDataViewer.cpp
//--------------------------------------------------------------------------------------
#include "RawDataViewer.h"

RawDataViewer::RawDataViewer()
{

}
RawDataViewer::~RawDataViewer()
{
	SAFE_RELEASE(mRttFX);
	SAFE_RELEASE(mVB);
}
void RawDataViewer::Init(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{

	md3dDevice = pDevice;
	mDeviceContext = pDeviceContext;

	// Create a full-screen quad.

	VertexSlip v[6];

	v[0] = VertexSlip(-1.0f, 1.0f, 0.0f, 0.0f, 0.0f);
	v[1] = VertexSlip(1.0f, 1.0f, 0.0f, 1.0f, 0.0f);
	v[2] = VertexSlip(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f);
	v[3] = VertexSlip(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f);
	v[4] = VertexSlip(1.0f, 1.0f, 0.0f, 1.0f, 0.0f);
	v[5] = VertexSlip(1.0f, -1.0f, 0.0f, 1.0f, 1.0f);

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(v) * 6;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = sizeof(v);
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = v;
	vinitData.SysMemPitch = 0;
	vinitData.SysMemSlicePitch = 0;

	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));

	// Load data visualization shaders.
	ID3D10Blob* pCode;
	ID3D10Blob* pError;

	HRESULT hr = 0;
	hr = D3DX11CompileFromFile("Shaders/ShowRawData.fx", NULL, NULL, NULL, "fx_5_0", D3D10_SHADER_PREFER_FLOW_CONTROL | D3D10_SHADER_OPTIMIZATION_LEVEL3, NULL, NULL, &pCode, &pError, NULL);

	if (FAILED(hr))
	{
		if (pError)
		{
			MessageBoxA(0, (char*)pError->GetBufferPointer(), 0, 0);
			SAFE_RELEASE(pError);
		}
		DXTraceW(_T(__FILE__), __LINE__, hr, L"D3DX11CreateEffectFromFile", TRUE);
	}

	HR(D3DX11CreateEffectFromMemory(pCode->GetBufferPointer(), pCode->GetBufferSize(), NULL, md3dDevice, &mRttFX));


	mRttTech = mRttFX->GetTechniqueByName("RTT");
	mTargetVar = mRttFX->GetVariableByName("gTarget")->AsShaderResource();
	mIndexVar = mRttFX->GetVariableByName("index")->AsScalar();

	// Create the input layout.
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	D3DX11_PASS_DESC PassDesc;
	mRttTech->GetPassByIndex(0)->GetDesc(&PassDesc);
	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &mVertexSlipLayout));
}

void RawDataViewer::ShowRawVoxeldata(ID3D11ShaderResourceView*	pData, float fIndex)
{
	// Find a slice by an index, and then render it on a full-screen quad.
	mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mTargetVar->SetResource(pData);
	mIndexVar->SetFloat(fIndex);

	mDeviceContext->IASetInputLayout(mVertexSlipLayout);
	UINT stride = sizeof(VertexSlip);
	UINT offset = 0;
	mDeviceContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	ID3DX11EffectPass* pass = mRttTech->GetPassByIndex(0);

	pass->Apply(0, mDeviceContext);
	mDeviceContext->Draw(6, 0);

}
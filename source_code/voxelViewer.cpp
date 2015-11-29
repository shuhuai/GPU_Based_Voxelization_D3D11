//--------------------------------------------------------------------------------------
// File: voxelViewer.cpp
//--------------------------------------------------------------------------------------

#include"voxelViewer.h"

voxelViewer::voxelViewer()
{

}
voxelViewer::~voxelViewer()
{

}

void voxelViewer::Init(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
	md3dDevice = pDevice;
	mDeviceContext = pDeviceContext;
	// Create a color table with 256 colors.
	createColorTable(256);
	// Initialize shader programs.
	buildFX();

}
void voxelViewer::buildFX()
{
	DWORD shaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
	ID3D10Blob* pCode;
	ID3D10Blob* pError;

	HRESULT hr = 0;
	hr = D3DX11CompileFromFile("Shaders/voxelViewer.fx", NULL, NULL, NULL, "fx_5_0", shaderFlags, NULL, NULL, &pCode, &pError, NULL);

	if (FAILED(hr))
	{
		if (pError)
		{
			MessageBoxA(0, (char*)pError->GetBufferPointer(), 0, 0);
			SAFE_RELEASE(pError);
		}
		DXTraceW(_T(__FILE__), __LINE__, hr, L"D3DX11CreateEffectFromFile", TRUE);
	}

	HR(D3DX11CreateEffectFromMemory(pCode->GetBufferPointer(), pCode->GetBufferSize(), NULL, md3dDevice, &mFX));
	mTech = mFX->GetTechniqueByName("ShowVoxelRenderTech");
	mWVPVar = mFX->GetVariableByName("gViewProj")->AsMatrix();
	mLightDirVar = mFX->GetVariableByName("gLightDir")->AsVector();
	mEdgeTexVar = mFX->GetVariableByName("gEdge")->AsShaderResource();
	mVoxelScaleVar = mFX->GetVariableByName("gVoxelScale")->AsScalar();
	mVoxelVar = mFX->GetVariableByName("gVoxelList")->AsShaderResource();
	mColorTableVar = mFX->GetVariableByName("gColorTable")->AsShaderResource();
	// Set a texture for voxels.
	ID3D11ShaderResourceView* edgeTex;
	D3DX11CreateShaderResourceViewFromFile(md3dDevice, "..//media//edge.png", 0, 0, &edgeTex, 0);
	mEdgeTexVar->SetResource(edgeTex);
	mColorTableVar->SetResource(mColorTex);
	// Set a light direction.
	D3DXVECTOR3 dir = D3DXVECTOR3(1, 3.5, 0.8);
	D3DXVec3Normalize(&dir, &dir);
	mLightDirVar->SetFloatVector((float*)&dir);

}

void voxelViewer::ShowVoxels(ID3D11ShaderResourceView* pVoxel, int iNumber, float fScale, int iPass)
{

	mVoxelVar->SetResource(pVoxel);

	float voxelScale = fScale / (float)m_iRes;
	mVoxelScaleVar->SetFloat((float)(voxelScale));
	mWVPVar->SetMatrix((float*)&(g_Camera->viewProj()));

	// To render cubes, we use Vertex_ID to load a voxel in Texture3D and then generate a cube,
	// so we don't need to bind anything to IA.
	mDeviceContext->IASetInputLayout(NULL);
	mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	mTech->GetPassByIndex(iPass)->Apply(0, mDeviceContext);
	mDeviceContext->Draw(iNumber, 0);
}

void voxelViewer::createColorTable(int number)
{
	D3D11_TEXTURE1D_DESC text1_desc;

	ZeroMemory(&text1_desc, sizeof(D3D11_TEXTURE1D_DESC));

	text1_desc.Width = number;
	text1_desc.MipLevels = 1;
	text1_desc.ArraySize = 1;
	text1_desc.Usage = D3D11_USAGE_IMMUTABLE;
	text1_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	text1_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Random colors for my color table.
	UINT* pData = new UINT[text1_desc.Width];
	srand(time(NULL));
	for (int i = 0; i < text1_desc.Width; i++)
	{
		UINT col;
		UINT r = rand() % 256;
		UINT g = rand() % 256;
		UINT b = rand() % 256;
		// Combine R,G,B value to a 32-bit variable.
		pData[i] = r | g * 512 | b * 512 * 512;
	}
	// Create a SRV for my color table.
	D3D11_SUBRESOURCE_DATA sr_data;
	ZeroMemory(&sr_data, sizeof(D3D11_SUBRESOURCE_DATA));
	sr_data.pSysMem = pData;

	ID3D11Texture1D* pTexture1D = nullptr;
	HR(md3dDevice->CreateTexture1D(&text1_desc, &sr_data, &pTexture1D));

	D3D11_SHADER_RESOURCE_VIEW_DESC srcDesc;
	ZeroMemory(&srcDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srcDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srcDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
	srcDesc.Texture1D.MostDetailedMip = 0;
	srcDesc.Texture1D.MipLevels = 1;
	HR(md3dDevice->CreateShaderResourceView(pTexture1D, &srcDesc, &mColorTex));
}

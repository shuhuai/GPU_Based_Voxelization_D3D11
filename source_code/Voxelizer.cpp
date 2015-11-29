//--------------------------------------------------------------------------------------
// File: Voxelizer.cpp
//--------------------------------------------------------------------------------------

#include "Voxelizer.h"
#include <iostream>
#include <fstream>


Voxelizer::Voxelizer()
{

}

Voxelizer::~Voxelizer()
{
	SAFE_RELEASE(mVolumeTex);
	SAFE_RELEASE(mUAV);
	SAFE_RELEASE(mSRV);
}

void Voxelizer::ResetVoxelRes(UINT uRes, DXGI_FORMAT ColorFormat)
{
	m_iWidth = uRes;
	m_iHeight = uRes;
	m_iDepth = uRes;
	mColorMapFormat = ColorFormat;

	SAFE_RELEASE(mVolumeTex);
	SAFE_RELEASE(mUAV);
	SAFE_RELEASE(mSRV);

	build3DUAV();

	mResVar->SetInt(uRes);
}

void Voxelizer::Init(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, UINT uRes, DXGI_FORMAT ColorFormat)
{
	// Initialize parameters.
	mDeviceContext = pDeviceContext;
	md3dDevice = pDevice;

	m_iWidth = uRes; 
	m_iHeight = uRes;
	m_iDepth = uRes;
	mColorMapFormat = ColorFormat;
	m_bSetView = false;

	mViewport.TopLeftX = 0;
	mViewport.TopLeftY = 0;
	mViewport.MinDepth = 0.0f;
	mViewport.MaxDepth = 1.0f;
	
	// Create voxel GPU resources.
	build3DUAV();
	// Create voxelization shaders.
	buildFX();

	mResVar->SetInt(uRes);
}



void Voxelizer::buildFX()
{
	// Compile FX files.
	DWORD shaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
	ID3D10Blob* pCode;
	ID3D10Blob* pError;

	HRESULT hr = 0;
	hr = D3DX11CompileFromFile("Shaders/voxelization.fx", NULL, NULL, NULL, "fx_5_0", shaderFlags, NULL, NULL, &pCode, &pError, NULL);

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

	// Link effect variables to shader parameters.
	mTech = mFX->GetTechniqueByName("RWTexVoxelTech");
	mViewMatVar = mFX->GetVariableByName("gView")->AsMatrix();
	mVoxelDimVar = mFX->GetVariableByName("gVoxelSize")->AsVector();
	mTargetVar = mFX->GetVariableByName("gTargetUAV")->AsUnorderedAccessView();
	mObjIDVar = mFX->GetVariableByName("gObjectID")->AsScalar();
	mResVar = mFX->GetVariableByName("gVoxelDim")->AsScalar();
	mWorMatVar = mFX->GetVariableByName("gWorld")->AsMatrix();

	// Create the input layout.
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	D3DX11_PASS_DESC PassDesc;
	mTech->GetPassByIndex(0)->GetDesc(&PassDesc);
	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &mLayout));

}

void Voxelizer::SetViewMatrix(const D3DXMATRIX& mCam, bool bUse)
{
	m_bSetView = bUse;
	mView = mCam;

}

void Voxelizer::BeginVoxelize(int iPrecision)
{
	// Because the algorithm uses GPU's rasterizer to generate voxels from triangles.
	// Rasterizer helps us slice triangle to a lot of chunks.
	// Set the size of viewport to define the precision of voxelization (While the size of viewport increases, it generates more chunks).
	mViewport.Width = iPrecision;
	mViewport.Height = iPrecision;
	mDeviceContext->RSSetViewports(1, &mViewport);

	// Set a custom view camera to generate voxel data.
	// The camera's position towards the center position (center of 3D scene) to maximize the usage of volume data (voxel GPU resource).
	if (m_bSetView){
		mViewMatVar->SetMatrix((float*)&(mView));

	}

	// Clear all voxel data to zero.
	const float zero[4] = { 0, 0, 0, 0 };
	mDeviceContext->ClearUnorderedAccessViewFloat(mUAV, zero);
	mTargetVar->SetUnorderedAccessView(mUAV);
	// Save all voxel data to an UAV, so it doesn't use any render target.
	mDeviceContext->OMSetRenderTargets(0, NULL, NULL);

	// Set the layout and the shaders (including vertex,geometry,pixel shaders) for voxelization.
	mDeviceContext->IASetInputLayout(mLayout);
	mTech->GetPassByIndex(0)->Apply(0, mDeviceContext);
}

ID3D11ShaderResourceView* Voxelizer::SRV()
{
	return mSRV;
}

void Voxelizer::build3DUAV()
{
	ID3D11Texture3D* pVolume = 0;
	D3D11_TEXTURE3D_DESC dstex;
	dstex.Width = m_iWidth;
	dstex.Height = m_iHeight;
	dstex.Depth = m_iDepth;
	dstex.MipLevels = 0;
	dstex.Format = mColorMapFormat;
	dstex.Usage = D3D11_USAGE_DEFAULT;
	dstex.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	dstex.CPUAccessFlags = 0;
	dstex.MiscFlags = 0;

	HR(md3dDevice->CreateTexture3D(&dstex, NULL, &mVolumeTex));

	// Create the render target views.
	D3D11_UNORDERED_ACCESS_VIEW_DESC  UAVDesc;
	UAVDesc.Format = dstex.Format;
	UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
	UAVDesc.Texture3D.FirstWSlice = 0;
	UAVDesc.Texture3D.MipSlice = 0;
	UAVDesc.Texture3D.WSize = m_iDepth;
	HR(md3dDevice->CreateUnorderedAccessView(mVolumeTex, &UAVDesc, &mUAV));

	// Create the resource view.
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory(&SRVDesc, sizeof(SRVDesc));
	SRVDesc.Format = dstex.Format;
	SRVDesc.Texture3D.MipLevels = 1;
	SRVDesc.Texture3D.MostDetailedMip = 0;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;

	HR(md3dDevice->CreateShaderResourceView(mVolumeTex, &SRVDesc, &mSRV));
}


void Voxelizer::copyDataFromSRV(char* name)
{
	struct VoxelData
	{
		int x;
		int y;
		int z;
	};

	std::vector<voxelViewData> results;


	// Copy voxel data from GPU.
	D3D11_MAPPED_SUBRESOURCE MappedResource = { 0 };
	// Copy GPU data to a stage buffer.
	mDeviceContext->CopyResource(mStageBuf, mLinkListBuf);
	HR(mDeviceContext->Map(mStageBuf, 0, D3D11_MAP_READ, 0, &MappedResource));
	// Load data from this stage buffer.
	voxelViewData* data = (voxelViewData*)(MappedResource.pData);

	// Start to write data.
	using namespace std;
	ofstream myfile;
	myfile.open(name);
	// The nonempty data are put in the front of this buffer, so loading a null data means the end of this buffer.
	for (int i = 0; i < m_iWidth*m_iHeight*m_iDepth; i++)
	{
		if (data[i].ID == 0)
		{
			break;
		}

		//File format: 
		//Position(Vector3)
		//Object ID(int)
		//Normal(Vector3)
		myfile << data[i].pos.x << " " << data[i].pos.y << " " << data[i].pos.z << "\n";
		myfile << (int)(data[i].ID * 255) << "\n";
		myfile << data[i].normal.x << " " << data[i].normal.y << " " << data[i].normal.z << "\n";
		results.push_back(data[i]);
	}
	myfile.close();
	mDeviceContext->Unmap(mStageBuf, 0);
}

void Voxelizer::CreateAppendBuffer()
{
	// Create an append buffer.
	D3D11_BUFFER_DESC desc;

	ZeroMemory(&desc, sizeof(desc));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc.ByteWidth = sizeof(voxelViewData)*m_iWidth*m_iHeight*m_iDepth;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.CPUAccessFlags = 0;
	desc.StructureByteStride = sizeof(voxelViewData);

	HR(md3dDevice->CreateBuffer(&desc, 0, &mLinkListBuf));
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
	uavDesc.Buffer.NumElements = m_iWidth*m_iHeight*m_iDepth;
	HR(md3dDevice->CreateUnorderedAccessView(mLinkListBuf, &uavDesc, &mUavList));
	ZeroMemory(&desc, sizeof(desc));

	desc.Usage = D3D11_USAGE_STAGING;
	desc.ByteWidth = sizeof(voxelViewData)*m_iWidth*m_iHeight*m_iDepth;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.StructureByteStride = sizeof(voxelViewData);
	HR(md3dDevice->CreateBuffer(&desc, NULL, &mStageBuf));
}

void Voxelizer::buildAppendCS()
{
	// Create a computer shader.
	DWORD shaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
	ID3D10Blob* pCode;
	ID3D10Blob* pError;

	HRESULT hr = 0;
	hr = D3DX11CompileFromFile("Shaders/AppendVoxelList.hlsl", NULL, NULL, "CSMain", "cs_5_0", shaderFlags, NULL, NULL, &pCode, &pError, NULL);

	if (FAILED(hr))
	{
		if (pError)
		{
			MessageBoxA(0, (char*)pError->GetBufferPointer(), 0, 0);
			SAFE_RELEASE(pError);
		}
		DXTraceW(_T(__FILE__), __LINE__, hr, L"D3DX11CreateEffectFromFile", TRUE);
	}

	HR(md3dDevice->CreateComputeShader(pCode->GetBufferPointer(), pCode->GetBufferSize(), NULL, &mAppendCS));
	D3D11_BUFFER_DESC constant_buffer_desc;

	ZeroMemory(&constant_buffer_desc, sizeof(constant_buffer_desc));
	constant_buffer_desc.ByteWidth = 16;
	constant_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constant_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	HR(md3dDevice->CreateBuffer(&constant_buffer_desc, NULL, &mCB));
}

void Voxelizer::AppendVoxelData(ID3D11ShaderResourceView* voxelData, ID3D11UnorderedAccessView* appendVoxelBuf)
{
	// Preparing data for a computer shader.
	const int NUM_THREAD = 64;
	struct CB{
		float fVoxelDim;
	};
	ID3D11ShaderResourceView* aRViews[1] = { voxelData };

	CB cb;
	cb.fVoxelDim = m_iWidth;
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	mDeviceContext->Map(mCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	memcpy(MappedResource.pData, &cb, sizeof(cb));
	mDeviceContext->Unmap(mCB, 0);
	mDeviceContext->CSSetConstantBuffers(0, 1, &mCB);
	mDeviceContext->CSSetShaderResources(0, 1, aRViews);
	mDeviceContext->CSSetUnorderedAccessViews(0, 1, &appendVoxelBuf, NULL);
	mDeviceContext->CSSetShader(mAppendCS, NULL, 0);

	mDeviceContext->Dispatch(m_iWidth*m_iHeight*m_iDepth / NUM_THREAD, 1, 1);

	// Reset to initial states.
	mDeviceContext->CSSetShader(NULL, NULL, 0);
	ID3D11UnorderedAccessView* ppUAViewNULL[1] = { NULL };
	mDeviceContext->CSSetUnorderedAccessViews(0, 1, ppUAViewNULL, NULL);
	ID3D11ShaderResourceView* ppSRVNULL[1] = { NULL };
	mDeviceContext->CSSetShaderResources(0, 1, ppSRVNULL);
	ID3D11Buffer* ppCBNULL[1] = { NULL };
	mDeviceContext->CSSetConstantBuffers(0, 1, ppCBNULL);
}

void Voxelizer::WriteToFile(char* filename)
{
	// Sort voxel data to the front of buffer to speed up the output process.
	CreateAppendBuffer();
	buildAppendCS();
	AppendVoxelData(mSRV, mUavList);

	// Copy data to CPU.
	copyDataFromSRV(filename);

	// Release the GPU resources after finishing the output process.
	SAFE_RELEASE(mStageBuf);
	SAFE_RELEASE(mUavList);
	SAFE_RELEASE(mLinkListBuf);
}

void Voxelizer::SetWorldMatrix(const D3DXMATRIX* const  pWorld)
{
	mWorMatVar->SetMatrix((float*)pWorld);
	mTech->GetPassByIndex(0)->Apply(0, mDeviceContext);
}

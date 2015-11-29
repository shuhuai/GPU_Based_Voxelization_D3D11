//--------------------------------------------------------------------------------------
// File: Voxelizer.h
//
// The declaration of voxelizer class.
//--------------------------------------------------------------------------------------

#ifndef _VOXELIZER_H_
#define _VOXELIZER_H_

#include "d3dutil.h"
#include <d3dCompiler.h>
#include <d3dx11effect.h>
#include "ConvertObj.h"
#include "Vertex.h"

struct voxelViewData
{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 normal;
	float ID;
};

class Voxelizer
{
public:
	Voxelizer(void);
	~Voxelizer();
	
	// Initialize voxel D3D resources.
	void Init(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, UINT uRes, DXGI_FORMAT ColorFormat);

	// Apply voxelization shaders.
	void BeginVoxelize(int iPrecision);
	
	// Settings for voxelization.

	// Set world matrix for different transforms.
	void SetWorldMatrix(const D3DXMATRIX* const pWorld);
	// Set voxelization camera.
	void SetViewMatrix(const D3DXMATRIX& view, bool bSetView);
	// Set voxel size in world unit.
	void SetVoxelSize(const float* const pfDim){ mVoxelDimVar->SetFloatVector(pfDim); }
	// Set object ID.
	void SetObjID(int iID){ mObjIDVar->SetFloat((float)iID / 256.f);	mTech->GetPassByIndex(0)->Apply(0, mDeviceContext); }

	// Reset the resolution (e.g.:128x128x128) of voxels.
	void ResetVoxelRes(UINT uRes, DXGI_FORMAT ColorFormat);

	// Get the voxel GPU resource.
	ID3D11ShaderResourceView* SRV();
	// Get current voxel resolution.
	float GetResoultion(){ return m_iWidth; }

	// Write voxel data from GPU resources to a CPU file.
	void WriteToFile(char* filename);

private:
	void CreateAppendBuffer();
	void AppendVoxelData(ID3D11ShaderResourceView* voxelData, ID3D11UnorderedAccessView* appendVoxelBuf);
	void buildAppendCS();
	void copyDataFromSRV(char* name);
	void buildFX();
	void build3DUAV();


private:

	bool m_bSetView;

	D3DXMATRIX mWVPVolume;
	D3DXMATRIX mView;

	UINT m_iWidth;
	UINT m_iHeight;
	UINT m_iDepth;

	DXGI_FORMAT mColorMapFormat;
	D3D11_VIEWPORT mViewport;

	// Effect variables.
	ID3DX11Effect* mFX;
	ID3DX11EffectTechnique* mTech;
	ID3DX11EffectMatrixVariable* mViewMatVar;
	ID3DX11EffectMatrixVariable* mWorMatVar;
	ID3DX11EffectScalarVariable* mObjIDVar;
	ID3DX11EffectScalarVariable* mResVar;
	ID3DX11EffectVectorVariable* mVoxelDimVar;
	ID3DX11EffectUnorderedAccessViewVariable* mTargetVar;

	// GPU resources.
	ID3D11ComputeShader* mAppendCS;
	ID3D11Texture3D* mVolumeTex;
	ID3D11Buffer* mCB;
	ID3D11Buffer* mLinkListBuf;
	ID3D11Buffer* mStageBuf;

	ID3D11InputLayout* mLayout;
	ID3D11Device* md3dDevice;
	ID3D11DeviceContext* mDeviceContext;

	ID3D11ShaderResourceView* mSRV;
	ID3D11ShaderResourceView* mSrvList;
	ID3D11UnorderedAccessView* mUAV;
	ID3D11UnorderedAccessView *mUavList;
};
#endif // _VOXELIZER_H_
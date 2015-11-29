//--------------------------------------------------------------------------------------
// File: RawDataViewer.h
//
// A class for showing raw voxel data.
//--------------------------------------------------------------------------------------
#ifndef _VOXELSHOWER_H_
#define _VOXELSHOWER_H_

#include "d3dutil.h"
#include <d3dCompiler.h>
#include <d3dx11effect.h>
#include "ConvertObj.h"
#include "Vertex.h"

class RawDataViewer
{
public:
	RawDataViewer(void);
	~RawDataViewer();

	void Init(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);
	// Show one slice of voxel data on a full-screen quad.
	void ShowRawVoxeldata(ID3D11ShaderResourceView*	pData, float fIndex);

private:
	
	ID3D11Device* md3dDevice;
	ID3D11DeviceContext* mDeviceContext;
	// GPU resources.
	ID3D11Buffer* mVB;
	ID3D11InputLayout* mVertexSlipLayout;

	// Effect variables.
	ID3DX11EffectTechnique*	mRttTech;
	ID3DX11Effect* mRttFX;
	ID3DX11EffectShaderResourceVariable*	mTargetVar;
	ID3DX11EffectScalarVariable* mIndexVar;
	
};
#endif
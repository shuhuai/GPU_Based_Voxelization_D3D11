//--------------------------------------------------------------------------------------
// File: voxelViewer.h
//
// A class for voxel visualization.
//--------------------------------------------------------------------------------------
#ifndef _VOXELVIEWER_H_
#define _VOXELVIEWER_H_
#include "d3dutil.h"
#include <d3dCompiler.h>
#include <d3dx11effect.h>
#include "ConvertObj.h"
#include "Vertex.h"
#include "Voxelizer.h"

class  voxelViewer
{

public:
	voxelViewer(void);
	~voxelViewer();
	void Init(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);

	//--------------------------------------------------------------------------------------
	// Render cubes to visualize voxel data.
	// Voxel: voxel data(Texture3D).
	// Number: total number of voxels.
	// Scale: the size of cubes.
	// Pass: show different type of data:	
	// 0: Normal
	// 1: Binary 
	// 2: Object ID
	//--------------------------------------------------------------------------------------
	void ShowVoxels(ID3D11ShaderResourceView* pVoxel, int iNumber, float fScale, int iPass);
	void SetRes(int iRes){ m_iRes=iRes; }
private:
	void createColorTable(int iNumber);	//Create a color table to visualize different object.
	void buildFX();
	void setupInputLayout(); 

	int m_iRes;

	ID3D11Device* md3dDevice;
	ID3D11DeviceContext* mDeviceContext;
	// GPU resources.
	ID3D11ShaderResourceView * mColorTex;
	ID3D11InputLayout*  mLayout;

	
	// Effect variables.
	ID3DX11Effect* mFX;
	ID3DX11EffectShaderResourceVariable* mVoxelVar;
	ID3DX11EffectShaderResourceVariable* mColorTableVar;
	ID3DX11EffectTechnique* mTech;
	ID3DX11EffectMatrixVariable* mWVPVar;
	ID3DX11EffectMatrixVariable* mPVar;
	ID3DX11EffectVectorVariable* mLightDirVar;
	ID3DX11EffectScalarVariable* mVoxelScaleVar;
	ID3DX11EffectShaderResourceVariable*	mEdgeTexVar;

	
};
#endif 